#include "road.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
int main(){
    auto road=villageRoadSamples();double shortest=10,longest=0;
    check(std::abs(road.front().u-.087f)<.0001&&std::abs(road.back().u-.190f)<.0001,"route enters and exits at the paved lane");
    for(int i=0;i<roadSampleCount;++i){
        auto p=road[i];
        check(p.u>=.0869&&p.u<=.1901&&p.v>.2965&&p.v<.3051,"car path stays on traced paved lane");
        check(p.du>.97f&&std::abs(p.dv)<.25f,"car faces along the road, not across it");
        if(i){
            auto prev=road[i-1];double step=std::hypot((p.u-prev.u)*16./9.,p.v-prev.v);
            shortest=std::min(shortest,step);longest=std::max(longest,step);
            check(p.u>prev.u,"route has no reversing or oscillation");
            check(p.du*prev.du+p.dv*prev.dv>.995,"steering follows continuous bends");
        }
    }
    check(longest/shortest<1.02,"arc-length sampling keeps speed consistent through bends");
    std::cout<<"Road confinement, smooth steering and uniform-distance movement passed\n";
}
