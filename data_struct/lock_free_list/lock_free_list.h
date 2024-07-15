//
// Created by codelover on 2024/7/14.
//

#ifndef CRPC_LOCK_FREE_LIST_H
#define CRPC_LOCK_FREE_LIST_H
#include <atomic>
#include <iostream>

template <typename T>
class LockFreeList {
public:
    LockFreeList() {
        Node* node = new Node();
        node->next = nullptr;
        head.store(node);
        tail.store(node);
    }

    ~LockFreeList() {
        T output;
        while (Dequeue(output)) {
        }
        delete head.load();
    }

    /*
     * https://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
     */
    void Enqueue(const T &value) {
        Node* newNode = new Node(value);
        Node* tailPtr;
        while (true) {
            tailPtr = tail.load();
            Node* nextPtr = tailPtr->next.load();
            if (tailPtr != tail.load()) {
                continue;
            }
            if (nextPtr != nullptr) {
                continue;
            }
            if (tailPtr->next.compare_exchange_strong(nextPtr, newNode)) {
                break;
            }
        }
        tail.compare_exchange_strong(tailPtr, newNode);
    }

    bool Dequeue(T &output) {
        Node *headPtr = head.load();
        Node *dequeuePtr;
        Node *nextPtr;
        while (true){
            dequeuePtr = headPtr->next.load();
            if (dequeuePtr == nullptr){
                return false;
            }
            if (headPtr->next != dequeuePtr){
                continue;
            }
            nextPtr = dequeuePtr->next.load();
            if(headPtr->next.compare_exchange_strong(dequeuePtr, nextPtr)){
                break;
            }
        }
        output = dequeuePtr->value;
        delete dequeuePtr;
        return true;
    }

private:
    struct Node {
        T value;
        std::atomic<Node *> next;
        explicit Node(const T &val = T()) : value(val), next(nullptr) {}
    };

    std::atomic<Node *> head;
    std::atomic<Node *> tail;
};

#endif //CRPC_LOCK_FREE_LIST_H
