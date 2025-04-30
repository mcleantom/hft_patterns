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
CacheBenchmark/BM_CacheCold                10017893 ns     10000000 ns           75
CacheBenchmark/BM_CacheWarm                10015633 ns     10009766 ns           64
SIMDBenchmark/BM_AddArray                     12096 ns        11998 ns        56000
SIMDBenchmark/BM_AddArraySIMD                 11844 ns        11963 ns        64000
PrefetchBenchmark/NoPrefetchAutoSIMD         475249 ns       481413 ns         1493
PrefetchBenchmark/WithPrefetch               196887 ns       199507 ns         3446
PrefetchBenchmark/WithSIMD                   157524 ns       156948 ns         4480
ArenaBenchmark/NormalAllocation              225590 ns       242535 ns         3801
PoolBenchmark/PoolAllocation                  30948 ns        31145 ns        23579
CRTPBenchmark/Polymorphism                      445 ns          446 ns      1120000
CRTPBenchmark/CRTP                              121 ns          119 ns      4977778
BranchReductionBenchmark/Branching          1911861 ns      1881143 ns          407
BranchReductionBenchmark/BranchReduction    2078858 ns      2083333 ns          345
StructOfVectorsBenchmark/VectorOfStructs        678 ns          670 ns      1120000
StructOfVectorsBenchmark/StructOfVectors        772 ns          767 ns       896000
AffinityBenchmark/NoAffinity                6770155 ns      6835938 ns          112
AffinityBenchmark/Affinity                  5713665 ns      5719866 ns          112
```