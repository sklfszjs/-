#include"Taskq.h"
class Threadpool {
public:
	Threadpool(int min, int max);
	~Threadpool();
	void addtasktotaskq(Task task);
	int getbusynum();
	int getalivenum();




private:
	Taskq *taskq;
	pthread_mutex_t mutexpool;
	pthread_cond_t notempty;
	int busynum;
	int livenum;
	int exitnum;
	int minnum;
	int maxnum;
	bool shutdown;
	static const int step = 5;
	static void* worker(void*arg);
	static void *manager(void* arg);
	void threadexit();
	pthread_t managerid;
	pthread_t* workerid;

};

/*
#ifndef _THREADPOOL_H
#define _THREADPOOL_H
struct Task;
struct ThreadPool;
void thread_exit(ThreadPool* pool);
ThreadPool* threadpool_create(int minnum, int maxnum, int queuecapacity, int step = 5);
void* worker(void* args);
void* manager(void* args);
void producer(ThreadPool* pool, void(*fun)(void*), void* args);
int get_busy_num(ThreadPool* pool);
int get_live_num(ThreadPool* pool);
void threadpool_destroy(ThreadPool* pool);

#endif // !_THREADPOOL_H
*/

/*#ifndef _THREADPOOL_H
#define _THREADPOOL_H

typedef struct ThreadPool ThreadPool;
// 创建线程池并初始化
ThreadPool* threadPoolCreate(int min, int max, int queueSize);

// 销毁线程池
int threadPoolDestroy(ThreadPool* pool);

// 给线程池添加任务
void threadPoolAdd(ThreadPool* pool, void(*func)(void*), void* arg);

// 获取线程池中工作的线程的个数
int threadPoolBusyNum(ThreadPool* pool);

// 获取线程池中活着的线程的个数
int threadPoolAliveNum(ThreadPool* pool);

//////////////////////
// 工作的线程(消费者线程)任务函数
void* worker(void* arg);
// 管理者线程任务函数
void* manager(void* arg);
// 单个线程退出
void threadExit(ThreadPool* pool);
#endif  // _THREADPOOL_H



*/