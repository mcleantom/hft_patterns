#pragma once

#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>


constexpr int kSize = 10000000;  
std::vector<int> data(kSize);
std::vector<int> indices(kSize);


class CacheBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {

    }

    void TearDown(const ::benchmark::State& state) override {

    }
};


BENCHMARK_F(CacheBenchmark, BM_CacheCold)(benchmark::State& state) {
    for (auto& index : indices) {
        index = rand() % kSize;
    }

    for (auto _ : state) {
        int sum = 0;
        for (int i = 0; i < kSize; ++i) {
            benchmark::DoNotOptimize(sum += data[indices[i]]);
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK_F(CacheBenchmark, BM_CacheWarm)(benchmark::State& state) {
    int sum_warm = 0;
    for (int i = 0; i < kSize; ++i) {
        benchmark::DoNotOptimize(sum_warm += data[i]);
    }
    benchmark::ClobberMemory();

    for (auto _ : state) {
        int sum = 0;
        for (int i = 0; i < kSize; ++i) {
            benchmark::DoNotOptimize(sum += data[i]);
        }
        benchmark::ClobberMemory();
    }
}
