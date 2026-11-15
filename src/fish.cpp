#include "fish.h"
#include <algorithm>
#include <cmath>
FishSchool::FishSchool(){
    for(int i=0;i<fishCount;++i){int group=i/8,n=i%8;float a=n*2.399963f;
        fish[i]={float(-2+group*2)+std::cos(a)*(.25f+n*.12f),-.48f-group*.12f,float(1-group*2)+std::sin(a)*(.25f+n*.12f),.20f+.014f*(n%5),.24f,.08f,float(group),n*.83f};}
}
void FishSchool::step(float dt){
    auto previous=fish;
    for(int i=0;i<fishCount;++i){const auto &a=previous[i];float sx=0,sz=0,cx=0,cz=0,vx=0,vz=0;int neighbours=0;
        for(int j=0;j<fishCount;++j){if(i==j)continue;const auto &b=previous[j];float x=a.x-b.x,z=a.z-b.z,d=x*x+z*z;
            if(d<.15f&&d>.0001f){sx+=x/(d+.025f);sz+=z/(d+.025f);}
            if(a.kind==b.kind&&d<5.f){cx+=b.x;cz+=b.z;vx+=b.vx;vz+=b.vz;++neighbours;}
        }
        float ax=sx*.10f,az=sz*.10f;
        if(neighbours){ax+=(cx/neighbours-a.x)*.12f+(vx/neighbours-a.vx)*.8f;az+=(cz/neighbours-a.z)*.12f+(vz/neighbours-a.vz)*.8f;}
        float targetX=std::sin(elapsed*.035+a.kind*2.1)*4.f;
        float targetZ=-.2f-a.kind*2.f+std::cos(elapsed*.025+a.kind)*1.1f;
        ax+=(targetX-a.x)*.035f;az+=(targetZ-a.z)*.035f;
        if(a.x>8)ax-=(a.x-8)*.8f;if(a.x< -8)ax+=(-8-a.x)*.8f;
        if(a.z>1)az-=(a.z-1)*.8f;if(a.z< -19)az+=(-19-a.z)*.8f;
        float force=std::hypot(ax,az);if(force>.50f){ax*=.50f/force;az*=.50f/force;}
        auto &b=fish[i];b.vx+=ax*dt;b.vz+=az*dt;float speed=std::hypot(b.vx,b.vz);
        if(speed>.42f){b.vx*=.42f/speed;b.vz*=.42f/speed;}else if(speed<.16f){float angle=std::atan2(b.vz,b.vx);b.vx=std::cos(angle)*.16f;b.vz=std::sin(angle)*.16f;}
        b.x+=b.vx*dt;b.z+=b.vz*dt;b.y=-.48f-a.kind*.12f+.08f*std::sin(elapsed*.7+a.phase);
    }
}
void FishSchool::update(double seconds){
    if(seconds<elapsed||seconds-elapsed>300.){*this=FishSchool();if(seconds>300.)elapsed=seconds-120.;}
    constexpr double dt=1./30.;while(elapsed+dt<=seconds+1e-8){step(dt);elapsed+=dt;}
}
