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
// �����̳߳ز���ʼ��
ThreadPool* threadPoolCreate(int min, int max, int queueSize);

// �����̳߳�
int threadPoolDestroy(ThreadPool* pool);

// ���̳߳��������
void threadPoolAdd(ThreadPool* pool, void(*func)(void*), void* arg);

// ��ȡ�̳߳��й������̵߳ĸ���
int threadPoolBusyNum(ThreadPool* pool);

// ��ȡ�̳߳��л��ŵ��̵߳ĸ���
int threadPoolAliveNum(ThreadPool* pool);

//////////////////////
// �������߳�(�������߳�)������
void* worker(void* arg);
// �������߳�������
void* manager(void* arg);
// �����߳��˳�
void threadExit(ThreadPool* pool);
#endif  // _THREADPOOL_H



*/