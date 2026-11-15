#include "people.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr double aspect=16./9.,metres=200.;
double ease(double x){x=std::clamp(x,0.,1.);return x*x*(3.-2.*x);}
bool dinnerTime(double hour){return hour>=18.||hour<1.5;}
bool openTime(double hour){return hour>=9.||hour<1.5;}
bool childTime(double hour){return hour>=9.&&hour<20.5;}
}
double People::distance(Point a,Point b){return std::hypot((a.u-b.u)*aspect,a.v-b.v)*metres;}
People::Point People::terrace(double u,double v){return {.294+.082*u,.272+.049*v};}
People::People(){
    const Point homes[]={{.097,.282},{.161,.287},{.176,.287},{.119,.283}};
    for(int i=0;i<10;++i){
        Actor a;a.id=i;a.role=i<4?0:i<6?2:1;
        a.home=i<4?homes[i]:i<6?terrace(.25,.717):i<8?Point{.126,.286}:terrace(.25,.717);
        a.position=a.previous=a.home;
        a.next=i<4?i*6.:i<6?(i-4)*8.:i<8?3.+(i-6)*3.:2.+(i-8)*3.;
        a.baseSpeed=a.role==2?.87:a.role==1?1.85:1.08+.07*i;
        actors.push_back(a);
    }
}
void People::plan(Actor &a,double hour,bool restaurant){
    a.route.clear();a.index=0;a.legWalked=0;++a.visit;
    auto add=[&](Point p,double hold=0,State action=Rest){a.route.push_back({p,hold,action});};
    auto t=[&](double x,double y,double hold=0,State action=Rest){add(terrace(x,y),hold,action);};
    if(a.role==2){
        if(!restaurant||!openTime(hour))return;
        // Different table each trip; a second worker is offset by two tables.
        int table=(a.visit*2+(a.id-4)*2)%5;
        double x=.49+table*.095;
        t(.39,.725+(a.id-4)*.008);
        t(x,.725+(a.id-4)*.008);
        t(x,.704,4.2+table*.35,Serve);
        t(x,.725+(a.id-4)*.008);
        t(.39,.725+(a.id-4)*.008);
        add(a.home,2.4,Rest);
    }else if(a.role==1){
        if(!childTime(hour)||(a.id>=8&&!restaurant))return;
        if(a.id<8){
            add({.127,.291});
            // These points lie on the upper edge of the paved frontage,
            // separated from vehicle traffic on the lower lane.
            Point left{.129+(a.id-6)*.0015,.2918+(a.id-6)*.0013},right{.145+(a.id-6)*.0015,.2938+(a.id-6)*.0013};
            add(left,1.0,Chat);
            for(int lap=0;lap<2;++lap){add(right,1.1+.4*(a.id-6),Chat);add(left,1.5,Chat);}
            add({.127,.291});add(a.home);
        }else{
            t(.28,.729);t(.39,.733,1.5,Chat);t(.255,.733,1.2,Chat);
            t(.39,.733,1.4,Chat);t(.28,.729);add(a.home);
        }
    }else{
        // Door -> steps -> sidewalk. All journeys finish at a real doorway.
        Point entry=a.id==0?Point{.098,.294}:a.id==1?Point{.163,.299}:a.id==2?Point{.174,.301}:Point{.120,.295};
        add(entry);
        bool eating=restaurant&&dinnerTime(hour)&&(a.id==1||a.id==2);
        if(eating){
            const Point lane[]={{.174,.301},{.190,.302},{.211,.305},{.235,.309},{.263,.310},{.285,.310},{.310,.308}};
            for(Point p:lane)if(p.u>entry.u+.0001)add(p);
            t(.25,.717);t(.43,.724);t(.60+(a.id-1)*.19,.724);
            t(.60+(a.id-1)*.19,.693,55.+a.id*13.,Dine);
            t(.60+(a.id-1)*.19,.724);t(.43,.724);t(.25,.717);
            add({.310,.308});
            for(int i=5;i>=0;--i)if(lane[i].u>entry.u+.0001)add(lane[i]);
        }else{
            // Residents 0 and 3 overlap their pause to meet by the lane.
            Point meeting{.142+(a.id==3?-.0017:0.),.298-(a.id==3?.0011:0.)};
            if(a.id==0||a.id==3){add({.126,.296});add(meeting,10.+a.id,Chat);}
            else {add({.174,.301});add({.190,.302},6.+a.id*2.,Chat);}
        }
        add(entry);add(a.home);
    }
    a.state=Walk;
}
void People::step(double dt,float hour,bool restaurant){
    for(auto &a:actors){a.previous=a.position;}
    for(auto &a:actors){
        if(a.state==Home){
            a.opacity=0;a.speed=0;
            bool awake=a.role==0?(hour>=7.||hour<1.):a.role==2?openTime(hour):childTime(hour);
            if(elapsed>=a.next&&awake){plan(a,hour,restaurant);if(a.route.empty())a.next=elapsed+4.;}
            continue;
        }
        if(a.state!=Walk){
            a.speed=0;a.hold-=dt;
            if(a.state==Chat&&a.role==0)a.right=a.id==3;
            if(a.hold<=0){++a.index;a.legWalked=0;a.state=Walk;}
        }
        if(a.state==Walk){
            if(a.index>=int(a.route.size())){
                a.state=Home;a.position=a.home;a.opacity=0;a.speed=0;
                a.next=elapsed+(a.role==2?1.8:a.role==1?31.+a.id*2.:15.+a.id*4.);
                continue;
            }
            const Stop &goal=a.route[a.index];
            double remaining=distance(a.position,goal.point);
            if(remaining<.00003){
                if(a.speed>.05){a.speed=std::max(0.,a.speed-1.5*dt);continue;}
                a.position=goal.point;
                if(goal.hold>0){a.state=goal.action;a.hold=a.holdTotal=goal.hold;a.speed=0;}
                else {++a.index;a.legWalked=0;}
                continue;
            }
            double du=(goal.point.u-a.position.u)*aspect*metres/remaining,dv=(goal.point.v-a.position.v)*metres/remaining;
            a.heading={du,dv};if(std::abs(du)>.15)a.right=du>0;
            double desired=a.baseSpeed;
            // Stairs and doorway approaches are slower than level paving.
            if(std::abs(dv)>.46&&a.role==0)desired*=.56;
            // Look through short collinear segments to the next actual stop.
            // This starts braking early enough before a doorway or table.
            double brakingDistance=remaining;Point incoming{du,dv};
            for(int j=a.index;j+1<int(a.route.size())&&a.route[j].hold<=0;++j){
                Point at=a.route[j].point,next=a.route[j+1].point;
                double len=distance(at,next);if(len<.0001)continue;
                Point outgoing{(next.u-at.u)*aspect*metres/len,(next.v-at.v)*metres/len};
                if(incoming.u*outgoing.u+incoming.v*outgoing.v<.55)break;
                brakingDistance+=len;incoming=outgoing;
            }
            desired=std::min(desired,std::sqrt(2.*1.15*brakingDistance));
            // Following pedestrians keep a gap; opposite directions pass in
            // separate narrow lanes instead of stopping nose to nose.
            for(const auto &other:actors){
                if(other.id==a.id||other.state==Home||other.state==Dine)continue;
                double x=(other.previous.u-a.position.u)*aspect*metres,y=(other.previous.v-a.position.v)*metres;
                double ahead=x*du+y*dv,side=std::abs(x*dv-y*du);
                double alignment=du*other.heading.u+dv*other.heading.v;
                if(alignment>.3&&ahead>0&&ahead<1.1&&side<.33){desired=std::min(desired,std::max(0.,(ahead-.43)*1.5));}
                if(alignment<-.3&&ahead>0&&ahead<1.2&&side<.5)desired=std::min(desired,a.baseSpeed*.72);
            }
            a.speed+=std::clamp(desired-a.speed,-1.5*dt,1.0*dt);
            double moved=std::min(remaining,a.speed*dt);
            a.position.u+=du*moved/(aspect*metres);a.position.v+=dv*moved/metres;
            a.walked+=moved;a.legWalked+=moved;
            double targetLane=.00115*ease(distance(a.position,a.home)/.65);
            targetLane*=ease(std::max(0.,brakingDistance-moved)/.65);
            a.lane+=std::clamp(targetLane-a.lane,-.003*dt,.003*dt);
        }
        double doorway=distance(a.position,a.home);
        a.opacity=ease(doorway/.48);
        if(a.role==2)a.opacity=std::max(a.opacity,.15*ease(doorway/.12));
        if(a.state==Dine)a.opacity=0;
        if(a.state==Walk&&a.index<int(a.route.size())){
            const auto &goal=a.route[a.index];
            if(goal.action==Dine)a.opacity*=ease(distance(a.position,goal.point)/.42);
            if(a.index>0&&a.route[a.index-1].action==Dine)a.opacity*=ease(a.legWalked/.42);
        }
    }
}
void People::update(double seconds,float hour,bool restaurant){
    seconds=std::max(seconds,0.);
    if(!initialized||seconds<elapsed-1e-7||seconds-elapsed>300.){
        *this=People();initialized=true;
        // A long scripted seek warms up a bounded recent interval rather than
        // simulating days of history. Normal playback always advances fixed steps.
        if(seconds>300.){elapsed=seconds-120.;for(auto &a:actors)a.next+=elapsed;}
    }
    constexpr double dt=1./30.;
    while(elapsed+dt<=seconds+1e-8){step(dt,hour,restaurant);elapsed+=dt;}
}
std::vector<PersonSprite> People::sprites(float size) const{
    std::vector<PersonSprite> result;
    for(const auto &a:actors){
        if(a.opacity<.01||a.state==Home||a.state==Dine)continue;
        double height=a.role==1?.0056:a.role==2?.0093:.0082;
        // Tiny perspective change follows the local ground depth, never gait pose.
        height*=std::clamp(1.+(a.position.v-.300)*4.,.93,1.08)*size/1.8;
        int atlas=a.role==2?2:a.role==1?1:0,frame=0;
        if(a.state==Serve){frame=4+std::clamp(int((1.-a.hold/a.holdTotal)*4.),0,3);}
        else if(a.speed>.08&&a.state==Walk){double stride=a.role==1?1.36:a.role==2?1.02:1.22;frame=int(a.walked/stride*4.)%4;}
        else {atlas=3;frame=(a.role==2?2:a.role==1?1:0);if(a.state==Chat&&int(elapsed/3.+a.id)%3==0)frame+=3;}
        // Stay on the right of each path while passing. Taper at doorways/stops.
        double lane=a.lane;
        double u=a.position.u-a.heading.v*lane/aspect,v=a.position.v+a.heading.u*lane;
        double warm=(u>=.294&&u<=.376&&v>=.272&&v<=.321)?1.:0.;
        result.push_back({{float(u),float(v),float(height),float(a.opacity)},
                          {float(frame),float(atlas),float(a.right),float(warm)}});
    }
    std::stable_sort(result.begin(),result.end(),[](const PersonSprite &a,const PersonSprite &b){return a.place[1]<b.place[1];});
    return result;
}
std::vector<PeopleDebug> People::inspect() const{
    std::vector<PeopleDebug> result;for(const auto &a:actors)result.push_back({a.id,a.role,a.state,a.position.u,a.position.v,a.speed,a.walked,a.opacity});return result;
}
