#include <benchmark/benchmark.h>
#include <vector>
#include <emmintrin.h>

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
    int prefetch_distance = 64 / sizeof(int);  // Cache line size / size of int
};

// Function without __builtin_prefetch
BENCHMARK_F(PrefetchBenchmark, NoPrefetch)(benchmark::State& state) {
    for (auto _ : state) {
        long sum = 0;
        for (int i = 0; i < data.size(); i++) {
            sum += data[i];
        }
        // Prevent compiler optimization to discard the sum
        benchmark::DoNotOptimize(sum);
    }
}

// Function with __builtin_prefetch
BENCHMARK_F(PrefetchBenchmark, WithPrefetch)(benchmark::State& state) {
    for (auto _ : state) {
        long sum = 0;
        int i = 0;
        int size = static_cast<int>(data.size());

        for (; i <= size - 16; i += 8) {
            PREFETCH(&data[i + 8], 3);
            __m128i vec1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&data[i]));
            __m128i vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&data[i + 4]));
            __m128i sum_vec = _mm_add_epi32(vec1, vec2);

            int temp[4];
            _mm_storeu_si128(reinterpret_cast<__m128i*>(temp), sum_vec);
            sum += temp[0] + temp[1] + temp[2] + temp[3];
        }

        for (; i < size; ++i) {
            sum += data[i];
        }
        // Prevent compiler optimization to discard the sum
        benchmark::DoNotOptimize(sum);
    }
}


BENCHMARK_F(PrefetchBenchmark, WithSIMD)(benchmark::State& state) {
    for (auto _ : state) {
        long sum = 0;
        int i = 0;
        int size = static_cast<int>(data.size());

        for (; i <= size - 16; i += 8) {
            //PREFETCH(&data[i + 8], 3);
            __m128i vec1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&data[i]));
            __m128i vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&data[i + 4]));
            __m128i sum_vec = _mm_add_epi32(vec1, vec2);

            int temp[4];
            _mm_storeu_si128(reinterpret_cast<__m128i*>(temp), sum_vec);
            sum += temp[0] + temp[1] + temp[2] + temp[3];
        }

        for (; i < size; ++i) {
            sum += data[i];
        }
        // Prevent compiler optimization to discard the sum
        benchmark::DoNotOptimize(sum);
    }
}
