#include "boat_model.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool b,const char *s){if(!b)throw std::runtime_error(s);}
int main(int argc,char **argv){
    if(argc==3&&std::string(argv[1])=="--export"){exportFishingBoat(argv[2]);return 0;}
    auto mesh=makeFishingBoat();check(mesh.size()>3000&&mesh.size()%3==0,"real triangle mesh contains hull and fittings");
    float low=100,high=-100;for(auto &v:mesh){check(std::isfinite(v.x)&&std::isfinite(v.nx),"finite mesh");check(std::abs(v.nx*v.nx+v.ny*v.ny+v.nz*v.nz-1)<.001,"normalized mesh normals");low=std::min(low,v.y);high=std::max(high,v.y);}
    check(low<-.5&&high>4.8,"mesh has a submerged keel and raised mast");
    int skin=0,shirt=0,glass=0,charts=0;
    for(const auto &v:mesh){skin+=v.material==12;shirt+=v.material==15;glass+=v.material==4;charts+=v.material==13;}
    check(mesh.size()>18000,"refined hull, fittings and wheelhouse mesh");
    check(skin>100&&shirt>100&&glass>=36&&charts>=36,"captain, transparent glazing and chart are actual model geometry");
    Ocean ocean(64);ocean.update(20);auto a=fishingBoatAt(20,ocean),b=fishingBoatAt(21,ocean);
    check((b.position-a.position).length()>1.2&&(b.position-a.position).length()<1.3,"plausible steady fishing boat speed");
    check(std::abs(a.position.y()-ocean.sampleHeight(a.position.x(),a.position.z())-.015)<.0001,"boat floats on actual simulated sea");
    check(a.lamp.y()>1.4&&a.forward.length()>.99,"lamp and forward direction follow model");
    for(int kind=0;kind<boatKinds;++kind){auto model=makeGreekBoat(BoatKind(kind));check(model.size()>5000,"each vessel is a detailed independent triangle mesh");
        float top=-100;for(const auto &v:model){check(std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z),"new meshes are finite");top=std::max(top,v.y);}
        if(kind==2)check(top>10.,"sailing caique has a raised lateen rig");
    }
    check(activeGreekBoat(64)==0&&activeGreekBoat(240)==1&&activeGreekBoat(460)==2,"vessel types take turns, never share the renderer at once");
    for(double boundary:{148.,318.,558.}){
        auto old=displayedGreekBoatAt(BoatKind(activeGreekBoat(boundary-.01)),boundary-.01,ocean);
        auto next=displayedGreekBoatAt(BoatKind(activeGreekBoat(boundary)),boundary,ocean);
        for(auto &ship:{old,next})check(std::abs(ship.position.x())>(10.-ship.position.z())*(16./9.*1.12)*.5+8.,"boat changes occur fully outside the visible frame");
    }
    std::cout<<"3D boat mesh, speed, buoyancy and lamp attachment passed\n";
}
