#pragma once

#include <atomic>
#include <cstdio>
#include <iostream>
#include "hazard_pointer_raii.hpp"


namespace bhh {

template <typename T>
class ListReclaimer;


template<typename T>
class LockFreeLinkedList {
    static_assert(std::is_copy_constructible_v<T>, "T requires copy constructor");
    friend ListReclaimer<T>;
    struct Node;

public:
    LockFreeLinkedList() : head_(new Node()), size_(0) {}
    LockFreeLinkedList(const LockFreeLinkedList& other) = delete;
    LockFreeLinkedList(LockFreeLinkedList&& other) = delete;
  
    LockFreeLinkedList& operator=(const LockFreeLinkedList& other) = delete;
    LockFreeLinkedList& operator=(LockFreeLinkedList&& other) = delete;

    ~LockFreeLinkedList() {
        Node* p = head_;
        while (p != nullptr) {
            Node* tmp = p;
            p = p->next.load(std::memory_order_acquire);
            delete tmp;
        }
    }

private:
    Node* head_;
    std::atomic<size_t> size_;
    static Reclaimer::HazardPointerList global_hp_list_;
};

}