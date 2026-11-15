#include "road.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
struct Point {double x,y;};
// This is the continuous pale paved lane directly in front of the village
// houses. The cliff paths, gardens, retaining walls and steps are excluded.
constexpr std::array<Point,6> controls{{{.087,.298},{.105,.297},{.126,.299},{.149,.302},{.174,.304},{.190,.305}}};
Point curve(double t){
    double k=std::clamp(t,0.,.999999)*double(controls.size()-1);int i=int(k);double u=k-i;
    Point a=controls[std::max(0,i-1)],b=controls[i],c=controls[i+1],d=controls[std::min(int(controls.size()-1),i+2)];
    auto spline=[&](double w,double x,double y,double z){return .5*((2*x)+(-w+y)*u+(2*w-5*x+4*y-z)*u*u+(-w+3*x-3*y+z)*u*u*u);};
    return {spline(a.x,b.x,c.x,d.x),spline(a.y,b.y,c.y,d.y)};
}
double distance(Point a,Point b){return std::hypot((a.x-b.x)*16./9.,a.y-b.y);}
}
std::array<RoadSample,roadSampleCount> villageRoadSamples(){
    constexpr int denseCount=640;
    std::array<Point,denseCount+1> dense{};std::array<double,denseCount+1> length{};
    for(int i=0;i<=denseCount;++i){dense[i]=curve(double(i)/denseCount);if(i)length[i]=length[i-1]+distance(dense[i],dense[i-1]);}
    std::array<RoadSample,roadSampleCount> result{};
    for(int i=0;i<roadSampleCount;++i){
        double target=length.back()*i/(roadSampleCount-1);
        int next=std::clamp(int(std::lower_bound(length.begin(),length.end(),target)-length.begin()),1,denseCount);
        double f=(target-length[next-1])/(length[next]-length[next-1]);
        Point a=dense[next-1],b=dense[next];
        double du=b.x-a.x,dv=b.y-a.y,norm=std::hypot(du,dv);
        result[i]={float(a.x+(b.x-a.x)*f),float(a.y+(b.y-a.y)*f),float(du/norm),float(dv/norm)};
    }
    return result;
}
