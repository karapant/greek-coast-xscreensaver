#include "boat_model.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>

namespace {
constexpr float pi=3.14159265358979323846f;
using V=QVector3D;
void triangle(std::vector<BoatVertex>&mesh,V a,V b,V c,float material){
    V n=V::crossProduct(b-a,c-a).normalized();
    if(n.lengthSquared()<.5f)return;
    for(V p:{a,b,c})mesh.push_back({p.x(),p.y(),p.z(),n.x(),n.y(),n.z(),(p.x()+3.4f)/7.2f,(p.y()+p.z())*.6f,material});
}
void quad(std::vector<BoatVertex>&m,V a,V b,V c,V d,float material){triangle(m,a,b,c,material);triangle(m,a,c,d,material);}
void box(std::vector<BoatVertex>&m,V center,V size,float material){
    V p[8];for(int i=0;i<8;++i)p[i]=center+V((i&1?.5f:-.5f)*size.x(),(i&2?.5f:-.5f)*size.y(),(i&4?.5f:-.5f)*size.z());
    constexpr int faces[6][4]={{0,4,6,2},{1,3,7,5},{0,1,5,4},{2,6,7,3},{0,2,3,1},{4,5,7,6}};
    for(auto &f:faces)quad(m,p[f[0]],p[f[1]],p[f[2]],p[f[3]],material);
}
void rod(std::vector<BoatVertex>&m,V a,V b,float radius,float material,int sides=8){
    V axis=(b-a).normalized(),right=V::crossProduct(axis,std::abs(axis.y())<.9f?V(0,1,0):V(1,0,0)).normalized(),up=V::crossProduct(right,axis);
    for(int i=0;i<sides;++i){float t0=2*pi*i/sides,t1=2*pi*(i+1)/sides;
        V q0=(right*std::cos(t0)+up*std::sin(t0))*radius,q1=(right*std::cos(t1)+up*std::sin(t1))*radius;
        quad(m,a+q0,b+q0,b+q1,a+q1,material);triangle(m,a,a+q1,a+q0,material);triangle(m,b,b+q0,b+q1,material);
    }
}
void sphere(std::vector<BoatVertex>&m,V center,V radius,float material){
    auto point=[&](float a,float b){return center+V(std::cos(a)*std::sin(b)*radius.x(),std::cos(b)*radius.y(),std::sin(a)*std::sin(b)*radius.z());};
    for(int i=0;i<12;++i)for(int j=0;j<8;++j){float a=2*pi*i/12,b=pi*j/8;
        quad(m,point(a,b),point(a+2*pi/12,b),point(a+2*pi/12,b+pi/8),point(a,b+pi/8),material);}
}
float hullWidth(float t){return .55f*(1-t)+1.02f*std::pow(std::max(0.f,std::sin(pi*t)),.75f);}
float sheer(float t){return .88f+.28f*std::pow(t,4);}
}
std::vector<BoatVertex> makeFishingBoat(){
    std::vector<BoatVertex> m;
    constexpr int stations=64,ribs=36;
    auto hull=[](int i,int j){float t=float(i)/stations,a=pi*j/ribs;
        return V(-3.35f+7.1f*t,sheer(t)-(1.62f-.35f*t)*std::sin(a),hullWidth(t)*std::cos(a));};
    for(int i=0;i<stations;++i){
        for(int j=0;j<ribs;++j)quad(m,hull(i,j),hull(i+1,j),hull(i+1,j+1),hull(i,j+1),0);
        V a=hull(i,0),b=hull(i+1,0),c=hull(i+1,ribs),d=hull(i,ribs);
        quad(m,a,b,c,d,2);
        rod(m,a+V(0,.035,0),b+V(0,.035,0),.065,1);
        rod(m,d+V(0,.035,0),c+V(0,.035,0),.065,1);
    }
    V stern(-3.35,.08,0);for(int j=0;j<ribs;++j)triangle(m,stern,hull(0,j+1),hull(0,j),0);
    // Compact wheelhouse, glazed windows, blue roof edge and deck details.
    // Open window apertures reveal an actual modeled cabin, rather than
    // opaque colored quads pasted over a solid wheelhouse box.
    box(m,V(-.10,1.18,0),V(2.0,.52,1.56),3);
    box(m,V(-1.08,1.87,0),V(.075,.98,1.56),3);
    box(m,V(-.10,2.33,0),V(2.0,.16,1.56),3);
    for(float side:{-1.f,1.f}){
        for(float x:{-.97f,.78f})box(m,V(x,1.87,side*.76),V(.11,.90,.10),3);
        // Blue window frames and thin, partly reflective glazing.
        box(m,V(-.08,1.87,side*.80),V(1.58,.035,.035),1);
        box(m,V(-.08,1.87,side*.79),V(1.60,.80,.012),4);
        rod(m,V(1.0,1.13,side*.95),V(2.65,1.23,side*.56),.027,5);
        rod(m,V(1.0,.96,side*.95),V(1.0,1.13,side*.95),.028,5);
    }
    box(m,V(.91,1.87,0),V(.012,.80,1.32),4);
    box(m,V(.94,1.87,0),V(.055,.86,.055),1);
    box(m,V(-.10,2.43,0),V(2.25,.10,1.85),1);
    box(m,V(-.10,2.51,0),V(2.32,.10,1.91),3);
    // Teak floor, dashboard, instruments, chart, seat and ceiling light.
    box(m,V(-.10,.94,0),V(1.85,.045,1.42),2);
    box(m,V(.62,1.45,0),V(.38,.34,1.28),2);
    box(m,V(.46,1.63,-.35),V(.16,.025,.25),13);
    box(m,V(.46,1.65,.40),V(.10,.05,.18),14);
    box(m,V(-.47,1.25,.06),V(.44,.12,.47),1);
    box(m,V(-.67,1.44,.06),V(.09,.40,.46),1);
    rod(m,V(-.47,.98,.06),V(-.47,1.23,.06),.045,5);
    box(m,V(-.14,2.22,0),V(.26,.025,.16),6);
    // Seated captain looking forward. Natural head/body proportions,
    // dark work shirt, pale skin, cap, forearms reaching the helm.
    sphere(m,V(-.33,1.57,.06),V(.20,.31,.22),15);
    sphere(m,V(-.24,1.99,.06),V(.125,.16,.13),12);
    sphere(m,V(-.25,2.105,.06),V(.14,.045,.145),1);
    box(m,V(-.10,2.09,.06),V(.15,.025,.25),1);
    sphere(m,V(-.112,1.99,.06),V(.04,.045,.045),12);
    for(float z:{-.13f,.25f}){
        rod(m,V(-.27,1.75,z),V(.00,1.60,z),.062,15,12);
        rod(m,V(.00,1.60,z),V(.30,1.69,z),.043,12,12);
        sphere(m,V(.30,1.69,z),V(.055,.05,.05),12);
        rod(m,V(-.32,1.31,z),V(.10,1.21,z),.08,1,12);
        rod(m,V(.10,1.21,z),V(.12,1.02,z),.075,1,12);
        box(m,V(.20,1.00,z),V(.29,.075,.12),5);
    }
    // Circular wooden steering wheel, six spokes and a metal hub.
    V helm(.30,1.67,.06);
    for(int segment=0;segment<32;++segment){
        float a=2*pi*segment/32,b=2*pi*(segment+1)/32;
        rod(m,helm+V(0,std::sin(a)*.205,std::cos(a)*.205),helm+V(0,std::sin(b)*.205,std::cos(b)*.205),.016,2,8);
    }
    for(int spoke=0;spoke<6;++spoke){float a=2*pi*spoke/6;rod(m,helm,helm+V(0,std::sin(a)*.20,std::cos(a)*.20),.011,5);}
    rod(m,V(.32,1.67,.06),V(.58,1.51,.06),.033,5);
    rod(m,V(-.65,2.57,0),V(-.65,4.95,0),.040,3);
    rod(m,V(-.65,4.50,-.52),V(-.65,4.50,.52),.025,3);
    rod(m,V(-.65,4.90,0),V(3.50,1.05,0),.011,5,6);
    rod(m,V(-.65,4.90,0),V(-3.10,.98,0),.011,5,6);
    sphere(m,V(-.65,4.58,0),V(.055,.075,.055),6);
    sphere(m,V(.40,2.58,-.60),V(.055,.055,.055),7);
    sphere(m,V(.40,2.58,.60),V(.055,.055,.055),8);
    // A modest working lamp over the water, inspired by Greek lamp fishing.
    rod(m,V(2.10,1.03,.55),V(2.10,1.92,.55),.035,5);
    rod(m,V(2.10,1.92,.55),V(2.10,1.92,1.55),.030,5);
    sphere(m,V(2.10,1.80,1.55),V(.095,.12,.095),6);
    rod(m,V(2.10,1.94,1.55),V(2.10,1.90,1.55),.19,5,12);
    // Net reel, rope coils, fish crates and orange floats on the aft deck.
    rod(m,V(-2.15,1.16,-.52),V(-2.15,1.16,.52),.34,9,16);
    box(m,V(-2.80,1.12,.42),V(.58,.34,.48),1);
    box(m,V(-2.78,1.12,-.43),V(.57,.34,.48),2);
    for(int i=0;i<3;++i)sphere(m,V(-2.50+i*.27,1.07,.84),V(.11,.11,.11),11);
    return m;
}
BoatState fishingBoatAt(double seconds,const Ocean &ocean){
    BoatState s;double phase=std::fmod(seconds+62.,210.);if(phase<0)phase+=210.;s.phase=float(phase);
    float theta=s.phase*2*pi/210.f;
    float x=-128.f+1.22f*s.phase,z=-54.f-7.f*std::cos(theta);
    s.position=V(x,ocean.sampleHeight(x,z)+.015f,z);
    V forward(1.22f,0,7.f*(2*pi/210.f)*std::sin(theta));forward.normalize();
    V right=V::crossProduct(forward,V(0,1,0)).normalized();
    float pitch=(ocean.sampleHeight(x+forward.x()*2,z+forward.z()*2)-ocean.sampleHeight(x-forward.x()*2,z-forward.z()*2))/4;
    float roll=(ocean.sampleHeight(x+right.x(),z+right.z())-ocean.sampleHeight(x-right.x(),z-right.z()))/2;
    V up=V(-forward.x()*pitch-right.x()*roll,1,-forward.z()*pitch-right.z()*roll).normalized();
    right=V::crossProduct(forward,up).normalized();up=V::crossProduct(right,forward).normalized();
    s.forward=forward;s.model.setToIdentity();
    s.model.setColumn(0,QVector4D(forward,0));s.model.setColumn(1,QVector4D(up,0));s.model.setColumn(2,QVector4D(right,0));s.model.setColumn(3,QVector4D(s.position,1));
    s.lamp=s.model.map(V(2.10,1.80,1.55));return s;
}
void exportFishingBoat(const std::string &path){
    std::ofstream f(path);if(!f)throw std::runtime_error("Cannot export boat mesh");auto vertices=makeFishingBoat();
    f<<"# Greek Coast — procedural 3D Greek fishing kaiki, metres\n";
    for(auto &v:vertices)f<<"v "<<v.x<<' '<<v.y<<' '<<v.z<<'\n';
    for(auto &v:vertices)f<<"vn "<<v.nx<<' '<<v.ny<<' '<<v.nz<<'\n';
    for(auto &v:vertices)f<<"vt "<<v.u<<' '<<v.v<<'\n';
    for(size_t i=0;i<vertices.size();i+=3)f<<"f "<<i+1<<'/'<<i+1<<'/'<<i+1<<' '<<i+2<<'/'<<i+2<<'/'<<i+2<<' '<<i+3<<'/'<<i+3<<'/'<<i+3<<'\n';
}


namespace {
void shapedHull(std::vector<BoatVertex> &m,float length,float beam,float deck,float draught){
    constexpr int stations=64,ribs=32;
    auto point=[&](int i,int j){float t=float(i)/stations,a=pi*j/ribs;
        float width=beam*(.25f*(1-t)+.80f*std::pow(std::max(0.f,std::sin(pi*t)),.70f));
        return V((t-.5f)*length,deck+.16f*t*t-draught*std::sin(a),width*std::cos(a));};
    for(int i=0;i<stations;++i){
        for(int j=0;j<ribs;++j)quad(m,point(i,j),point(i+1,j),point(i+1,j+1),point(i,j+1),0);
        quad(m,point(i,0),point(i+1,0),point(i+1,ribs),point(i,ribs),2);
        for(int j:{0,ribs})rod(m,point(i,j)+V(0,.035,0),point(i+1,j)+V(0,.035,0),.055,1);
    }
    for(int j=0;j<ribs;++j)triangle(m,V(-length*.5f,deck-.25f,0),point(0,j+1),point(0,j),0);
}
void greekFlag(std::vector<BoatVertex> &m,V origin,float width){
    float h=width*.60f;
    for(int row=0;row<9;++row)box(m,origin+V(width*.5f,-h*(row+.5f)/9,0),V(width,h/9,.012f),row%2?3:1);
    box(m,origin+V(width*.19f,-h*.27f,.012),V(width*.38f,h*.54f,.013),1);
    box(m,origin+V(width*.19f,-h*.27f,.024),V(width*.38f,h*.10f,.013),3);
    box(m,origin+V(width*.19f,-h*.27f,.024),V(width*.06f,h*.54f,.013),3);
}
void deckCaptain(std::vector<BoatVertex> &m,V at){
    sphere(m,at+V(0,.53f,0),V(.18f,.30f,.20f),15);
    sphere(m,at+V(.05f,.95f,0),V(.12f,.15f,.13f),12);
    sphere(m,at+V(.05f,1.08f,0),V(.14f,.035f,.14f),1);
    for(float side:{-.18f,.18f}){
        rod(m,at+V(0,.72f,side),at+V(.41f,.55f,side),.05f,12,12);
        rod(m,at+V(0,.26f,side*.6f),at+V(.03f,.02f,side*.6f),.075f,1,12);
    }
    V center=at+V(.42f,.57f,0);
    for(int i=0;i<24;++i){float a=2*pi*i/24,b=2*pi*(i+1)/24;
        rod(m,center+V(0,.20f*std::sin(a),.20f*std::cos(a)),center+V(0,.20f*std::sin(b),.20f*std::cos(b)),.018f,2,8);}
    for(int i=0;i<6;++i){float a=2*pi*i/6;rod(m,center,center+V(0,.20f*std::sin(a),.20f*std::cos(a)),.012f,5);}
}
}
std::vector<BoatVertex> makeGreekBoat(BoatKind kind){
    if(kind==BoatKind::Fishing)return makeFishingBoat();
    std::vector<BoatVertex> m;
    if(kind==BoatKind::Ferry){
        shapedHull(m,14.6f,2.15f,1.35f,2.08f);
        // Small island passenger ferry, with an open shaded passenger deck,
        // fore wheelhouse, blue belt, stern boarding ramp and safety rails.
        box(m,V(-.2,1.48,0),V(9.2,.24,3.6),3);
        box(m,V(-.7,3.1,0),V(9.4,.16,3.7),3);
        box(m,V(-.7,3.23,0),V(9.55,.10,3.8),1);
        for(float side:{-1.f,1.f}){
            for(int i=0;i<8;++i){float x=-4.7f+i*1.23f;rod(m,V(x,1.52,side*1.7),V(x,3.05,side*1.7),.045,3);}
            rod(m,V(-6.1,2.04,side*1.62),V(6.0,2.04,side*1.62),.025,3);
            rod(m,V(-6.1,1.72,side*1.62),V(6.0,1.72,side*1.62),.022,3);
            for(int i=0;i<11;++i)rod(m,V(-6.+i*1.1,1.40,side*1.62),V(-6.+i*1.1,2.04,side*1.62),.025,3);
            for(int i=0;i<5;++i){box(m,V(-4.+i*1.35,1.91,side*.98),V(.65,.55,.38),1);}
            // Lifebuoys, opaque center opening supplied by a dark inset.
            for(float x:{-3.8f,1.0f}){sphere(m,V(x,2.55,side*1.76),V(.24,.24,.055),11);sphere(m,V(x,2.55,side*1.82),V(.13,.13,.02),5);}
        }
        // Glazed wheelhouse with actual interior helm and captain.
        box(m,V(3.65,2.15,0),V(2.3,.55,2.6),3);
        box(m,V(2.54,2.85,0),V(.08,1.05,2.6),3);
        box(m,V(3.65,3.45,0),V(2.4,.16,2.7),1);
        for(float side:{-1.f,1.f}){
            for(float x:{2.55f,4.72f})box(m,V(x,2.92,side*1.27),V(.10,1.05,.10),3);
            box(m,V(3.63,2.93,side*1.31),V(2.1,.96,.014),4);
        }
        box(m,V(4.75,2.93,0),V(.014,.96,2.45),4);
        box(m,V(4.2,2.43,0),V(.36,.40,2.3),2);deckCaptain(m,V(3.48,2.35,.10));
        box(m,V(3.5,3.35,0),V(.35,.025,.25),6);
        box(m,V(-6.6,1.4,0),V(.95,.08,2.5),1);
        rod(m,V(-1.5,3.35,0),V(-1.5,5.0,0),.05,3);
        sphere(m,V(-1.5,4.8,0),V(.06,.10,.06),6);
        greekFlag(m,V(-4.7,3.05,0),.85);
        for(float side:{-1.f,1.f})sphere(m,V(4.25,3.55,side*1.15),V(.075,.075,.075),side>0?8:7);
    }else{
        shapedHull(m,9.4f,1.32f,.98f,1.7f);
        box(m,V(-.8,1.24,0),V(2.6,.50,1.70),0);
        box(m,V(-.8,1.52,0),V(2.72,.08,1.82),1);
        for(float side:{-1.f,1.f}){
            box(m,V(-.8,1.29,side*.86),V(1.6,.27,.014),4);
            rod(m,V(-4.,1.22,side*.70),V(3.65,1.29,side*.64),.025,5);
            for(int i=0;i<6;++i){float x=-3.5+i*1.3;rod(m,V(x,1.,side*.90),V(x,1.25,side*.90),.025,5);}
        }
        rod(m,V(-.5,1.15,0),V(-.5,9.9,0),.09,2,16);
        rod(m,V(-4.,2.5,0),V(2.,10.8,0),.065,2,12);
        rod(m,V(-.5,9.9,0),V(4.15,1.10,0),.015,5);
        rod(m,V(-.5,9.9,0),V(-4.1,1.10,0),.015,5);
        // Billowed triangular linen lateen sail, tessellated for smooth normals.
        constexpr int n=24;V a(-4,2.5,0),b(2,10.8,0),c(3.6,2.5,0);
        auto sail=[&](int i,int j){float u=float(i)/n,v=float(j)/n,w=1-u-v;return a*w+b*u+c*v+V(0,0,.60f*27*u*v*w);};
        for(int i=0;i<n;++i)for(int j=0;j<n-i;++j){triangle(m,sail(i,j),sail(i+1,j),sail(i,j+1),16);if(i+j<n-1)triangle(m,sail(i+1,j),sail(i+1,j+1),sail(i,j+1),16);}
        deckCaptain(m,V(-2.8,1.01,.13));
        greekFlag(m,V(-3.8,2.2,0),.65);
        sphere(m,V(-.5,9.8,0),V(.06,.08,.06),6);
        for(float side:{-1.f,1.f})sphere(m,V(.3,1.62,side*.92),V(.065,.065,.065),side>0?8:7);
        box(m,V(-1.1,1.48,0),V(.16,.03,.15),6);
    }
    return m;
}
BoatState greekBoatAt(BoatKind kind,double seconds,const Ocean &ocean){
    if(kind==BoatKind::Fishing)return fishingBoatAt(seconds,ocean);
    const bool ferry=kind==BoatKind::Ferry;
    const double period=ferry?170.:240.,speed=ferry?-3.:1.85;
    double phase=std::fmod(seconds+(ferry?60.:105.),period);if(phase<0)phase+=period;
    double angle=phase*2*pi/period;
    float x=ferry?260.f-3.f*phase:-220.f+1.85f*phase;
    float z=(ferry?-100.f:-140.f)-(ferry?9.f:7.f)*std::cos(angle);
    V forward(speed,0,(ferry?9.:7.)*2*pi/period*std::sin(angle));forward.normalize();
    V right=V::crossProduct(forward,V(0,1,0)).normalized();
    float span=ferry?4.:2.8;
    float pitch=(ocean.sampleHeight(x+forward.x()*span,z+forward.z()*span)-ocean.sampleHeight(x-forward.x()*span,z-forward.z()*span))/(span*2);
    float roll=(ocean.sampleHeight(x+right.x()*1.5,z+right.z()*1.5)-ocean.sampleHeight(x-right.x()*1.5,z-right.z()*1.5))/3;
    V up=V(-forward.x()*pitch-right.x()*roll,1,-forward.z()*pitch-right.z()*roll).normalized();
    right=V::crossProduct(forward,up).normalized();up=V::crossProduct(right,forward).normalized();
    BoatState state;state.phase=phase;state.position=V(x,ocean.sampleHeight(x,z)+.015,z);state.forward=forward;
    state.model.setToIdentity();state.model.setColumn(0,QVector4D(forward,0));state.model.setColumn(1,QVector4D(up,0));state.model.setColumn(2,QVector4D(right,0));state.model.setColumn(3,QVector4D(state.position,1));
    state.lamp=state.model.map(ferry?V(3.5,3.35,0):V(-1.1,1.48,0));return state;
}

int activeGreekBoat(double seconds){
    double phase=std::fmod(seconds+62.,620.);if(phase<0)phase+=620.;
    return phase<210.?0:phase<380.?1:2;
}
BoatState displayedGreekBoatAt(BoatKind kind,double seconds,const Ocean &ocean){
    double phase=std::fmod(seconds+62.,620.);if(phase<0)phase+=620.;
    int index=int(kind);double local=index==0?phase:index==1?phase-210.:phase-380.;
    if(activeGreekBoat(seconds)!=index)local=0;
    return greekBoatAt(kind,local-(index==0?62.:index==1?60.:105.),ocean);
}
