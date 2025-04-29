#pragma once

#include <benchmark/benchmark.h>


#ifdef _MSC_VER
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#else
    #define LIKELY(x)   (__builtin_expect(!!(x), 1))
    #define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#endif


void handleErrorA() {
    static int dummy = 0;
    benchmark::DoNotOptimize(++dummy);
}

void handleErrorB() {
    static int dummy = 0;
    benchmark::DoNotOptimize(++dummy);
}

void handleErrorC() {
    static int dummy = 0;
    benchmark::DoNotOptimize(++dummy);
}

void executeHotPath() {
    static int dummy = 0;
    benchmark::DoNotOptimize(++dummy);
}


namespace branching {


bool checkForErrorA() {
    static int errorCounterA = 0;
    errorCounterA++;
    return (errorCounterA % 10) == 0;
}

bool checkForErrorB() {
    return false;
}

bool checkForErrorC() {
    return false;
}

void doFn() {
    if (checkForErrorA())
        handleErrorA();
    else if (checkForErrorB())
        handleErrorB();
    else if (checkForErrorC())
        handleErrorC();
    else
        executeHotPath();
}

}

namespace branchreduction {

enum ErrorFlags {
    ErrorA = 1 << 0,
    ErrorB = 1 << 1,
    ErrorC = 1 << 2,
    NoError = 0
};

ErrorFlags checkForErrorA() {
    static int errorCounterA = 0;
    errorCounterA++;
    return ((errorCounterA % 10) == 0) ? ErrorFlags::ErrorA : ErrorFlags::NoError;
}

ErrorFlags checkForErrorB() {
    return ErrorFlags::NoError;
}

ErrorFlags checkForErrorC() {
    return ErrorFlags::NoError;
}

ErrorFlags checkForErrors()
{
    return static_cast<ErrorFlags>(
        checkForErrorA() | 
        checkForErrorB() | 
        checkForErrorC()
    );
}

void handleErrors(ErrorFlags errors)
{
    if (errors && ErrorFlags::ErrorA) {
        handleErrorA();
    }
    if (errors && ErrorFlags::ErrorB) {
        handleErrorB();
    }
    if (errors && ErrorFlags::ErrorC) {
        handleErrorC();
    }
}

void doFn() {
    ErrorFlags errors = checkForErrors();
    if (LIKELY(errors == ErrorFlags::NoError)) {
        executeHotPath();
    }
    else {
        handleErrors(errors);
    }
}


}


class BranchReductionBenchmark : public benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State& state) override {

    }

    void TearDown(const ::benchmark::State& state) override {

    }

    size_t nLoops = 1000000;
};

BENCHMARK_F(BranchReductionBenchmark, Branching)(benchmark::State& state)
{
    for (auto _ : state)
    {
        for (size_t i=0; i<nLoops; ++i) {
            branching::doFn();
        }
    }
}


BENCHMARK_F(BranchReductionBenchmark, BranchReduction)(benchmark::State& state)
{
    for (auto _ : state)
    {
        for (size_t i=0; i<nLoops; ++i) {
            branchreduction::doFn();
        }
    }
}