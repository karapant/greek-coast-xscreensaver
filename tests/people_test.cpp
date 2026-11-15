#include "people.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool value,const char *message){if(!value)throw std::runtime_error(message);}
int main(){
    People steady,irregular;
    for(int tick=0;tick<=9000;++tick)steady.update(tick/30.,19.5);
    double t=0;while(t<300){t=std::min(300.,t+.071);irregular.update(t,19.5);}
    auto a=steady.inspect(),b=irregular.inspect();
    for(size_t i=0;i<a.size();++i){check(std::abs(a[i].u-b[i].u)<1e-9&&std::abs(a[i].distance-b[i].distance)<1e-8,"motion must be independent of frame cadence");}
    People life;bool served=false,dined=false,chatted=false,idle=false;double maxAcceleration=0;
    auto previous=life.inspect();
    for(int tick=0;tick<18000;++tick){
        life.update(tick/30.,19.5);auto now=life.inspect();
        for(size_t i=0;i<now.size();++i){
            auto &p=now[i];check(std::isfinite(p.u)&&p.u>.08&&p.u<.38&&p.v>.26&&p.v<.33,"all agents stay on village and hillside grounds");
            check(p.speed>=0&&p.speed<=1.851,"agents keep plausible unaccelerated walking/running speeds");
            double delta=std::hypot((p.u-previous[i].u)*16./9.,p.v-previous[i].v)*200.;
            check(delta<.065,"no route reset, wraparound or teleport while moving");
            maxAcceleration=std::max(maxAcceleration,std::abs(p.speed-previous[i].speed)*30.);
            served|=p.role==2&&p.state==3;dined|=p.role==0&&p.state==4;
            chatted|=p.role==1&&p.state==2;
        }
        for(auto &s:life.sprites())if(s.style[1]==3)idle=true;
        previous=now;
    }
    check(maxAcceleration<=1.501,"starts and stops respect bounded human acceleration");
    check(served&&dined&&chatted&&idle,"workers serve, guests dine, children pause, idle poses have planted feet");
    // Crossing a clock threshold must let current journeys finish, not erase actors.
    People closing;closing.update(18.,19.5);auto before=closing.inspect();closing.update(18.+1./30.,3.);
    auto after=closing.inspect();for(size_t i=0;i<before.size();++i)check(std::abs(after[i].u-before[i].u)<.001,"closing time never teleports active people home");
    closing.update(300.,3.);closing.update(600.,3.);for(auto &p:closing.inspect())check(p.state==0,"quiet-night people finish their current tasks and return indoors");
    People hidden;hidden.update(64.,23.,false);for(auto &p:hidden.inspect())check(p.role!=2||p.state==0,"disabled taverna has no wandering waiters");
    People night;night.update(64.,3.);check(night.sprites().empty(),"no new children or diners emerge during the quiet hours");
    auto sprites=life.sprites();for(size_t i=1;i<sprites.size();++i)check(sprites[i].place[1]>=sprites[i-1].place[1],"actors draw back to front");
    std::cout<<"Fixed-step movement, plausible speeds, continuity, dining/service/play tasks, closing-time return and depth order passed (max acceleration "<<maxAcceleration<<")\n";
}
