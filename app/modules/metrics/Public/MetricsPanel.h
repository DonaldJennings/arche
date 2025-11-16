#pragma once

#include <string>
#include <memory>
#include <string_view>
#include <vector>
#include <chrono>
#include <cstdint>

#include <IPanel.h>
#include <WorldSystem.h>
#include "EditorSession.h"

namespace Arche {
    namespace GUI {

        // Simple fixed-capacity circular buffer for plotting small history windows.
        class CircularBuffer {
          public:
            explicit CircularBuffer(size_t capacity = 128)
                : data_(capacity, 0.0f), head_(0), full_(false) {}

            void push(float v) {
                data_[head_] = v;
                head_ = (head_ + 1) % data_.size();
                if (head_ == 0) full_ = true;
            }

            // Returns raw storage (not in chronological order if buffer wrapped).
            const std::vector<float>& data() const { return data_; }

            // Number of valid samples currently stored (<= capacity).
            size_t size() const { return full_ ? data_.size() : head_; }

            // Copies samples into 'out' in chronological order (oldest -> newest).
            void copyLinear(std::vector<float>& out) const {
                out.clear();
                if (!full_ && head_ == 0) return;
                if (!full_) {
                    out.insert(out.end(), data_.begin(), data_.begin() + head_);
                } else {
                    out.insert(out.end(), data_.begin() + head_, data_.end());
                    out.insert(out.end(), data_.begin(), data_.begin() + head_);
                }
            }

          private:
            std::vector<float> data_;
            size_t head_;
            bool full_;
        };

        class MetricsPanel : public IPanel {
          public:
            explicit MetricsPanel(std::shared_ptr<EditorSession> contextIn);

            void Draw() override;
            std::string_view GetName() const override;

          private:
            // UI helper to draw a single metric column (label + current value + sparkline)
            void DrawMetricColumn(const char *label, CircularBuffer &buffer);

            // history buffers (percentage values 0..100)
            CircularBuffer cpuBuffer_;
            CircularBuffer memBuffer_;
            CircularBuffer netBuffer_;
            CircularBuffer gpuBuffer_;

            // Sampling providers (platform-specific implementations live in .cpp)
            float SampleGpuMemoryPercent();
            float SampleMemoryPercent();
            float SampleProcessCpuPercent(int elapsedMs);
            float SampleNetworkPercent(int elapsedMs);

            // Helper (windows implementation uses OS network tables)
            uint64_t SumIfTableOctets();
            uint64_t SumIfTableSpeeds();

            // Timing / sampling
            std::chrono::steady_clock::time_point lastSampleTime_{};
            const int sampleIntervalMs_{1000};

            // Previous values for delta calculations
            std::uint64_t prevNetBytes_{0};
            std::uint64_t prevSysIdleTime_{0};
            std::uint64_t prevSysKernelTime_{0};
            std::uint64_t prevSysUserTime_{0};
            std::uint64_t prevProcTime_{0};

            // Best-effort GPU adapter handle (opaque pointer to avoid leaking DXGI headers into the public header)
            void *dxgiAdapter3_{nullptr};

            std::string name;
            std::shared_ptr<EditorSession> context;
        };

    } // namespace GUI
} // namespace Arche