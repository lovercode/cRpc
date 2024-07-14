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
        head = new Node();
        pre = head.load();
        tail = new Node();
        head.load()->next.store(tail.load());
    }

    ~LockFreeList() {
        T output;
        while (Dequeue(output)) {
        }
        delete head.load();
    }

    void Enqueue(const T &value) {
        // h 1 2 3
        Node *newNode = new Node(value);
        newNode->next.store(tail.load());
        Node *prevTail;
        Node *tailP = tail.load();
        do {
            prevTail = pre.load();
        } while (!prevTail->next.compare_exchange_weak(tailP, newNode));
        pre.compare_exchange_strong(prevTail, newNode);
    }

    bool Dequeue(T &output) {
        Node *prevHead;
        do {
            prevHead = head.load();
            if (prevHead->next.load() == tail.load()) {
                return false;
            }
        } while (!head.compare_exchange_weak(prevHead, prevHead->next.load()));
        output = prevHead->next.load()->value;
        delete prevHead;
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
    std::atomic<Node *> pre;
};

#endif //CRPC_LOCK_FREE_LIST_H
