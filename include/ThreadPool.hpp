//
// Created by Theofilos on 07-Jan-22.
//

#ifndef MATCHING_ENGINE_THREADPOOL_HPP
#define MATCHING_ENGINE_THREADPOOL_HPP

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

class ThreadPool {
private:
    typedef std::function<void(int)> Job;
    bool terminate_pool;
    std::vector<std::thread> threads;
    std::hash<std::string> hash;
    std::queue<Job>* job_queues;
    std::mutex* queue_locks;
    std::condition_variable* queue_cvs;

    static void queue_loop(ThreadPool *tp, int thread_id);
public:
    ThreadPool();
    ~ThreadPool();
    [[nodiscard]] inline int hasher(const std::string& key) const;
    void add_job(const Job& job, const std::string& asset);
    void terminate();
    const unsigned int num_threads;
};

#endif //MATCHING_ENGINE_THREADPOOL_HPP
