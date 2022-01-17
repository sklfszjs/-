#include "Threadpool.h"
#include<unistd.h>
#include<iostream>
/*#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>
using namespace std;
void* worker(void* args);
void* manager(void* args);
struct Task {
    void (*function)(void* args);
    void* args;
};
struct ThreadPool {
    Task* taskq;
    int queuecapacity;
    int queuesize;
    int queuefront;
    int queuerear;

    pthread_t manageid;
    pthread_t* workerid;
    int minnum;
    int maxnum;
    int busynum;
    int livenum;
    int exitnum;
    pthread_mutex_t mutexpool;
    pthread_mutex_t mutexbusy;
    pthread_cond_t notfull;
    pthread_cond_t notempty;
    bool shutdown;
    int step;

};
void thread_exit(ThreadPool* pool) {
    pthread_t threadid = pthread_self();
    for (int i = 0; i < pool->maxnum; i++) {
        std::cout << "thread exit" << std::endl;
        if (pool->workerid[i] == threadid) {
            pool->workerid[i] = 0;//这里当初写成了==
            break;//不要忘了break
        }
        pthread_exit(nullptr);
    }

}
ThreadPool* threadpool_create(int minnum, int maxnum, int queuecapacity, int step) {
    ThreadPool* threadpool = (ThreadPool*)malloc(sizeof(ThreadPool));
    threadpool->taskq = (Task*)malloc(sizeof(Task) * queuecapacity);
    threadpool->queuecapacity = queuecapacity;

    threadpool->queuesize = 0;
    threadpool->queuefront = 0;
    threadpool->queuerear = 0;

    threadpool->workerid = (pthread_t*)malloc(sizeof(pthread_t) * maxnum);
    bzero(threadpool->workerid, sizeof(threadpool->workerid));
    threadpool->minnum = minnum;
    threadpool->maxnum = maxnum;
    threadpool->busynum = 0;
    threadpool->livenum = minnum;//不是0
    threadpool->exitnum = 0;
    pthread_mutex_init(&threadpool->mutexbusy, nullptr);
    pthread_mutex_init(&threadpool->mutexpool, nullptr);
    pthread_cond_init(&threadpool->notempty, nullptr);
    pthread_cond_init(&threadpool->notfull, nullptr);
    threadpool->shutdown = false;
    threadpool->step = step;
    pthread_create(&threadpool->manageid, nullptr, manager, threadpool);
    for (int i = 0; i < minnum; i++) {
        pthread_create(&threadpool->workerid[i], nullptr, worker, threadpool);

        pthread_detach(threadpool->workerid[i]);
        std::cout << "thread create success,id:" << threadpool->workerid[i] << std::endl;
    }
    return threadpool;
}
void* worker(void* args) {
    ThreadPool* pool = (ThreadPool*)args;
    while (1) {
        pthread_mutex_lock(&pool->mutexpool);
        while (pool->queuesize == 0 && !pool->shutdown) {//防止虚假唤醒
            pthread_cond_wait(&pool->notempty, &pool->mutexpool);
            //这里是管理者线程回收需要
            if (pool->exitnum > 0) {
                pool->exitnum--;//不可放到下面if中去，因为if判断为否它也得减一
                if (pool->livenum > pool->minnum) {//二次判断，因为之前管理者线程做判断用的是取出了一段时间的livenum，可能此时已经改变了，在判断一次防止杀掉过多线程。
                    pool->livenum--;
                    pthread_mutex_unlock(&pool->mutexpool);
                    thread_exit(pool);
                }

            }
            //这里结束

        }
        if (pool->shutdown) {//这就是线程池如何关闭的
            pthread_mutex_unlock(&pool->mutexpool);
            thread_exit(pool);

        }

        Task task;
        task.function = pool->taskq[pool->queuefront].function;
        task.args = pool->taskq[pool->queuefront].args;
        pool->queuefront = (pool->queuefront + 1) % pool->queuecapacity;
        pool->queuesize--;//
        //这里是后加的，唤醒生产者操作
        pthread_cond_signal(&pool->notfull);

        //这里结束
        pthread_mutex_unlock(&pool->mutexpool);
        pthread_mutex_lock(&pool->mutexbusy);
        pool->busynum++;
        pthread_mutex_unlock(&pool->mutexbusy);
        task.function(task.args);
        pthread_mutex_lock(&pool->mutexbusy);
        pool->busynum--;
        pthread_mutex_unlock(&pool->mutexbusy);


    }
    return nullptr;

}
void* manager(void* args) {
    ThreadPool* pool = (ThreadPool*)args;
    while (!pool->shutdown) {
        sleep(4);
        if (pool->shutdown) {
            pthread_exit(nullptr);

        }
        pthread_mutex_lock(&pool->mutexpool);
        int busynum = pool->busynum;//销毁时候看忙线程，创建时候看活线程
        int livenum = pool->livenum;//这里时候要这样写？因为后面要对这个值进行更gai
        int size = pool->queuesize;
        pthread_mutex_unlock(&pool->mutexpool);
        if (size > livenum && livenum < pool->maxnum) {
            cout << "\n\n\nfuzhiyici\n\n\n";
            pthread_mutex_lock(&pool->mutexpool);
            int count = 0;
            for (int i = 0; i < pool->maxnum && count < pool->step && pool->livenum < pool->maxnum; i++) {
                if (pool->workerid[i] == 0) {
                    count += 1;
                    pthread_create(&pool->workerid[i], nullptr, worker, pool);
                    pthread_detach(pool->workerid[i]);
                    pool->livenum++;
                }

            }
            pthread_mutex_unlock(&pool->mutexpool);
        }
        if (busynum<livenum / 2 && livenum>pool->minnum) {
            pthread_mutex_lock(&pool->mutexpool);
            pool->exitnum = pool->step;
            pthread_mutex_unlock(&pool->mutexpool);
            for (int i = 0; i < pool->step; i++) {
                pthread_cond_signal(&pool->notempty);
            }


        }








    }


    return nullptr;
}
void producer(ThreadPool* pool, void(*fun)(void*), void* args) {
    pthread_mutex_lock(&pool->mutexpool);
    while (pool->queuecapacity == pool->queuesize && !pool->shutdown) {


        pthread_cond_wait(&pool->notfull, &pool->mutexpool);

    }
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->mutexpool);
        return;
    }
    cout << "queuesize and queuecapacity" << pool->queuesize << pool->queuecapacity << "\n";


    pool->taskq[pool->queuerear].args = args;
    pool->taskq[pool->queuerear].function = fun;
    pool->queuerear++;
    pool->queuerear %= pool->queuecapacity;//是百分之而不是除以********
    pool->queuesize++;

    int a = pthread_cond_signal(&pool->notempty);
    if (a != 0)cout << "signal error\n";
    pthread_mutex_unlock(&pool->mutexpool);

}
int get_busy_num(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexbusy);
    int a = pool->busynum;
    pthread_mutex_unlock(&pool->mutexbusy);
    return a;
}

int get_live_num(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexpool);
    int a = pool->livenum;
    pthread_mutex_unlock(&pool->mutexpool);
    return a;
}
void threadpool_destroy(ThreadPool* pool) {
    std::cout << "destroy start\n";
    if (pool == nullptr) {
        return;
    }
    pool->shutdown = true;
    pthread_join(pool->manageid, nullptr);//要回收
    for (int i = 0; i < pool->livenum; i++) {
        pthread_cond_signal(&pool->notempty);

    }
    if (pool->taskq) {
        free(pool->taskq);
    }
    if (pool->workerid) {
        free(pool->workerid);
    }
    pthread_mutex_destroy(&pool->mutexpool);
    pthread_mutex_destroy(&pool->mutexbusy);
    pthread_cond_destroy(&pool->notempty);
    pthread_cond_destroy(&pool->notfull);
    if (pool) {
        free(pool);
    }
    std::cout << "destory finish\n";
}*/
/*

#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>
#include<stdio.h>
#define NUMBER 4
// 任务结构体
typedef struct Task
{
    void (*function)(void* arg);
    void* arg;
}Task;

// 线程池结构体
struct ThreadPool
{
    // 任务队列
    Task* taskQ;
    int queueCapacity;  // 容量
    int queueSize;      // 当前任务个数
    int queueFront;     // 队头 -> 取数据
    int queueRear;      // 队尾 -> 放数据

    pthread_t managerID;    // 管理者线程ID
    pthread_t* threadIDs;   // 工作的线程ID
    int minNum;             // 最小线程数量
    int maxNum;             // 最大线程数量
    int busyNum;            // 忙的线程的个数
    int liveNum;            // 存活的线程的个数
    int exitNum;            // 要销毁的线程个数
    pthread_mutex_t mutexPool;  // 锁整个的线程池
    pthread_mutex_t mutexBusy;  // 锁busyNum变量
    pthread_cond_t notFull;     // 任务队列是不是满了
    pthread_cond_t notEmpty;    // 任务队列是不是空了

    int shutdown;           // 是不是要销毁线程池, 销毁为1, 不销毁为0
};
void* worker(void* arg);
// 管理者线程任务函数
void* manager(void* arg);
// 单个线程退出
void threadExit(ThreadPool* pool);


ThreadPool* threadPoolCreate(int min, int max, int queueSize)
{
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    do
    {
        if (pool == NULL)
        {
            printf("malloc threadpool fail...\n");
            break;
        }

        pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t) * max);
        if (pool->threadIDs == NULL)
        {
            printf("malloc threadIDs fail...\n");
            break;
        }
        memset(pool->threadIDs, 0, sizeof(pthread_t) * max);
        pool->minNum = min;
        pool->maxNum = max;
        pool->busyNum = 0;
        pool->liveNum = min;    // 和最小个数相等
        pool->exitNum = 0;

        if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
            pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
            pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
            pthread_cond_init(&pool->notFull, NULL) != 0)
        {
            printf("mutex or condition init fail...\n");
            break;
        }

        // 任务队列
        pool->taskQ = (Task*)malloc(sizeof(Task) * queueSize);
        pool->queueCapacity = queueSize;
        pool->queueSize = 0;
        pool->queueFront = 0;
        pool->queueRear = 0;

        pool->shutdown = 0;

        // 创建线程
        pthread_create(&pool->managerID, NULL, manager, pool);
        for (int i = 0; i < min; ++i)
        {
            pthread_create(&pool->threadIDs[i], NULL, worker, pool);
            pthread_detach(pool->threadIDs[i]);
        }
        return pool;
    } while (0);

    // 释放资源
    if (pool && pool->threadIDs) free(pool->threadIDs);
    if (pool && pool->taskQ) free(pool->taskQ);
    if (pool) free(pool);

    return NULL;
}

int threadPoolDestroy(ThreadPool* pool)
{
    if (pool == NULL)
    {
        return -1;
    }

    // 关闭线程池
    pool->shutdown = 1;
    // 阻塞回收管理者线程
    pthread_join(pool->managerID, NULL);
    // 唤醒阻塞的消费者线程
    for (int i = 0; i < pool->liveNum; ++i)
    {
        pthread_cond_signal(&pool->notEmpty);
    }
    // 释放堆内存
    if (pool->taskQ)
    {
        free(pool->taskQ);
    }
    if (pool->threadIDs)
    {
        free(pool->threadIDs);
    }

    pthread_mutex_destroy(&pool->mutexPool);
    pthread_mutex_destroy(&pool->mutexBusy);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);

    free(pool);
    pool = NULL;

    return 0;
}


void threadPoolAdd(ThreadPool* pool, void(*func)(void*), void* arg)
{
    pthread_mutex_lock(&pool->mutexPool);
    while (pool->queueSize == pool->queueCapacity && !pool->shutdown)
    {
        // 阻塞生产者线程
        pthread_cond_wait(&pool->notFull, &pool->mutexPool);
    }

    if (pool->shutdown)
    {
        pthread_mutex_unlock(&pool->mutexPool);
        return;
    }
    // 添加任务
    pool->taskQ[pool->queueRear].function = func;
    pool->taskQ[pool->queueRear].arg = arg;
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
    pool->queueSize++;
    printf("size %d \n", pool->queueSize);
    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->mutexPool);
}

int threadPoolBusyNum(ThreadPool* pool)
{
    pthread_mutex_lock(&pool->mutexBusy);
    int busyNum = pool->busyNum;
    pthread_mutex_unlock(&pool->mutexBusy);
    return busyNum;
}

int threadPoolAliveNum(ThreadPool* pool)
{
    pthread_mutex_lock(&pool->mutexPool);
    int aliveNum = pool->liveNum;
    pthread_mutex_unlock(&pool->mutexPool);
    return aliveNum;
}

void* worker(void* arg)
{
    ThreadPool* pool = (ThreadPool*)arg;

    while (1)
    {
        pthread_mutex_lock(&pool->mutexPool);
        // 当前任务队列是否为空
        while (pool->queueSize == 0 && !pool->shutdown)
        {
            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

            // 判断是不是要销毁线程
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum)
                {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    threadExit(pool);
                }
            }
        }

        // 判断线程池是否被关闭了
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexPool);
            threadExit(pool);
        }

        // 从任务队列中取出一个任务
        Task task;
        task.function = pool->taskQ[pool->queueFront].function;
        task.arg = pool->taskQ[pool->queueFront].arg;
        // 移动头结点
        pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
        pool->queueSize--;
        // 解锁
        pthread_cond_signal(&pool->notFull);
        pthread_mutex_unlock(&pool->mutexPool);

        printf("thread %ld start working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexBusy);
        task.function(task.arg);
        free(task.arg);
        task.arg = NULL;

        printf("thread %ld end working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexBusy);
    }
    return NULL;
}

void* manager(void* arg)
{
    ThreadPool* pool = (ThreadPool*)arg;
    while (!pool->shutdown)
    {
        // 每隔3s检测一次
        sleep(3);

        // 取出线程池中任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->queueSize;
        int liveNum = pool->liveNum;
        pthread_mutex_unlock(&pool->mutexPool);

        // 取出忙的线程的数量
        pthread_mutex_lock(&pool->mutexBusy);
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->mutexBusy);

        // 添加线程
        // 任务的个数>存活的线程个数 && 存活的线程数<最大线程数
        if (queueSize > liveNum && liveNum < pool->maxNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            int counter = 0;
            for (int i = 0; i < pool->maxNum && counter < NUMBER
                && pool->liveNum < pool->maxNum; ++i)
            {
                if (pool->threadIDs[i] == 0)
                {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    counter++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }
        // 销毁线程
        // 忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数
        if (busyNum * 2 < liveNum && liveNum > pool->minNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexPool);
            // 让工作的线程自杀
            for (int i = 0; i < NUMBER; ++i)
            {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return NULL;
}

void threadExit(ThreadPool* pool)
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < pool->maxNum; ++i)
    {
        if (pool->threadIDs[i] == tid)
        {
            pool->threadIDs[i] = 0;
            printf("threadExit() called, %ld exiting...\n", tid);
            break;
        }
    }
    pthread_exit(NULL);
}




*/

Threadpool::Threadpool(int min,int max)
{
    do {
        taskq = new Taskq;
        pthread_mutex_init(&mutexpool, NULL);
        pthread_cond_init(&notempty, NULL);
        minnum = min;
        maxnum = max;
        busynum = 0;
        exitnum = 0;
        livenum = min;
        shutdown = false;
        workerid = new pthread_t[max];
        pthread_create(&managerid, nullptr, manager, this);
        for (int i = 0; i < min; i++) {
            pthread_create(&workerid[i], nullptr, worker, this);
        }
    } while (0);
    if (workerid)delete[]workerid;
    if (taskq) delete taskq;
}

Threadpool::~Threadpool()
{
    shutdown = true;
    pthread_join(managerid, NULL);
    for (int i = 0; i < livenum; i++) {
        pthread_cond_signal(&notempty);

    }
    if (taskq)delete taskq;
    if (workerid)delete[]workerid;
    pthread_mutex_destroy(&mutexpool);
    pthread_cond_destroy(&notempty);
    
}

void Threadpool::addtasktotaskq(Task task)//不用加锁
{
    if (shutdown) {
    
        return;
    }
    taskq->addtask(task);
    pthread_cond_signal(&notempty);//这一行不要忘了
    std::cout << taskq->getnum()<<"#######\n";

    return;
}

int Threadpool::getbusynum()
{
    pthread_mutex_lock(&mutexpool);
    int a = busynum;
    pthread_mutex_unlock(&mutexpool);
    return a;
}

int Threadpool::getalivenum()
{
    pthread_mutex_lock(&mutexpool);
    int a = livenum;
    pthread_mutex_unlock(&mutexpool);
    return a;
}

void* Threadpool::worker(void* arg)
{
    Threadpool* pool = static_cast<Threadpool*>(arg);
    while (true) {
        pthread_mutex_lock(&pool->mutexpool);
        while (pool->taskq->getnum()==0 and !pool->shutdown) {
            pthread_cond_wait(&pool->notempty, &pool->mutexpool);
            if (pool->exitnum > 0) {
                pool->exitnum--;
                if (pool->livenum > pool->minnum) {
                    pool->livenum--;
                    pthread_mutex_unlock(&pool->mutexpool);
                    pool->threadexit();
                }

            }

        }
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->mutexpool);
            pool->threadexit();
        }
        Task task = pool->taskq->gettask();
        pool->busynum++;
        pthread_mutex_unlock(&pool->mutexpool);

        task.f(task.arg);//这里暂时没加free
        pthread_mutex_lock(&pool->mutexpool);
        pool->busynum--;
        pthread_mutex_unlock(&pool->mutexpool);
    }

    return nullptr;
}

void* Threadpool::manager(void* arg)
{
    Threadpool* pool = static_cast<Threadpool*>(arg);
    while (!pool->shutdown)
    {
        sleep(3);
        pthread_mutex_lock(&pool->mutexpool);
        int livenum = pool->livenum;
        int busynum = pool->busynum;
        int queuesize = pool->taskq->getnum();
        pthread_mutex_unlock(&pool->mutexpool);
        if (livenum < queuesize && livenum < pool->maxnum) {
            int count = 0;
            for (int i = 0; i < pool->maxnum && count < pool->step && pool->livenum < pool->maxnum; i++) {
                if (pool->workerid[i] == 0) {
                    pthread_create(&pool->workerid[i], nullptr, worker, pool);
                    pool->livenum++;
                    count++;

                }
            }
        }

        if (busynum < queuesize / 2 && livenum > pool->maxnum) {

            pthread_mutex_lock(&pool->mutexpool);
            pool->exitnum = step;
            pthread_mutex_unlock(&pool->mutexpool);
            for (int i = 0; i < step; i++) {
                pthread_cond_signal(&pool->notempty);
            }

        }
    }
    return nullptr;
}

void Threadpool::threadexit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < maxnum; i++) {
        if (tid == workerid[i]) {
            workerid[i] = 0;
            break;
        }

    }
    pthread_exit(NULL);

}
