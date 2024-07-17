//
// Created by codelover on 2024/7/14.
//

#ifndef CRPC_LOCK_FREE_LIST_H
#define CRPC_LOCK_FREE_LIST_H
#include <atomic>
#include <iostream>

/*
 * 队列
 */
template <typename T>
class Queue {
public:
    virtual void Enqueue(const T &value) = 0;
    virtual bool Dequeue(T &output) = 0;
    virtual ~Queue() {}; // 声明为虚函数，不然基类指针析构的时候无法释放子类的空间
};




template <typename T>
class LockFreeQueue: public Queue<T>{
private:
    struct Node {
        T data;
        std::atomic<Node*> next;

        Node(T value) : data(value), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;

public:
    LockFreeQueue() {
        Node* dummy = new Node(T());
        head.store(dummy);
        tail.store(dummy);
    }

    ~LockFreeQueue() {
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }

    void Enqueue(const T& value) {
        Node* newNode = new Node(value);
        Node* oldTail = nullptr;

        while (true) {
            oldTail = tail.load();
            Node* next = oldTail->next.load();

            if (oldTail == tail.load()) {
                if (next == nullptr) {
                    if (oldTail->next.compare_exchange_weak(next, newNode)) {
                        break;
                    }
                } else {
                    tail.compare_exchange_weak(oldTail, next);
                }
            }
        }

        tail.compare_exchange_weak(oldTail, newNode);
    }

    bool Dequeue(T &output){
        Node* oldHead = nullptr;
        while (true) {
            oldHead = head.load();
            Node* oldTail = tail.load();
            Node* next = oldHead->next.load();

            if (oldHead == head.load()) {
                if (oldHead == oldTail) {
                    if (next == nullptr) {
                        return false;
                    }
                    tail.compare_exchange_weak(oldTail, next);
                } else {
                    if (next != nullptr) {
                        if (head.compare_exchange_weak(oldHead, next)) {
                            output = oldHead->data;
                            delete oldHead;
                            return true;
                        }
                    }
                }
            }
        }
    }
};

/*
 * 无锁队列的实现
 */
template <typename T>
class LockFreeList: public Queue<T> {
public:
    LockFreeList() {
        Node* node = new Node();
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
     * enqueue(Q: pointer to queue_t, value: data type)
   E1:   node = new_node()	// Allocate a new node from the free list
   E2:   node->value = value	// Copy enqueued value into node
   E3:   node->next.ptr = NULL	// Set next pointer of node to NULL
   E4:   loop			// Keep trying until Enqueue is done
   E5:      tail = Q->Tail	// Read Tail.ptr and Tail.count together
   E6:      next = tail.ptr->next	// Read next ptr and count fields together
   E7:      if tail == Q->Tail	// Are tail and next consistent?
               // Was Tail pointing to the last node?
   E8:         if next.ptr == NULL
                  // Try to link node at the end of the linked list
   E9:            if CAS(&tail.ptr->next, next, <node, next.count+1>)
  E10:               break	// Enqueue is done.  Exit loop
  E11:            endif
  E12:         else		// Tail was not pointing to the last node
                  // Try to swing Tail to the next node
  E13:            CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
  E14:         endif
  E15:      endif
  E16:   endloop
         // Enqueue is done.  Try to swing Tail to the inserted node
  E17:   CAS(&Q->Tail, tail, <node, tail.count+1>)
     */
    void Enqueue(const T &value) {
        Node* node = new Node(value);
        Node* tailPtr;
        Node* nextPtr;
        while (true){
            tailPtr = tail.load();
            nextPtr = tailPtr->next.load();
            if (tailPtr != tail.load()){
                continue;
            }
            if (nextPtr != nullptr){
                continue;
            }
            if (tail.load()->next.compare_exchange_strong(nextPtr, node)){
                break;
            }
        }
        tail.compare_exchange_strong(tailPtr, node);
    }

    void Enqueue1(const T &value) {
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

    /*
     *   dequeue(Q: pointer to queue_t, pvalue: pointer to data type): boolean
   D1:   loop			     // Keep trying until Dequeue is done
   D2:      head = Q->Head	     // Read Head
   D3:      tail = Q->Tail	     // Read Tail
   D4:      next = head.ptr->next    // Read Head.ptr->next
   D5:      if head == Q->Head	     // Are head, tail, and next consistent?
   D6:         if head.ptr == tail.ptr // Is queue empty or Tail falling behind?
   D7:            if next.ptr == NULL  // Is queue empty?
   D8:               return FALSE      // Queue is empty, couldn't dequeue
   D9:            endif
                  // Tail is falling behind.  Try to advance it
  D10:            CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
  D11:         else		     // No need to deal with Tail
                  // Read value before CAS
                  // Otherwise, another dequeue might free the next node
  D12:            *pvalue = next.ptr->value
                  // Try to swing Head to the next node
  D13:            if CAS(&Q->Head, head, <next.ptr, head.count+1>)
  D14:               break             // Dequeue is done.  Exit loop
  D15:            endif
  D16:         endif
  D17:      endif
  D18:   endloop
  D19:   free(head.ptr)		     // It is safe now to free the old node
  D20:   return TRUE                   // Queue was not empty, dequeue succeeded
     */
    bool Dequeue(T &output) {
        Node *headPtr;
        Node *nextPtr;
        while (true){
            headPtr = head.load();
            nextPtr = headPtr->next.load();
            if (headPtr != head.load()){
                continue;
            }
            if (nextPtr == nullptr){
                return false;
            }
            output = nextPtr->value;
            if (head.compare_exchange_strong(headPtr, nextPtr)){
                break;
            }
        }
        delete headPtr;
        return true;
    }


    bool Dequeue1(T &output) {
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
