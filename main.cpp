#include "data_struct/lock_free_list/lock_free_list.h"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
std::atomic<int> count(0);
std::atomic<int> incount(0);

const int NUM_THREADS = 1;
const int NUM_OPERATIONS = 10000000;

void enqueue_test(Queue<int>* list, int index) {
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        list->Enqueue(index*NUM_OPERATIONS + i);
        incount.fetch_add(1);
    }
}

void dequeue_test(Queue<int>* list) {
    int value;
    while (list->Dequeue(value)) {
        count.fetch_add(1);
    }
}

int main() {
    Queue<int>* list = new LockFreeList<int>();
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(enqueue_test, std::ref(list), i);
        threads.emplace_back(dequeue_test, std::ref(list));
    }

    for (auto& t: threads) {
        t.join();
    }

//    threads.clear();
//    threads.emplace_back(dequeue_test, std::ref(list));
//    threads.emplace_back(dequeue_test, std::ref(list));
//    for (auto& t: threads) {
//        t.join();
//    }
    dequeue_test(list);
    std::cout << "all in: " << incount.load() << " all out: " << count.load() << std::endl;
    delete list;
    return 0;
}