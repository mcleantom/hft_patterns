#pragma once
#include <benchmark/benchmark.h>

template <typename T>
class PoolAllocator {
public:
    explicit PoolAllocator(size_t capacity)
        : capacity_(capacity)
        , pool_(static_cast<char*>(::operator new(capacity * sizeof(T))))
        , free_list_(nullptr)
    {
        for (size_t i=0; i<capacity_; ++i) {
            void* ptr = pool_ + i * sizeof(T);
            FreeNode* node = static_cast<FreeNode*>(ptr);
            node->next = free_list_;
            free_list_ = node;
        }
    }
    
    ~PoolAllocator() {
        ::operator delete(pool_);
    }

    T* allocate() {
        if (!free_list_) {
            throw std::bad_alloc();
        }
        FreeNode* node = free_list_;
        free_list_ = node->next;
        return reinterpret_cast<T*>(node);
    }

    void deallocate(T* ptr) {
        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = free_list_;
        free_list_ = node;
    }
private:
    struct FreeNode {
        FreeNode* next;
    };
    size_t capacity_;
    char* pool_;
    FreeNode* free_list_;
};


struct MyPoolObject {
    int a, b, c, d;
};


class PoolBenchmark : public benchmark::Fixture {
public:
    static constexpr size_t object_count = 10000;
    void SetUp(const benchmark::State& state) override {
        pool = std::make_unique<PoolAllocator<MyPoolObject>>(object_count);
    }

    void TearDown(const benchmark::State& state) override {
        pool.reset();
    }

    std::unique_ptr<PoolAllocator<MyPoolObject>> pool;
};


BENCHMARK_F(PoolBenchmark, NormalAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<MyPoolObject*> objects;
        objects.reserve(object_count);
        for (int i = 0; i < object_count; ++i) {
            objects.push_back(new MyPoolObject{1, 2, 3, 4});
        }
        benchmark::DoNotOptimize(objects);
        for (auto obj : objects) {
            delete obj;
        }
    }
}

BENCHMARK_F(PoolBenchmark, PoolAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<MyPoolObject*> objects;
        objects.reserve(object_count);
        for (size_t i = 0; i < object_count; ++i) {
            objects.push_back(new (pool->allocate()) MyPoolObject{1, 2, 3, 4});
        }

        benchmark::DoNotOptimize(objects);

        for (auto ptr : objects) {
            ptr->~MyPoolObject();
            pool->deallocate(ptr);
        }
    }
}