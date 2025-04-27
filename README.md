# hft_patterns
Testing some common HFT optimizations

## Results

```
2025-04-26T13:43:41+01:00
Running C:\Users\tom.mclean\src\hft_patterns\build\release\HFTBenchmark.exe
Run on (16 X 2611 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1280 KiB (x8)
  L3 Unified 24576 KiB (x1)
-----------------------------------------------------------------------------------
Benchmark                                         Time             CPU   Iterations
-----------------------------------------------------------------------------------
CacheBenchmark/BM_CacheCold                12961857 ns     13113839 ns           56
CacheBenchmark/BM_CacheWarm                12552859 ns     12276786 ns           56
SIMDBenchmark/BM_AddArray                     12508 ns        12556 ns        56000
SIMDBenchmark/BM_AddArraySIMD                 12834 ns        12835 ns        56000
PrefetchBenchmark/NoPrefetch                 487334 ns       500000 ns         1000
PrefetchBenchmark/WithPrefetch               198919 ns       194972 ns         3446
PrefetchBenchmark/WithSIMD                   197258 ns       196725 ns         3733
ArenaBenchmark/NormalAllocation              204089 ns       177854 ns         2987
ArenaBenchmark/ArenaAllocation                18683 ns        18834 ns        37333
PoolBenchmark/NormalAllocation               315577 ns       313895 ns         2240
PoolBenchmark/PoolAllocation                  26862 ns        26681 ns        26353
CRTPBenchmark/Polymorphism                      391 ns          392 ns      1792000
CRTPBenchmark/CRTP                              107 ns          107 ns      6400000
BranchReductionBenchmark/Branching            0.327 ns        0.322 ns   2133333333
BranchReductionBenchmark/BranchReduction      0.341 ns        0.342 ns   2240000000
LockFreeBenchmark/LockBuffer                   27.7 ns         27.6 ns     24888889
LockFreeBenchmark/AtomicBuffer                 1.15 ns         1.15 ns    640000000
LockFreeBenchmark/MSLocking                 8392197 ns       921376 ns          407
LockFreeBenchmark/MSAtomic                 18908475 ns       734375 ns         1000
HazardPointerBenchmark/Locking                 76.3 ns         76.7 ns     11200000
HazardPointerBenchmark/NonLocking              43.9 ns         43.0 ns     16000000
```