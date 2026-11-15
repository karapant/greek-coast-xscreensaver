#pragma once
#include "ocean.h"
#include <QMatrix4x4>
#include <QVector3D>
#include <string>
#include <vector>
#include <array>
enum class BoatKind { Fishing=0, Ferry=1, Sailing=2 };
constexpr int boatKinds=3;
struct BoatVertex {float x,y,z,nx,ny,nz,u,v,material;};
struct BoatState {
    QMatrix4x4 model;
    QVector3D position,forward,lamp;
    float phase=0;
};
std::vector<BoatVertex> makeFishingBoat();
BoatState fishingBoatAt(double seconds,const Ocean &ocean);
void exportFishingBoat(const std::string &path);

std::vector<BoatVertex> makeGreekBoat(BoatKind kind);
BoatState greekBoatAt(BoatKind kind,double seconds,const Ocean &ocean);
int activeGreekBoat(double seconds);
BoatState displayedGreekBoatAt(BoatKind kind,double seconds,const Ocean &ocean);
