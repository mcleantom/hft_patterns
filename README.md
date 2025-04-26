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
CacheBenchmark/BM_CacheCold                21987830 ns     21959459 ns           37
CacheBenchmark/BM_CacheWarm                17843412 ns     17463235 ns           34
SIMDBenchmark/BM_AddArray                     18202 ns        18415 ns        40727
SIMDBenchmark/BM_AddArraySIMD                 18345 ns        18415 ns        40727
PrefetchBenchmark/NoPrefetch                 690855 ns       671875 ns         1000
PrefetchBenchmark/WithPrefetch               285279 ns       278700 ns         2635
PrefetchBenchmark/WithSIMD                   276091 ns       256696 ns         2800
ArenaBenchmark/NormalAllocation              304270 ns       332031 ns         1600
ArenaBenchmark/ArenaAllocation                30230 ns        30831 ns        26353
PoolBenchmark/NormalAllocation               489974 ns       487717 ns         1730
PoolBenchmark/PoolAllocation                  40432 ns        38608 ns        16593
CRTPBenchmark/Polymorphism                      459 ns          455 ns      1544828
CRTPBenchmark/CRTP                              107 ns          107 ns      6400000
BranchReductionBenchmark/Branching            0.330 ns        0.330 ns   2036363636
BranchReductionBenchmark/BranchReduction      0.368 ns        0.368 ns   1866666667
LockFreeBenchmark/LockBuffer                   27.9 ns         27.6 ns     24888889
LockFreeBenchmark/AtomicBuffer                 1.12 ns         1.12 ns    640000000
```