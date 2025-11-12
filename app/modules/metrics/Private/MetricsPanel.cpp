#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "MetricsPanel.h"

#include <imgui.h>
#include <WorldSystem.h>

#include <iostream>
#include <chrono>
#include <vector>
#include <array>
#include <numeric>
#include <algorithm> // for std::max, std::min

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")
#include <dxgi1_4.h>
#pragma comment(lib, "dxgi.lib")
#endif

namespace Arche {
    namespace GUI {

#ifdef _WIN32
        static inline uint64_t FiletimeToUInt64(const FILETIME &ft) {
            return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
        }
#endif

        MetricsPanel::MetricsPanel(std::shared_ptr<UIContext> contextIn)
            : name{"Metrics"}, context(std::move(contextIn)),
              cpuBuffer_(120), memBuffer_(120), netBuffer_(120), gpuBuffer_(120)
        {
            lastSampleTime_ = std::chrono::steady_clock::now() - std::chrono::milliseconds(sampleIntervalMs_);
#ifdef _WIN32
            // initialize previous system times
            FILETIME idleFT{}, kernelFT{}, userFT{};
            if (GetSystemTimes(&idleFT, &kernelFT, &userFT)) {
                prevSysIdleTime_   = FiletimeToUInt64(idleFT);
                prevSysKernelTime_ = FiletimeToUInt64(kernelFT);
                prevSysUserTime_   = FiletimeToUInt64(userFT);
            } else {
                prevSysIdleTime_ = prevSysKernelTime_ = prevSysUserTime_ = 0;
            }

            // previous process time (kernel + user)
            FILETIME procCreation{}, procExit{}, procKernel{}, procUser{};
            HANDLE hProc = GetCurrentProcess();
            if (GetProcessTimes(hProc, &procCreation, &procExit, &procKernel, &procUser)) {
                prevProcTime_ = FiletimeToUInt64(procKernel) + FiletimeToUInt64(procUser);
            } else {
                prevProcTime_ = 0;
            }

            // network baseline
            prevNetBytes_ = SumIfTableOctets();

            // Initialize DXGI adapter for GPU memory (best-effort)
            dxgiAdapter3_ = nullptr;
            IDXGIFactory1* factory = nullptr;
            if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory))) {
                IDXGIAdapter1* adapter1 = nullptr;
                if (SUCCEEDED(factory->EnumAdapters1(0, &adapter1))) {
                    IDXGIAdapter3* adapter3 = nullptr;
                    if (SUCCEEDED(adapter1->QueryInterface(__uuidof(IDXGIAdapter3), (void**)&adapter3))) {
                        dxgiAdapter3_ = reinterpret_cast<void*>(adapter3);
                    }
                    adapter1->Release();
                }
                factory->Release();
            }
#endif
        }

        void MetricsPanel::Draw() {
            ImGui::Begin(name.c_str());
            // Show frame rate and frame time
            ImGuiIO &io = ImGui::GetIO();
            ImGui::Text("Framerate: %.1f FPS", io.Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);

            // Sample metrics at configured interval
            auto now = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSampleTime_).count();
            if (elapsedMs >= sampleIntervalMs_) {
#ifdef _WIN32
                float cpuPct = SampleProcessCpuPercent(static_cast<int>(elapsedMs));
                cpuBuffer_.push(cpuPct);

                float memPct = SampleMemoryPercent();
                memBuffer_.push(memPct);

                float netPct = SampleNetworkPercent(static_cast<int>(elapsedMs));
                netBuffer_.push(netPct);

                float gpuPct = SampleGpuMemoryPercent();
                gpuBuffer_.push(gpuPct);
#else
                cpuBuffer_.push(0.0f);
                memBuffer_.push(0.0f);
                netBuffer_.push(0.0f);
                gpuBuffer_.push(0.0f);
#endif
                lastSampleTime_ = now;
            }

            // Draw small summary row
            ImGui::Separator();
            ImGui::Text("System Metrics (Process/Device %%):");

            DrawMetricColumn("CPU %", cpuBuffer_);
            DrawMetricColumn("Memory %", memBuffer_);
            DrawMetricColumn("Network %", netBuffer_);
            DrawMetricColumn("GPU %", gpuBuffer_);

            ImGui::End();
        }

        std::string_view MetricsPanel::GetName() const { return name; }

        void MetricsPanel::DrawMetricColumn(const char* label, CircularBuffer &buffer) {
            std::vector<float> linear;
            buffer.copyLinear(linear);
            float current = linear.empty() ? 0.0f : linear.back();
            ImGui::Text("%s", label);
            ImGui::TextColored(ImVec4(1,1,0,1), "%.1f%%", current);
            if (!linear.empty()) {
                ImGui::PlotLines(label, linear.data(), static_cast<int>(linear.size()), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 60));
            } else {
                ImGui::TextUnformatted("N/A");
            }
        }

#ifdef _WIN32
        float MetricsPanel::SampleProcessCpuPercent(int /*elapsedMs*/) {
            FILETIME idleFT{}, kernelFT{}, userFT{};
            if (!GetSystemTimes(&idleFT, &kernelFT, &userFT)) return 0.0f;

            uint64_t sysIdle = FiletimeToUInt64(idleFT);
            uint64_t sysKernel = FiletimeToUInt64(kernelFT);
            uint64_t sysUser = FiletimeToUInt64(userFT);

            uint64_t sysTotal = (sysKernel - prevSysKernelTime_) + (sysUser - prevSysUserTime_);
            // process times
            FILETIME procCreation{}, procExit{}, procKernel{}, procUser{};
            uint64_t procTime = 0;
            HANDLE hProc = GetCurrentProcess();
            if (GetProcessTimes(hProc, &procCreation, &procExit, &procKernel, &procUser)) {
                procTime = FiletimeToUInt64(procKernel) + FiletimeToUInt64(procUser);
            }

            uint64_t procDelta = (procTime > prevProcTime_) ? (procTime - prevProcTime_) : 0;

            // update prev
            prevSysIdleTime_ = sysIdle;
            prevSysKernelTime_ = sysKernel;
            prevSysUserTime_ = sysUser;
            prevProcTime_ = procTime;

            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            const uint32_t numProcs = static_cast<uint32_t>(sysInfo.dwNumberOfProcessors ? sysInfo.dwNumberOfProcessors : 1);

            double sysTotalD = static_cast<double>(sysTotal);
            if (sysTotalD <= 0.0) return 0.0f;

            double pct = (static_cast<double>(procDelta) / sysTotalD) * 100.0;
            if (pct < 0.0) pct = 0.0;
            if (pct > 100.0 * numProcs) pct = 100.0 * numProcs;
            pct = pct / static_cast<double>(numProcs);
            return static_cast<float>(pct);
        }

        float MetricsPanel::SampleMemoryPercent() {
            PROCESS_MEMORY_COUNTERS_EX pmc = {};
            if (!GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
                return 0.0f;
            }
            MEMORYSTATUSEX memStatus;
            memStatus.dwLength = sizeof(memStatus);
            if (!GlobalMemoryStatusEx(&memStatus)) return 0.0f;

            uint64_t workingSet = static_cast<uint64_t>(pmc.WorkingSetSize);
            uint64_t totalPhys = memStatus.ullTotalPhys;
            if (totalPhys == 0) return 0.0f;
            double pct = (static_cast<double>(workingSet) / static_cast<double>(totalPhys)) * 100.0;
            return static_cast<float>(pct);
        }

        uint64_t MetricsPanel::SumIfTableOctets() {
            PMIB_IFTABLE table = nullptr;
            uint64_t total = 0;
            // First call to get required size
            ULONG size = 0;
            if (GetIfTable(nullptr, &size, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
                table = (PMIB_IFTABLE)LocalAlloc(LPTR, size);
                if (table) {
                    if (GetIfTable(table, &size, FALSE) == NO_ERROR) {
                        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
                            MIB_IFROW &row = table->table[i];
                            if (row.dwOperStatus == IF_OPER_STATUS_OPERATIONAL) {
                                total += static_cast<uint64_t>(row.dwInOctets) + static_cast<uint64_t>(row.dwOutOctets);
                            }
                        }
                    }
                    LocalFree(table);
                }
            }
            return total;
        }

        uint64_t MetricsPanel::SumIfTableSpeeds() {
            PMIB_IFTABLE table = nullptr;
            uint64_t totalBitsPerSec = 0;
            ULONG size = 0;
            if (GetIfTable(nullptr, &size, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
                table = (PMIB_IFTABLE)LocalAlloc(LPTR, size);
                if (table) {
                    if (GetIfTable(table, &size, FALSE) == NO_ERROR) {
                        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
                            MIB_IFROW &row = table->table[i];
                            if (row.dwOperStatus == IF_OPER_STATUS_OPERATIONAL) {
                                totalBitsPerSec += static_cast<uint64_t>(row.dwSpeed);
                            }
                        }
                    }
                    LocalFree(table);
                }
            }
            return totalBitsPerSec;
        }

        float MetricsPanel::SampleNetworkPercent(int elapsedMs) {
            uint64_t currentBytes = SumIfTableOctets();
            uint64_t deltaBytes = 0;
            if (currentBytes >= prevNetBytes_) deltaBytes = currentBytes - prevNetBytes_;
            else deltaBytes = 0;
            prevNetBytes_ = currentBytes;

            double seconds = std::max(1.0, elapsedMs / 1000.0);
            double bitsPerSecMeasured = (static_cast<double>(deltaBytes) * 8.0) / seconds;

            uint64_t totalLinkBitsPerSec = SumIfTableSpeeds();
            if (totalLinkBitsPerSec == 0) return 0.0f;

            double pct = (bitsPerSecMeasured / static_cast<double>(totalLinkBitsPerSec)) * 100.0;
            if (pct < 0.0) pct = 0.0;
            if (pct > 100.0) pct = 100.0;
            return static_cast<float>(pct);
        }

        float MetricsPanel::SampleGpuMemoryPercent() {
            if (!dxgiAdapter3_) return 0.0f;
            IDXGIAdapter3* adapter3 = reinterpret_cast<IDXGIAdapter3*>(dxgiAdapter3_);
            if (!adapter3) return 0.0f;
            DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
            HRESULT hr = adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
            if (FAILED(hr)) return 0.0f;
            if (info.Budget == 0) return 0.0f;
            double pct = (static_cast<double>(info.CurrentUsage) / static_cast<double>(info.Budget)) * 100.0;
            if (pct < 0.0) pct = 0.0;
            if (pct > 100.0) pct = 100.0;
            return static_cast<float>(pct);
        }
#endif // _WIN32

    } // namespace GUI
} // namespace Arche
