#pragma once
#include <array>
constexpr int roadSampleCount=64;
// Photo coordinates (u,v from top left), followed by a forward tangent.
struct RoadSample { float u,v,du,dv; };
std::array<RoadSample,roadSampleCount> villageRoadSamples();
