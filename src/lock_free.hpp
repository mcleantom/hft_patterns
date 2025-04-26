#pragma once
#include <benchmark/benchmark.h>
#include <atomic>
#include <mutex>


template<typename T, size_t Capacity>
class LockingRingBuffer
{
public:
    LockingRingBuffer()
    {
        static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2 for fast modulo");
    }

    bool push(const T& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t next_head = (m_head + 1) & (Capacity - 1);
        if (next_head == m_tail)
        {
            return false;
        }
        m_buffer[m_head] = item;
        m_head = next_head;
        return true;
    }

    bool pop(T& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_tail == m_head)
        {
            return false;
        }
        item = m_buffer[m_tail];
        m_tail = (m_tail + 1) & (Capacity - 1);
        return true;
    }
private:
    std::mutex m_mutex;
    size_t m_head = 0;
    size_t m_tail = 0;
    T m_buffer[Capacity];
};


template<typename T, size_t Capacity>
class AtomicRingBuffer
{
public:
    AtomicRingBuffer()
    {
        static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2 for fast modulo");
    }

    bool push(const T& item)
    {
        size_t head = m_head.load(std::memory_order_relaxed);
        size_t tail = m_tail.load(std::memory_order_acquire);

        if (((head + 1) & (Capacity - 1)) == (tail & (Capacity - 1)))
        {
            return false;
        }
        m_buffer[head & (Capacity - 1)] = item;
        m_head.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item)
    {
        size_t tail = m_tail.load(std::memory_order_relaxed);
        size_t head = m_head.load(std::memory_order_acquire);

        if ((tail & (Capacity - 1)) == (head & (Capacity - 1)))
        {
            return false;
        }

        item = m_buffer[tail & (Capacity - 1)];
        m_tail.store(tail + 1, std::memory_order_release);
        return true;
    }
private:
    alignas(64) std::atomic<size_t> m_head = 0;
    alignas(64) std::atomic<size_t> m_tail = 0;
    alignas(64) T m_buffer[Capacity];
};


class LockFreeBenchmark : public benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State& state) override {

    }

    void TearDown(const ::benchmark::State& state) override {

    }

};

BENCHMARK_F(LockFreeBenchmark, LockBuffer)(benchmark::State& state)
{
    LockingRingBuffer<int, 1024> buffer;
    int value = 0;
    for (auto _ : state) {
        buffer.push(42);
        buffer.pop(value);
        benchmark::ClobberMemory();
    }
}

BENCHMARK_F(LockFreeBenchmark, AtomicBuffer)(benchmark::State& state)
{
    AtomicRingBuffer<int, 1024> buffer;
    int value = 0;
    for (auto _ : state) {
        buffer.push(42);
        buffer.pop(value);
        benchmark::ClobberMemory();
    }
}