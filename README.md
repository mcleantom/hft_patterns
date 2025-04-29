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
CacheBenchmark/BM_CacheCold                11575889 ns     11474609 ns           64
CacheBenchmark/BM_CacheWarm                11377089 ns     11230469 ns           64
SIMDBenchmark/BM_AddArray                     11812 ns        11719 ns        56000
SIMDBenchmark/BM_AddArraySIMD                 11696 ns        11719 ns        64000
PrefetchBenchmark/NoPrefetchAutoSIMD         447346 ns       444984 ns         1545
PrefetchBenchmark/WithPrefetch               186709 ns       188354 ns         3733
PrefetchBenchmark/WithSIMD                   146872 ns       146484 ns         4480
ArenaBenchmark/NormalAllocation              200783 ns       230164 ns         2987
ArenaBenchmark/ArenaAllocation                17893 ns        17997 ns        37333
PoolBenchmark/NormalAllocation               306347 ns       306920 ns         2240
PoolBenchmark/PoolAllocation                  25842 ns        25495 ns        26353
CRTPBenchmark/Polymorphism                      433 ns          433 ns      1659259
CRTPBenchmark/CRTP                              102 ns          100 ns      6400000
BranchReductionBenchmark/Branching          1385363 ns      1411898 ns          498
BranchReductionBenchmark/BranchReduction    1585977 ns      1569475 ns          448
StructOfVectorsBenchmark/VectorOfStructs        522 ns          516 ns      1000000
StructOfVectorsBenchmark/StructOfVectors        536 ns          547 ns      1000000
```