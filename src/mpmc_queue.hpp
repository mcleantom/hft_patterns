#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <benchmark/benchmark.h>


template <typename T>
class LockingQueue {
public:
    LockingQueue(size_t capacity) : capacity(capacity) {}

    bool enqueue(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_not_full.wait(lock, [this]() { return queue.size() < capacity; });

        queue.push(item);

        cv_not_empty.notify_one();
        
        return true;
    }

    bool dequeue(T& item) {
        std::unique_lock<std::mutex> lock(mtx);

        cv_not_empty.wait(lock, [this]() { return !queue.empty(); });

        item = queue.front();
        queue.pop();

        cv_not_full.notify_one();

        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }

private:
    std::queue<T> queue;
    size_t capacity;
    mutable std::mutex mtx;
    std::condition_variable cv_not_empty;
    std::condition_variable cv_not_full;
};


template <typename T>
class MPMCQueue {
public:
    explicit MPMCQueue(size_t capacity)
        : buffer(nullptr)
        , buffer_mask(capacity - 1)
        , enqueue_pos(0)
        , dequeue_pos(0)
    {
        assert((capacity >= 2) && ((capacity & (capacity - 1)) == 0));

        buffer = new Cell[capacity];
        for (size_t i=0; i<capacity; ++i) {
            buffer[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    ~MPMCQueue() {
        delete[] buffer;
    }

    bool enqueue(const T& item) {
        Cell* cell;
        // Get the write heads current position
        size_t pos = enqueue_pos.load(std::memory_order_relaxed);

        for (;;) {
            // Find the cell (slot) in the buffer corresponding to this position
            // (pos & buffer_mask) is a fast modulo (wraps around if pos >= capacity)
            // buffer_mask = capacity - 1
            cell = &buffer[pos & buffer_mask];
            // Read the sequence number of this cell with acquire semantics
            // This gives us information on whether this cell is ready to be written to
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            // Compute the difference between the cells sequence and our position
            // If diff==0, the slot is ready to be written to
            // If diff<0, the slot is full; queue is full
            // If diff>0, someone else moved the head; reload and retry
            intptr_t diff = (intptr_t)seq - (intptr_t)pos;

            if (diff == 0) {  // Slot is ready for us to write
                // Try to atomically increment enqueue_pos from pos to pos + 1
                // compare_exchange_weak will:
                //      Update enqueue pos to pos + 1 if it is still equal to pos
                //      If it fails, it will update pos with the latest value of enqueue_pos (this is why it is weak)
                if (enqueue_pos.compare_exchange_weak(
                    pos, pos+1, std::memory_order_relaxed
                )) {
                    // We successfully updated the write head position, break out of the for loop
                    break;
                }
                // The value of pos is different, need to restart the loop
                // (remember that pos's value has been updated to the latest value
                // by compare_exchange_weak)
            } else if (diff < 0) {  // Slot is full, queue is full
                // If diff < 0, the slots sequence is behind - this slot hasnt been consumed yet
                // The queue is full
                return false;
            } else {  // Someone moved the head, reload and retry
                pos = enqueue_pos.load(std::memory_order_relaxed);
            }
        }

        // We exited the loop succesfully, so we have reserved a slot
        // Store the data on the cell, it is safe because no one else can claim it
        cell->data = item;
        // Mark the slot as ready to be read by setting sequence = pos + 1
        // i.e. the next expected sequence for this slot is now pos + 1
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }
    bool dequeue(T& item) {
        Cell* cell;
        // Get the current read head position
        size_t pos = dequeue_pos.load(std::memory_order_relaxed);

        for (;;) {
            // Cet the cell at the position, applying the buffer mask
            cell = &buffer[pos & buffer_mask];
            // Read the sequence
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            // Compare sequence with the expected sequence value
            intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
            if (diff == 0) {  // The item is ready to be consumed
                // Try and increment the read head atomically
                // If the function fails, it will return false but update pos with
                // the new value, which will be used in the next iteration
                if (dequeue_pos.compare_exchange_weak(
                    pos, pos + 1, std::memory_order_relaxed
                )) {
                    break;
                }
                
            } else if (diff < 0) {  
                // If the sequence of the current cell is not yet pos + 1, the item is either
                // not written yet or already consumed by another consumer
                return false;
            } else {
                // Retry, updating the read head position
                pos = dequeue_pos.load(std::memory_order_relaxed);
            }
        }

        // Set item to the data held in the cell we have successfully marked as read
        item = cell->data;
        // Update the sequence position to outside the buffer to mark as deleted and safe to reuse
        cell->sequence.store(pos + buffer_mask + 1, std::memory_order_release);
        return true;
    }
private:
    /*
    The sequence is a value stored in each cell of the queue that manages synchronization between
    producers (who enqueue) and consumers (who dequeue). The sequence tracks the state of the cell and indicates
    whether its ready to be written to (by a producer) or read from (by a consumer)

    Producer: increments sequence to indicate the item has been written
    Consumer: Reads the item only when the sequence matches the expected value,
              signling that the item is ready to be consumed.

    1. Producer writes to the queue
        When a producer is ready to write to a cell as pos, it does the following:
            It first checks if the sequence of the cell is pos (the slot is ready to be written to)

            It then writes data to the cell

            After writing, the producer sets the sequence to pos + 1. This indicates the item in the slot is
            now ready to be consumed
    
    2. Consumer reads from the queue
        When the consumer wants to read from pos, it does the following:
            It first checks if the sequence of the cell is pos + 1, this means the producer has finished writing and the
            item is ready for consumption

            If the sequence == pos + 1, the consumer reads the data and then sets the sequence to pos + capacity (to mark the
            slot as empty and ready for the next producer to use)
    */
    struct Cell {
        std::atomic<size_t> sequence;
        T data;
    };
    Cell* buffer;
    size_t buffer_mask;
    std::atomic<size_t> enqueue_pos;
    std::atomic<size_t> dequeue_pos;
};


class QueueBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State&) override {

    }

    void TearDown(const ::benchmark::State&) override {

    }

    const size_t num_producers = 1;
    const size_t num_consumers = 1;
    const size_t queue_capacity = 10000;
    const size_t n_loops = 1000;
};


BENCHMARK_F(QueueBenchmark, LockingQueue)(benchmark::State& state)
{
    LockingQueue<int> queue(queue_capacity);
    std::atomic<bool> terminate_flag(false);

    auto producer_func = [&]() {
        for (int i=0; i<n_loops; ++i) {
            while (!queue.enqueue(i));
        }
    };

    auto consumer_func = [&]() {
        int item;
        while (!terminate_flag.load()) {
            queue.dequeue(item);
            if (item == n_loops - 1) {
                terminate_flag.store(true, std::memory_order_relaxed);
            }
        }
    };

    for (auto _ : state) {
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        terminate_flag.store(false, std::memory_order_relaxed);

        for (size_t i=0; i<num_producers; ++i) {
            producers.push_back(std::thread(producer_func));
        }

        for (size_t i=0; i<num_consumers; ++i) {
            consumers.push_back(std::thread(consumer_func));
        }

        for (auto& p : producers) p.join();
        for (auto& c : consumers) c.join();
    }
}


BENCHMARK_F(QueueBenchmark, NonLockingQueue)(benchmark::State& state)
{
    MPMCQueue<int> queue(queue_capacity);
    std::atomic<bool> terminate_flag(false);

    auto producer_func = [&]() {
        for (int i=0; i<n_loops; ++i) {
            while (!queue.enqueue(i));
        }
    };

    auto consumer_func = [&]() {
        int item;
        while (!terminate_flag.load()) {
            queue.dequeue(item);
            if (item == n_loops - 1) {
                terminate_flag.store(true, std::memory_order_relaxed);
            }
        }
    };

    for (auto _ : state) {
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        terminate_flag.store(false, std::memory_order_relaxed);

        for (size_t i=0; i<num_producers; ++i) {
            producers.push_back(std::thread(producer_func));
        }

        for (size_t i=0; i<num_consumers; ++i) {
            consumers.push_back(std::thread(consumer_func));
        }

        for (auto& p : producers) p.join();
        for (auto& c : consumers) c.join();
    }
}
