#ifndef H_TASKPOOL
#define H_TASKPOOL

#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>

struct TaskClosurePool{
private:
    // NOTE(hugo): task is executed by the worker thread
    //             on_complete can be executed by the main thread when task is done using ExecuteClosures
    struct Task{
        void (*task)();
        void (*closure)();
    };

public:

    TaskClosurePool(int nthreads);
    ~TaskClosurePool();

    // NOTE(hugo): closure = std::function<void()>() can be used to enqueue an empty function
    void EnqueueTask(void (*task)(), void (*closure)() = nullptr);

    bool isTasksEmpty();
    bool isClosuresEmpty();

    int CountTasks();
    int CountClosures();

    void SynchronizeTasks();
    void ExecuteClosures();

private:

    void EnqueueTask(Task& task);

    std::mutex tasks_mutex;
    std::queue<Task> tasks;

    std::mutex closures_mutex;
    std::queue<void(*)()> closures;

    std::condition_variable workers_cv;
    std::vector<std::thread> workers;

    std::condition_variable synctask_cv;

    std::atomic<bool> terminate;
    std::atomic<int> working_threads;
};

#endif
