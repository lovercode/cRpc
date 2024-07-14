//
// Created by codelover on 2024/7/14.
//

#ifndef CRPC_THREAD_POOL_H
#define CRPC_THREAD_POOL_H


class ThreadTask {

};

class ThreadPool {
public:
    ThreadPool(int thread_num);
    AddTask(const ThreadTask& task);
private:
    int thread_num;
};


#endif //CRPC_THREAD_POOL_H
