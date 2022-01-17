#pragma once
#include<queue>
#include<pthread.h>
using callback = void(*)(void* arg);
struct Task {
	callback f;
	void* arg;

	Task() {
		f = nullptr;
		arg = nullptr;
	}

	Task(callback fun,void *args) {
		f =fun;
		arg = args;
	}

};
class Taskq
{
public:
	Taskq();
	~Taskq();
	void addtask(Task task);
	void addtask(callback f,void * arg);
	Task gettask();
	int getnum() {
		pthread_mutex_lock(&mutextaskq);
		int a = m_taskq.size();
		

		pthread_mutex_unlock(&mutextaskq);

		return a;//这里不能写在unlock前面
	}
private:
	pthread_mutex_t mutextaskq;
	std::queue<Task> m_taskq;
};

