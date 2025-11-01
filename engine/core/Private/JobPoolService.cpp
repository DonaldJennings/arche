#include "JobPoolService.h"

namespace Arche {
namespace Core {

JobPoolService::JobPoolService(size_t numThreads) {
    stop_ = false;
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this]() {
            while (true) {
                Job job;
                {
                    std::unique_lock<std::mutex> lock(queueMutex_);
                    jobsCv_.wait(lock, [this]() {
                        return stop_ || !jobs_.empty();
                    });
                    if (stop_ && jobs_.empty()) {
                        return;
                    }
                    job = std::move(jobs_.front());
                    jobs_.pop();
                    ++activeJobs_;
                }
                job();
                --activeJobs_;
                jobsCv_.notify_all();
            }
        });
    }
}

JobPoolService::~JobPoolService() {
    stop_ = true;
    jobsCv_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void JobPoolService::enqueue(Job job) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    jobs_.push(std::move(job));
    jobsCv_.notify_one();
}

void JobPoolService::wait() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    jobsCv_.wait(lock, [this]() { return jobs_.empty() && activeJobs_ == 0; });
}

} // namespace Core
} // namespace Arche