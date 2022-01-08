//
// Created by Theofilos on 07-Jan-22.
//

#include <iostream>
#include "../include/ThreadPool.hpp"

ThreadPool::ThreadPool() : terminate_pool(false), num_threads(std::thread::hardware_concurrency()) {
    job_queues = new std::queue<Job>[num_threads];
    queue_locks = new std::mutex[num_threads];
    queue_cvs = new std::condition_variable[num_threads];
    for (uint32_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(queue_loop, this, i);
    }
}

void ThreadPool::queue_loop(ThreadPool *tp, int thread_id) {
    while (true)
    {
        std::unique_lock<std::mutex> queue_lock(tp->queue_locks[thread_id]);
        auto& queue = tp->job_queues[thread_id];
        tp->queue_cvs[thread_id].wait(queue_lock, [&queue, &tp]{ return !queue.empty() || tp->terminate_pool; });
        if (tp->terminate_pool) {
            break;
        }
        auto job = queue.front();
        queue.pop();
        job(thread_id);
    }
}

void ThreadPool::add_job(const Job& job, const std::string& asset) {
    auto tid = hasher(asset);
    {
        std::unique_lock<std::mutex> queue_lock(queue_locks[tid]);
        job_queues[tid].push(job);
    }
    queue_cvs[tid].notify_one();
}

int ThreadPool::hasher(const std::string& key) const {
    return static_cast<int>(hash(key) % num_threads);
}

ThreadPool::~ThreadPool() {
    delete[] job_queues;
    delete[] queue_cvs;
    delete[] queue_locks;
}

void ThreadPool::terminate() {
    terminate_pool = true;
    for(int i = 0; i < num_threads; ++i) {
        queue_cvs[i].notify_one();
    }
    for(auto& thread : threads) {
        thread.join();
    }
}

