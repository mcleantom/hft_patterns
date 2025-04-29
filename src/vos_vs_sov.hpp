#pragma once
#include <vector>
#include <benchmark/benchmark.h>
#include <cmath>
#include <numeric>
#include <random>

namespace VOS {

struct OpenHighLowClose {
    float open;
    float high;
    float low;
    float close;
};

typedef std::vector<OpenHighLowClose> OpenHighLowCloses;

float ADX(const OpenHighLowCloses& data, size_t period = 14) {
    if (data.size() <= period) return 0.0f;

    const size_t size = data.size();
    float smoothedTR = 0.0f, smoothedPlusDM = 0.0f, smoothedMinusDM = 0.0f;
    float adx = 0.0f;

    // Initial smoothing values
    for (size_t i = 1; i <= period; ++i) {
        const auto& curr = data[i];
        const auto& prev = data[i - 1];
        float tr = std::max({curr.high - curr.low, std::abs(curr.high - prev.close), std::abs(curr.low - prev.close)});
        float plusDM = (curr.high - prev.high > prev.low - curr.low && curr.high - prev.high > 0) ? (curr.high - prev.high) : 0;
        float minusDM = (prev.low - curr.low > curr.high - prev.high && prev.low - curr.low > 0) ? (prev.low - curr.low) : 0;
        smoothedTR += tr;
        smoothedPlusDM += plusDM;
        smoothedMinusDM += minusDM;
    }

    smoothedTR /= period;
    smoothedPlusDM /= period;
    smoothedMinusDM /= period;

    float plusDI = 100.0f * (smoothedPlusDM / smoothedTR);
    float minusDI = 100.0f * (smoothedMinusDM / smoothedTR);
    float dx = 100.0f * std::abs(plusDI - minusDI) / (plusDI + minusDI);
    adx = dx;

    for (size_t i = period + 1; i < size; ++i) {
        const auto& curr = data[i];
        const auto& prev = data[i - 1];
        float tr = std::max({curr.high - curr.low, std::abs(curr.high - prev.close), std::abs(curr.low - prev.close)});
        float plusDM = (curr.high - prev.high > prev.low - curr.low && curr.high - prev.high > 0) ? (curr.high - prev.high) : 0;
        float minusDM = (prev.low - curr.low > curr.high - prev.high && prev.low - curr.low > 0) ? (prev.low - curr.low) : 0;

        smoothedTR = (smoothedTR * (period - 1) + tr) / period;
        smoothedPlusDM = (smoothedPlusDM * (period - 1) + plusDM) / period;
        smoothedMinusDM = (smoothedMinusDM * (period - 1) + minusDM) / period;

        plusDI = 100.0f * (smoothedPlusDM / smoothedTR);
        minusDI = 100.0f * (smoothedMinusDM / smoothedTR);
        dx = 100.0f * std::abs(plusDI - minusDI) / (plusDI + minusDI);
        adx = (adx * (period - 1) + dx) / period;
    }

    return adx;
}

}

namespace SOV {

struct OpenHighLowCloses {
    std::vector<float> opens;
    std::vector<float> highs;
    std::vector<float> lows;
    std::vector<float> closes;
};

float ADX(const OpenHighLowCloses& data, size_t period = 14) {
    size_t size = data.closes.size();
    if (size <= period) return 0.0f;

    float smoothedTR = 0.0f, smoothedPlusDM = 0.0f, smoothedMinusDM = 0.0f;
    float adx = 0.0f;

    for (size_t i = 1; i <= period; ++i) {
        float tr = std::max({
            data.highs[i] - data.lows[i],
            std::abs(data.highs[i] - data.closes[i - 1]),
            std::abs(data.lows[i] - data.closes[i - 1])
        });

        float plusDM = (data.highs[i] - data.highs[i - 1] > data.lows[i - 1] - data.lows[i] &&
                        data.highs[i] - data.highs[i - 1] > 0) ? (data.highs[i] - data.highs[i - 1]) : 0;
        float minusDM = (data.lows[i - 1] - data.lows[i] > data.highs[i] - data.highs[i - 1] &&
                         data.lows[i - 1] - data.lows[i] > 0) ? (data.lows[i - 1] - data.lows[i]) : 0;

        smoothedTR += tr;
        smoothedPlusDM += plusDM;
        smoothedMinusDM += minusDM;
    }

    smoothedTR /= period;
    smoothedPlusDM /= period;
    smoothedMinusDM /= period;

    float plusDI = 100.0f * (smoothedPlusDM / smoothedTR);
    float minusDI = 100.0f * (smoothedMinusDM / smoothedTR);
    float dx = 100.0f * std::abs(plusDI - minusDI) / (plusDI + minusDI);
    adx = dx;

    for (size_t i = period + 1; i < size; ++i) {
        float tr = std::max({
            data.highs[i] - data.lows[i],
            std::abs(data.highs[i] - data.closes[i - 1]),
            std::abs(data.lows[i] - data.closes[i - 1])
        });

        float plusDM = (data.highs[i] - data.highs[i - 1] > data.lows[i - 1] - data.lows[i] &&
                        data.highs[i] - data.highs[i - 1] > 0) ? (data.highs[i] - data.highs[i - 1]) : 0;
        float minusDM = (data.lows[i - 1] - data.lows[i] > data.highs[i] - data.highs[i - 1] &&
                         data.lows[i - 1] - data.lows[i] > 0) ? (data.lows[i - 1] - data.lows[i]) : 0;

        smoothedTR = (smoothedTR * (period - 1) + tr) / period;
        smoothedPlusDM = (smoothedPlusDM * (period - 1) + plusDM) / period;
        smoothedMinusDM = (smoothedMinusDM * (period - 1) + minusDM) / period;

        plusDI = 100.0f * (smoothedPlusDM / smoothedTR);
        minusDI = 100.0f * (smoothedMinusDM / smoothedTR);
        dx = 100.0f * std::abs(plusDI - minusDI) / (plusDI + minusDI);
        adx = (adx * (period - 1) + dx) / period;
    }

    return adx;
}

}


class StructOfVectorsBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {

    }

    void TearDown(const ::benchmark::State& state) override {

    }

    static const size_t n_OHLC = 100;
};


inline float randFloat(float min, float max) {
    static std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}


inline VOS::OpenHighLowCloses generateVOSData(size_t n) {
    VOS::OpenHighLowCloses data(n);
    for (size_t i=0; i<n; ++i) {
        float open = randFloat(90.0f, 110.0f);
        float high = open + randFloat(0.0f, 10.0f);
        float low = open - randFloat(0.0f, 10.0f);
        float close = randFloat(low, high);
        data[i] = {open, high, low, close};
    }
    return data;
}

inline SOV::OpenHighLowCloses generateSOVData(size_t n) {
    SOV::OpenHighLowCloses data;
    data.opens.reserve(n);
    data.highs.reserve(n);
    data.lows.reserve(n);
    data.closes.reserve(n);
    for (size_t i=0; i<n; ++i) {
        float open = randFloat(90.0f, 110.0f);
        float high = open + randFloat(0.0f, 10.0f);
        float low = open - randFloat(0.0f, 10.0f);
        float close = randFloat(low, high);
        data.opens.push_back(open);
        data.highs.push_back(high);
        data.lows.push_back(low);
        data.closes.push_back(close);
    }
    return data;
}


BENCHMARK_F(StructOfVectorsBenchmark, VectorOfStructs)(benchmark::State& state)
{
    auto data = generateVOSData(n_OHLC);
    for (auto _ : state) {
        float result = VOS::ADX(data);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK_F(StructOfVectorsBenchmark, StructOfVectors)(benchmark::State& state)
{
    auto data = generateSOVData(n_OHLC);
    for (auto _ : state) {
        float result = SOV::ADX(data);
        benchmark::DoNotOptimize(result);
    }
}
