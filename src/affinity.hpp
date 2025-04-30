#pragma once

#include <benchmark/benchmark.h>
#include <thread>
#include <iostream>
#include <vector>
#include <numeric>


#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <pthread.h>
    #include <sched.h>
#else
    #error "Thread affinity not supported on this platform"
#endif


bool set_thread_affinity(std::thread& thread, int cpu_id) {
#if defined(_WIN32)
    DWORD_PTR mask = 1ull << cpu_id;
    HANDLE handle = static_cast<HANDLE>(thread.native_handle());
    DWORD_PTR result = SetThreadAffinityMask(handle, mask);
    if (result == 0) {
        std::cerr << "Error setting thread affinity: " << GetLastError() << std::endl;
        return false;
    }
    return true;
#elif defined(__linux__)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    pthread_t handle = thread.native_handle();
    int result = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        std::cerr << "Error setting thread affinity: " << result << std::endl;
        return false;
    }
    return true;
#endif
}

bool set_current_thread_affinity(int cpu_id) {
#if defined(_WIN32)
    DWORD_PTR mask = 1ull << cpu_id;
    HANDLE handle = GetCurrentThread();
    DWORD_PTR result = SetThreadAffinityMask(handle, mask);
    return result != 0;
#elif defined(__linux__)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    return result == 0;
#endif
}


class AffinityBenchmark : public benchmark::Fixture {
public:
    void SetUp(const benchmark::State& state) override {
        data.resize(data_size, 1);
    }

    void TearDown(const benchmark::State& state) override {
        // No cleanup needed for this example, but this is where you could do it
    }

    std::vector<int> data;
    size_t data_size = 10000;
    const int num_threads = 4;
};


void compute_heavy_work(const std::vector<int>& input, int& result_out) {
    volatile int sum = 0;
    for (int i=0; i<1000; ++i) {
        for (int value : input) {
            sum += value * i;
        }
    }
    result_out = sum;
}


static void run_on_thread_with_affinity(bool use_affinity, const std::vector<int>& data, benchmark::State& state) {
    int result = 0;
    std::atomic<bool> ready = false;

    std::thread t([&] {
        if (use_affinity) {
            set_current_thread_affinity(2);
        }
        while (!ready);
        for (auto _ : state) {
            compute_heavy_work(data, result);
            benchmark::DoNotOptimize(result);
        }
    });

    ready = true;
    t.join();
}


BENCHMARK_F(AffinityBenchmark, NoAffinity)(benchmark::State& state)
{
    run_on_thread_with_affinity(false, data, state);
}

BENCHMARK_F(AffinityBenchmark, Affinity)(benchmark::State& state)
{
    run_on_thread_with_affinity(true, data, state);
}