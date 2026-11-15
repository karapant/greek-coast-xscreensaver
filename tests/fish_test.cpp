#include "fish.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool v,const char *s){if(!v)throw std::runtime_error(s);}
int main(){
    FishSchool a,b;float movement=0;
    auto initial=a.states();
    for(int tick=0;tick<=18000;++tick){a.update(tick/30.);for(auto &f:a.states()){
        float speed=std::hypot(f.vx,f.vz);check(std::isfinite(f.x)&&std::isfinite(f.z),"finite flock state");
        check(speed>=.1599f&&speed<=.4201f,"small fish keep plausible bounded speeds");
        check(f.x> -11&&f.x<11&&f.z> -20&&f.z<4,"shoals turn within the clear-water region without wrapping");
        check(f.y<-.30&&f.y> -1.1&&f.length<.27,"fish stay small and underwater");
    }}
    double t=0;while(t<600){t=std::min(600.,t+.071);b.update(t);}
    for(int i=0;i<fishCount;++i){auto x=a.states()[i],y=b.states()[i];check(std::abs(x.x-y.x)<1e-5&&std::abs(x.z-y.z)<1e-5,"flocking does not depend on render frame rate");movement+=std::hypot(x.x-initial[i].x,x.z-initial[i].z);}
    check(movement>10,"shoals actually swim rather than only wiggling textures");
    std::cout<<"Small underwater shoals, bounded speeds, confinement and frame-independent steering passed\n";
}
