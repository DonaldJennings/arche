#ifndef ARCHE_JOB_POOL_H
#define ARCHE_JOB_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

namespace Arche {
    namespace Core {
        /**
         * @brief A simple job pool for managing and executing jobs.
         */
        class JobPool {
          public:
            using Job = std::function<void()>;

            explicit JobPool(size_t numThreads = std::thread::hardware_concurrency())
            {
                stop = false;
                for (size_t i = 0; i < numThreads; ++i) {
                    workers.emplace_back([this]() {
                        while (true) {
                            Job job;
                            {
                                std::unique_lock<std::mutex> lock(queueMutex);
                                jobsCv.wait(lock, [this]() {
                                    return stop || !jobs.empty();
                                });
                                if (stop && jobs.empty()) {
                                    return;
                                }
                                job = std::move(jobs.front());
                                jobs.pop();
                                ++activeJobs;
                            }
                            job();
                            --activeJobs;
                            jobsCv.notify_all();
                        }
                    });
                }
            }

            ~JobPool() {
                stop = true;
                jobsCv.notify_all();
                for (std::thread &worker : workers) {
                    if (worker.joinable()) {
                        worker.join();
                    }
                }
            }

            void enqueue(Job job) {
                std::unique_lock<std::mutex> lock(queueMutex);
                jobs.push(std::move(job));
            }

            void wait() {
                std::unique_lock<std::mutex> lock(queueMutex);
                jobsCv.wait(
                    lock, [this]() { return jobs.empty() && activeJobs == 0; });
            }

            JobPool(const JobPool &) = delete;
            JobPool &operator=(const JobPool &) = delete;
            JobPool(JobPool &&) = delete;
            JobPool &operator=(JobPool &&) = delete;

          private:
            std::vector<std::thread> workers;
            std::queue<Job> jobs;
            std::mutex queueMutex;
            std::condition_variable jobsCv;
            std::atomic<bool> stop{false};
            std::atomic<size_t> activeJobs{0};
        };
    } // namespace Core
} // namespace Arche
#endif // ARCHE_JOB_POOL_H