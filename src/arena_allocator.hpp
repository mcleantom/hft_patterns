#pragma once
#include <benchmark/benchmark.h>
#include <vector>
#include <cstdlib>
#include <memory>


class Arena {
public:
    Arena(std::size_t size) {
        base = static_cast<char*>(std::malloc(size));
        current = base;
        end = base + size;
    }

    ~Arena() {
        std::free(base);
    }

    void* allocate(std::size_t size) {
        if (current + size > end) {
            throw std::bad_alloc();
        }

        void* result = current;
        current += size;
        return result;
    }

    void reset() {
        current = base;
    }
private:
    char* base;
    char* current;
    char* end;
};


struct MyObject {
    int a, b, c, d;
};


class ArenaBenchmark : public benchmark::Fixture {
public:
    static constexpr int num_objects = 10000;
    static constexpr std::size_t arena_size = num_objects * sizeof(MyObject);

    void SetUp(const benchmark::State&) override {
        arena = std::make_unique<Arena>(arena_size);
    }

    void TearDown(const benchmark::State&) override {
        arena.reset();
    }

    std::unique_ptr<Arena> arena;
};


BENCHMARK_F(ArenaBenchmark, NormalAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<MyObject*> objects;
        objects.reserve(num_objects);
        for (int i = 0; i < num_objects; ++i) {
            objects.push_back(new MyObject{1, 2, 3, 4});
        }
        benchmark::DoNotOptimize(objects);
        state.PauseTiming();
        for (auto obj : objects) {
            delete obj;
        }
        state.ResumeTiming();
    }
}

BENCHMARK_F(ArenaBenchmark, ArenaAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<MyObject*> objects;
        objects.reserve(num_objects);
        for (int i = 0; i < num_objects; ++i) {
            objects.push_back(new (arena->allocate(sizeof(MyObject))) MyObject{1, 2, 3, 4});
        }
        benchmark::DoNotOptimize(objects);
        state.PauseTiming();
        arena->reset();
        state.ResumeTiming();
    }
}