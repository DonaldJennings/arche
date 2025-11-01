#ifndef ARCHE_CORE_JOBPOOLSERVICE_H
#define ARCHE_CORE_JOBPOOLSERVICE_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

namespace Arche {
namespace Core {

/**
 * @brief JobPoolService provides a thread pool for managing and executing jobs.
 *        It is intended to be owned and managed by EngineCore and shared with subsystems.
 */
class JobPoolService {
public:
    using Job = std::function<void()>;

    explicit JobPoolService(size_t numThreads = std::thread::hardware_concurrency());
    ~JobPoolService();

    void enqueue(Job job);
    void wait();

    JobPoolService(const JobPoolService&) = delete;
    JobPoolService& operator=(const JobPoolService&) = delete;
    JobPoolService(JobPoolService&&) = delete;
    JobPoolService& operator=(JobPoolService&&) = delete;

private:
    std::vector<std::thread> workers_;
    std::queue<Job> jobs_;
    std::mutex queueMutex_;
    std::condition_variable jobsCv_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> activeJobs_{0};
};

} // namespace Core
} // namespace Arche

#endif // ARCHE_CORE_JOBPOOLSERVICE_H