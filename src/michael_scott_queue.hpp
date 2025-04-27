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
        while (true)
        {
            MSQueueNode<T>* oldTail = tail.load(std::memory_order_acquire);
            MSQueueNode<T>* tailNext = oldTail->next.load(std::memory_order_acquire);
    
            if (tailNext == nullptr)
            {
                if (oldTail->next.compare_exchange_weak(tailNext, newNode))
                {
                    tail.compare_exchange_weak(oldTail, newNode);
                    return true;
                }
            }
            else
            {
                tail.compare_exchange_weak(oldTail, tailNext);
            }
        }
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
        queue.push(i);
    }
}

static void MSConsumer(MSQueue<int>& queue, int numItems)
{
    int value;
    int popped = 0;
    while (popped < numItems)
    {
        if (queue.pop(value))
        {
            ++popped;
        }
    }
}

static void MutexProducer(MutexQueue<int>& queue, int numItems)
{
    for (int i = 0; i < numItems; ++i)
    {
        queue.push(i);
    }
}


static void MutexConsumer(MutexQueue<int>& queue, int numItems)
{
    int value;
    for (int i = 0; i < numItems; ++i)
    {
        while (!queue.pop(value));
    }
}


BENCHMARK_F(LockFreeBenchmark, MSLocking)(benchmark::State& state)
{
    for (auto _ : state) {
        MutexQueue<int> queue;

        std::vector<std::thread> producers;
        std::thread consumerThread(MutexConsumer, std::ref(queue), ms_numItems * ms_numProducers);
    
        for (int i = 0; i < ms_numProducers; ++i)
        {
            producers.push_back(std::thread(MutexProducer, std::ref(queue), ms_numItems));
        }
    
        for (auto& producer : producers)
        {
            producer.join();
        }
    
        consumerThread.join();
    }
}


BENCHMARK_F(LockFreeBenchmark, MSAtomic)(benchmark::State& state)
{
    for (auto _ : state) {
        MSQueue<int> queue;
    
        std::vector<std::thread> producers;
        std::thread consumerThread(MSConsumer, std::ref(queue), ms_numItems * ms_numProducers);
    
        for (int i = 0; i < ms_numProducers; ++i)
        {
            producers.push_back(std::thread(MSProducer, std::ref(queue), ms_numItems));
        }
    
        for (auto& producer : producers)
        {
            producer.join();
        }
    
        consumerThread.join();
    }
}