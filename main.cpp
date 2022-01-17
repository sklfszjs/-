
#include"Threadpool.h"
#include <stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>

#include<iostream>
void fun(void* arg) {
    int* num = (int*)arg;
    std::cout << "the number is " << *num << std::endl;
    sleep(1);
}
int main()
{

    Threadpool pool(3, 20);
    sleep(5);
    for (int i = 0; i < 20; i++) {
        int* arg = (int*)malloc(sizeof(int));

        *arg = i;
        int* num = (int*)((void*)arg);
        std::cout << "main the number is " << *num << std::endl;
        pool.addtasktotaskq(Task(fun, num));
        sleep(1);

    }
    std::cout << "创建完了\n";

    sleep(2000);
    return 0;}
/*
#include <stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include "Threadpool.h"
#include<iostream>
void fun(void* arg) {
    int* num = (int*)arg;
    std::cout << "the number is " << *num << std::endl;
    sleep(1);
}
int main()
{

    ThreadPool* pool = threadpool_create(5, 10, 15, 10);
    sleep(5);
    for (int i = 0; i < 20; i++) {
        int* arg = (int*)malloc(sizeof(int));

        *arg = i;
        int* num = (int*)((void*)arg);
        std::cout << "main the number is " << *num << std::endl;
        producer(pool, fun, (void*)arg);
        sleep(1);

    }
    std::cout << "创建完了\n";
    sleep(20);
    threadpool_destroy(pool);
    sleep(2000);
    return 0;
}*/
/*#include <stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include "Threadpool.h"
#include<iostream>




void taskFunc(void* arg)
{
    int num = *(int*)arg;
    printf("thread %ld is working, number = %d\n",
        pthread_self(), num);
    sleep(1);
}

int main()
{
    // 创建线程池
    ThreadPool* pool = threadPoolCreate(3, 10, 100);
    for (int i = 0; i < 100; ++i)
    {
        int* num = (int*)malloc(sizeof(int));
        *num = i + 100;
        threadPoolAdd(pool, taskFunc, num);

    }

    sleep(3000);

    threadPoolDestroy(pool);
    return 0;
}

*/