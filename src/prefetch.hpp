#include <benchmark/benchmark.h>
#include <vector>


#if defined(__GNUC__) || defined(__clang__)
    #define PREFETCH(addr, hint) __builtin_prefetch(addr, 0, hint)
#elif defined(_MSC_VER)
    #include <xmmintrin.h>  // For _mm_prefetch in MSVC
    #define PREFETCH(addr, hint) _mm_prefetch(reinterpret_cast<const char*>(addr), hint)
#else
    #define PREFETCH(addr, hint)  // No prefetch support for this compiler
#endif


class PrefetchBenchmark : public benchmark::Fixture {
public:
    static constexpr size_t data_size = 1 << 20;

    void SetUp(const benchmark::State& state) override {
        data.resize(data_size, 1); // Resize the vector based on the input argument
    }

    void TearDown(const benchmark::State& state) override {
        // No cleanup needed for this example, but this is where you could do it
    }

    std::vector<int> data;
};

// Function without __builtin_prefetch
BENCHMARK_F(PrefetchBenchmark, NoPrefetch)(benchmark::State& state) {
    for (auto _ : state) {
        long sum = 0;
        for (const auto& i : data) {
            sum += i;
        }
        // Prevent compiler optimization to discard the sum
        benchmark::DoNotOptimize(sum);
    }
}

// Function with __builtin_prefetch
BENCHMARK_F(PrefetchBenchmark, WithPrefetch)(benchmark::State& state) {
    int prefetch_distance = 10;
    for (auto _ : state) {
        long sum = 0;
        for (int i = 0; i < data.size(); i++) {
            if (i + prefetch_distance < data.size()) {
                PREFETCH(&data[i + prefetch_distance], 3);
            }
            sum += data[i];
        }
        // Prevent compiler optimization to discard the sum
        benchmark::DoNotOptimize(sum);
    }
}