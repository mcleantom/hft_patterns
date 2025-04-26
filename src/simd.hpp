#pragma once
#include <benchmark/benchmark.h>
#include <emmintrin.h>

const int size = 100000;

void add_arrays(float* a, float* b, float* c, int size) {
    for (int i=0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
}


void add_arrays_simd(float* a, float* b, float* c, int size) {
    __m128 a_chunk, b_chunk, c_chunk;

    for (int i=0; i < size; i+=4) {
        a_chunk = _mm_loadu_ps(&a[i]);
        b_chunk = _mm_loadu_ps(&b[i]);
        c_chunk = _mm_add_ps(a_chunk, b_chunk);
        _mm_storeu_ps(&c[i], c_chunk);
    }
}



class SIMDBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {

    }

    void TearDown(const ::benchmark::State& state) override {

    }
};


BENCHMARK_F(SIMDBenchmark, BM_AddArray)(benchmark::State& state) {
    float* a = new float[size];
    float* b = new float[size];
    float* c = new float[size];

    for (auto _ : state) {
        add_arrays(a, b, c, size);
    }

    delete[] a;
    delete[] b;
    delete[] c;
}

BENCHMARK_F(SIMDBenchmark, BM_AddArraySIMD)(benchmark::State& state) {
    float* a = new float[size];
    float* b = new float[size];
    float* c = new float[size];

    for (auto _ : state) {
        add_arrays_simd(a, b, c, size);
    }

    delete[] a;
    delete[] b;
    delete[] c;
}