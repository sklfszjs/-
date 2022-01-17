#include "Taskq.h"

Taskq::Taskq()
{
	pthread_mutex_init(&mutextaskq,NULL);

}

Taskq::~Taskq()
{
	pthread_mutex_destroy(&mutextaskq);
}

void Taskq::addtask(Task task)
{
	pthread_mutex_lock(&mutextaskq);
	m_taskq.push(task);
	pthread_mutex_unlock(&mutextaskq);

}

void Taskq::addtask(callback f, void* arg)
{
	pthread_mutex_lock(&mutextaskq);
	m_taskq.push(Task(f,arg));
	pthread_mutex_unlock(&mutextaskq);
}

Task Taskq::gettask()
{
	Task t;
	pthread_mutex_lock(&mutextaskq);
	if (m_taskq.empty()) {
		pthread_mutex_unlock(&mutextaskq);
		return t;
	}
	t = m_taskq.front();
	m_taskq.pop();
	pthread_mutex_unlock(&mutextaskq);
	return t;
}
