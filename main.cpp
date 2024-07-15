#include "data_struct/lock_free_list/lock_free_list.h"
#include <iostream>
#include <thread>
#include <vector>

const int NUM_THREADS = 4;
const int NUM_OPERATIONS = 1000000;

void enqueue_test(LockFreeList<int>& list, int index) {
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        list.Enqueue(index*1000000 + i);
    }
    std::cout<<"end eq"<< std::endl;
}

void dequeue_test(LockFreeList<int>& list) {
    int value;
    while (list.Dequeue(value)) {
        // Keep trying until we successfully dequeue a value
        std::cout << "dq Remaining value: " << value << std::endl;
    }
    std::cout<<"end de"<< std::endl;
}

int main() {
    LockFreeList<int> list;

    // Create threads for enqueue and dequeue tests
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(enqueue_test, std::ref(list), i);
    }
//    threads.emplace_back(dequeue_test, std::ref(list));
    // Wait for all threads to finish
    for (auto& t: threads) {
        t.join();
    }
    std::cout << "all eq end " << std::endl;
//    getchar();
    // Check if there are any remaining elements in the list
    int value;
    int count = 0;
    while (list.Dequeue(value)) {
        count++;
//        std::cout << "Remaining value: " << value << std::endl;
    }
        std::cout << "Remaining value: " << count << std::endl;
    return 0;
}