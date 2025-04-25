#include <benchmark/benchmark.h>
#include "cache_warming.hpp"
#include "simd.hpp"
#include "prefetch.hpp"

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}