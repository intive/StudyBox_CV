#ifndef PATR_THREADPOOL_H
#define PATR_THREADPOOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <memory>
#include <type_traits>

namespace Utility {

template<typename Job, typename Handler>
class ThreadPool : private std::conditional<std::is_void<Handler>::value, std::true_type, Handler>::type
{
public:
    typedef Handler HandlerType;
    typedef Job JobType;

    template<typename... HandlerArgs, typename H = Handler, typename std::enable_if<!std::is_void<H>::value>::type* = nullptr>
    ThreadPool(std::size_t maxThreads, std::size_t maxLoad, HandlerArgs&&... args) : Handler(std::forward<HandlerArgs>(args)...), maxThreads(maxThreads), maxLoad(maxLoad), stop(false)
    {
        start();
    }

    template<typename... HandlerArgs, typename H = Handler, typename std::enable_if<std::is_void<H>::value>::type* = nullptr>
    ThreadPool(std::size_t maxThreads, std::size_t maxLoad, HandlerArgs&&... args) : maxThreads(maxThreads), maxLoad(maxLoad), stop(false)
    {
        start();
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

    template<typename JobT>
    bool add(JobT&& job)
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (stop)
                throw std::runtime_error("attempted to add to stopped pool");

            if (jobs.size() > maxLoad)
                return false;

            jobs.emplace(std::forward<JobT>(job));
        }
        condition.notify_one();

        return true;
    }

private:

    void start()
    {
        for (size_t i = 0; i < maxThreads; ++i)
            workers.emplace_back(
                [this]
        {
            for (;;)
            {
                std::unique_ptr<Job> job;

                {
                    std::unique_lock<std::mutex> lock(mutex);
                    condition.wait(lock,
                        [this] { return stop || !jobs.empty(); });
                    if (stop && jobs.empty())
                        return;
                    job.reset(new Job(std::move(jobs.front())));
                    jobs.pop();
                }

                call(std::move(*job));
            }
        }
        );
    }

    template<typename H = Handler, typename std::enable_if<std::is_void<H>::value>::type* = nullptr>
    void call(Job&& job)
    {
        std::move(job)();
    }

    template<typename H = Handler, typename std::enable_if<!std::is_void<H>::value>::type* = nullptr>
    void call(Job&& job)
    {
        (*this)(std::move(job));
    }

    std::size_t maxThreads;
    std::size_t maxLoad;

    std::vector<std::thread> workers;
    std::queue<JobType> jobs;

    std::mutex mutex;
    std::condition_variable condition;
    bool stop;
};

} // namespace Utility

#endif // PATR_THREADPOOL_H
