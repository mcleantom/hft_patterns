#pragma once

#include <benchmark/benchmark.h>


namespace polymorphic 
{

struct Base {
    virtual int add(int a, int b) = 0;
    virtual ~Base() = default;
};

struct Derived : Base {
    int add(int a, int b) override {
        return a + b;
    }
};


int doAdd(Base* base, int times) {
    int result = 0;
    for (int i = 0; i < 1000; ++i) {
        result += base->add(i, i);
    }
    return result;
}

}


namespace CRTP {

template <typename Derived>
struct Base {
    int add(int a, int b) {
        return static_cast<Derived*>(this)->add(a, b);
    }
};

struct Derived : Base<Derived> {
    int add(int a, int b) {
        return a + b;
    };
};

template <typename Derived>
int doAdd(Base<Derived>* base, int times) {
    int result = 0;
    for (int i = 0; i < 1000; ++i) {
        result += base->add(i, i);
    }
    return result;
}

}


class CRTPBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {

    }

    void TearDown(const ::benchmark::State& state) override {

    }
};


BENCHMARK_F(CRTPBenchmark, Polymorphism)(benchmark::State& state)
{
    polymorphic::Derived d;
    polymorphic::Base* base = &d;
    int result = 0;
    for (auto _ : state) {
        result += doAdd(base, 1000);
    }
    benchmark::DoNotOptimize(result);
}

BENCHMARK_F(CRTPBenchmark, CRTP)(benchmark::State& state)
{
    CRTP::Derived d;

    int result = 0;
    for (auto _ : state) {
        result += CRTP::doAdd(&d, 1000);
    }
    benchmark::DoNotOptimize(result);
}



