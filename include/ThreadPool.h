// Adapted from https://github.com/KevinZeng08/sigmod-2024-contest.git)

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>
#include <iostream>

using namespace std;

class ThreadPool
{
public:
    ThreadPool(size_t);
    template <class F, class... Args>
    auto
    push(F &&f, Args &&...args) -> future<typename result_of<F(Args...)>::type>;
    ~ThreadPool();

    static shared_ptr<ThreadPool> GetBuildPool()
    {
        lock_guard<mutex> lock(build_pool_mutex_);
        if (build_pool_ == nullptr)
        {
            build_pool_ = make_shared<ThreadPool>(thread::hardware_concurrency());
        }
        return build_pool_;
    }

    static shared_ptr<ThreadPool> GetSearchPool()
    {
        lock_guard<mutex> lock(search_pool_mutex_);
        if (search_pool_ == nullptr)
        {
            search_pool_ = make_shared<ThreadPool>(thread::hardware_concurrency());
        }
        return search_pool_;
    }

private:
    // need to keep track of threads so we can join them
    vector<thread> workers;
    // the task queue
    queue<function<void()>> tasks;

    // synchronization
    mutex queue_mutex;
    condition_variable condition;
    bool stop;

    inline static mutex build_pool_mutex_;
    inline static shared_ptr<ThreadPool> build_pool_ = nullptr;

    inline static mutex search_pool_mutex_;
    inline static shared_ptr<ThreadPool> search_pool_ = nullptr;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this]
                             {
            for (;;) {
                function<void()> task;

                {
                    unique_lock<mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = move(this->tasks.front());
                    this->tasks.pop();
                }

                task();
            } });
}

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::push(F &&f, Args &&...args) -> future<typename result_of<F(Args...)>::type>
{
    using return_type = typename result_of<F(Args...)>::type;

    auto task = make_shared<packaged_task<return_type()>>(
        bind(forward<F>(f), forward<Args>(args)...));

    future<return_type> res = task->get_future();
    {
        unique_lock<mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop)
            throw runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]()
                      { (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        unique_lock<mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (thread &worker : workers)
        worker.join();
}
