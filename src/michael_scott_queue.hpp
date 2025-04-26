#pragma once

#include <benchmark/benchmark.h>
#include <atomic>
#include <thread>
#include <memory>
#include <queue>
#include <lock_free.hpp>


template<typename T>
struct MSQueueNode
{
    T data;
    std::atomic<MSQueueNode<T>*> next;

    MSQueueNode(const T& value) : data(value), next(nullptr) {}
};


template<typename T>
class MSQueue
{
public:
    MSQueue() : head(nullptr), tail(nullptr)
    {
        MSQueueNode<T>* dummy = new MSQueueNode<T>(T());
        head.store(dummy, std::memory_order_relaxed);
        tail.store(dummy, std::memory_order_relaxed);
    }

    ~MSQueue()
    {
        T value;
        while (pop(value));
    }

    bool push(const T& value)
    {
        MSQueueNode<T>* newNode = new MSQueueNode<T>(value);
        MSQueueNode<T>* oldTail = tail.load(std::memory_order_relaxed);

        while(!std::atomic_compare_exchange_weak(&oldTail->next, &newNode, nullptr))
        {
            oldTail = tail.load(std::memory_order_relaxed);
        }
        tail.compare_exchange_strong(oldTail, newNode, std::memory_order_relaxed);
        return true;
    }

    bool pop(T& value)
    {
        MSQueueNode<T>* oldHead = head.load(std::memory_order_relaxed);
        MSQueueNode<T>* nextNode = oldHead->next.load(std::memory_order_acquire);
        if (nextNode == nullptr)
        {
            return false;
        }
        value = nextNode ->data;
        head.store(nextNode, std::memory_order_release);
        delete oldHead;
        return true;
    }
private:
    std::atomic<MSQueueNode<T>*> head;
    std::atomic<MSQueueNode<T>*> tail;
};


template <typename T>
class MutexQueue
{
public:
    MutexQueue() = default;

    void push(const T& value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
    }

    bool pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
        {
            return false;
        }
        value = m_queue.front();
        m_queue.pop();
        return true;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};


static void MSProducer(MSQueue<int>& queue, int numItems)
{
    for (int i = 0; i < numItems; ++i)
    {
        queue.push(i);  // Push to the queue
    }
}

static void MSConsumer(MSQueue<int>& queue, int numItems)
{
    int value;
    for (int i = 0; i < numItems; ++i)
    {
        while (!queue.pop(value));  // Keep popping until the queue is not empty
    }
}

static void MutexProducer(MutexQueue<int>& queue, int numItems)
{
    for (int i = 0; i < numItems; ++i)
    {
        queue.push(i);  // Push to the queue
    }
}

// Consumer function for the mutex-based queue
static void MutexConsumer(MutexQueue<int>& queue, int numItems)
{
    int value;
    for (int i = 0; i < numItems; ++i)
    {
        while (!queue.pop(value));  // Keep popping until the queue is not empty
    }
}


BENCHMARK_F(LockFreeBenchmark, MSLocking)(benchmark::State& state)
{
    MutexQueue<int> queue;
    const int numItems = 10000;  // Number of items per producer
    const int numProducers = 4;  // Number of producer threads

    std::vector<std::thread> producers;
    std::thread consumerThread(MutexConsumer, std::ref(queue), numItems * numProducers);

    // Start multiple producer threads
    for (int i = 0; i < numProducers; ++i)
    {
        producers.push_back(std::thread(MutexProducer, std::ref(queue), numItems));
    }

    // Wait for all producer threads to finish
    for (auto& producer : producers)
    {
        producer.join();
    }

    // Wait for the consumer thread to finish
    consumerThread.join();
}


BENCHMARK_F(LockFreeBenchmark, MSAtomic)(benchmark::State& state)
{
    const int numItems = 10000;  // Number of items per producer
    const int numProducers = 4;  // Number of producer threads
    MSQueue<int> queue;

    std::vector<std::thread> producers;
    std::thread consumerThread(MSConsumer, std::ref(queue), numItems * numProducers);

    // Start multiple producer threads
    for (int i = 0; i < numProducers; ++i)
    {
        producers.push_back(std::thread(MSProducer, std::ref(queue), numItems));
    }

    // Wait for all producer threads to finish
    for (auto& producer : producers)
    {
        producer.join();
    }

    // Wait for the consumer thread to finish
    consumerThread.join();
}