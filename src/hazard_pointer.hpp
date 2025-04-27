#pragma once

#include <benchmark/benchmark.h>
#include <atomic>
#include <memory>
#include <mutex>


struct Node {
    int value;
    Node* next;

    Node(int val) : value(val), next(nullptr) {}
};


template <typename T>
class HazardPointer {
private:
    std::atomic<T*> hazardPointer;
public:
    HazardPointer() : hazardPointer(nullptr) {}

    void protect(T* ptr) {
        hazardPointer.store(ptr, std::memory_order_release);
    }

    void clear() {
        hazardPointer.store(nullptr, std::memory_order_release);
    }

    T* get() {
        return hazardPointer.load(std::memory_order_acquire);
    }

};


class LockFreeList {
private:
    std::atomic<Node*> head;
    HazardPointer<Node> hp;
public:
    LockFreeList() : head(nullptr) {}

    void insert(int value) {
        Node* newNode = new Node(value);
        Node* oldHead = head.load(std::memory_order_relaxed);
        do {
            newNode->next = oldHead;
        } while (!head.compare_exchange_weak(oldHead, newNode, std::memory_order_release, std::memory_order_relaxed));
    }

    void deleteNode(Node* nodeToDelete) {
        hp.protect(nodeToDelete);

        Node* current = head.load(std::memory_order_acquire);
        while (current != nullptr) {
            if (current == nodeToDelete) {
                delete current;
                hp.clear();
                break;
            }
            current = current->next;
        }
    }

    Node* getHead() const {
        return head.load(std::memory_order_acquire);
    }
};


class LockingList {
private:
    Node* head;
    std::mutex listMutex;
public:
    LockingList() : head(nullptr) {}

    void insert(int value) {
        std::lock_guard<std::mutex> lock(listMutex);
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }

    void deleteNode(Node* nodeToDelete) {
        std::lock_guard<std::mutex> lock(listMutex);
        Node* current = head;
        Node* prev = nullptr;
        while (current != nullptr) {
            if (current == nodeToDelete) {
                if (prev == nullptr) {
                    head = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    Node* getHead() {
        std::lock_guard<std::mutex> lock(listMutex);
        return head;
    }
};



class HazardPointerBenchmark : public benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State& state) override {

    }

    void TearDown(const ::benchmark::State& state) override {

    }

    const int nInserts = 1000;
    const int nThreads = 8;
};

BENCHMARK_F(HazardPointerBenchmark, Locking)(benchmark::State& state) {
    LockingList list;

    auto run_insert_delete = [&list, this]() {
        for (int i = 0; i < nInserts; ++i) {
            list.insert(42);
            Node* headNode = list.getHead();
            if (headNode) {
                list.deleteNode(headNode);
            }
        }
    };

    std::vector<std::thread> threads;
    for (auto _ : state) {
        threads.clear();
        for (int i = 0; i < nThreads; ++i) {
            threads.emplace_back(run_insert_delete);
        }
        for (auto& t : threads) {
            t.join();
        }
    }
}


BENCHMARK_F(HazardPointerBenchmark, NonLocking)(benchmark::State& state) {
    LockFreeList list;
    auto run_insert_delete = [&list, this]() {
        for (int i = 0; i < nInserts; ++i) {
            list.insert(42);
            Node* headNode = list.getHead();
            if (headNode) {
                list.deleteNode(headNode);
            }
        }
    };

    std::vector<std::thread> threads;
    for (auto _ : state) {
        threads.clear();
        for (int i = 0; i < nThreads; ++i) {
            threads.emplace_back(run_insert_delete);
        }
        for (auto& t : threads) {
            t.join();
        }
    }
}
