#include <benchmark/benchmark.h>
#include "cache_warming.hpp"
#include "simd_math.hpp"
#include "prefetch.hpp"
#include "arena_allocator.hpp"
#include "pool_allocator.hpp"
#include "curiously_recurring_template_pattern.hpp"
#include "branch_reduction.hpp"
// #include "lock_free.hpp"
// #include "michael_scott_queue.hpp"
// #include "hazard_pointer.hpp"
// #include "mpmc_queue.hpp"
#include "vos_vs_sov.hpp"
#include "affinity.hpp"

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}