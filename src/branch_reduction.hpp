#pragma once

#include <benchmark/benchmark.h>


void handleErrorA() {

}

void handleErrorB() {

}

void handleErrorC() {

}

void executeHotPath() {

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
    if (!errors) {
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