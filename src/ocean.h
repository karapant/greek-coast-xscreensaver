#pragma once
#include <complex>
#include <vector>
#include <cstdint>

class Ocean {
public:
    explicit Ocean(int size=128,float wind=5.f,uint32_t seed=1975);
    void update(double seconds);
    int size() const { return n; }
    float length() const { return 96.f; }
    const std::vector<float> &height() const { return heights; }
    float sampleHeight(float x,float z) const;
private:
    int n;
    std::vector<std::complex<float>> initial, evolved, spatial;
    std::vector<float> omega, heights;
    float scale=1;
};
