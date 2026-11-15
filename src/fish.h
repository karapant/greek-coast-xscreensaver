#pragma once
#include <array>
constexpr int fishCount=24;
struct FishState {float x,y,z,length,vx,vz,kind,phase;};
class FishSchool {
public:
    FishSchool();
    void update(double seconds);
    const std::array<FishState,fishCount> &states() const{return fish;}
private:
    std::array<FishState,fishCount> fish{};
    double elapsed=0;
    void step(float dt);
};
