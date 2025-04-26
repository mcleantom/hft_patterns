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
CRTPBenchmark/Polymorphism                      627 ns          641 ns      1000000
CRTPBenchmark/CRTP                              161 ns          157 ns      4977778
BranchReductionBenchmark/Branching            0.480 ns        0.473 ns   1486696296
BranchReductionBenchmark/BranchReduction      0.504 ns        0.506 ns   1544827586
```