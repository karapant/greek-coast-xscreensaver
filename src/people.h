#pragma once
#include <array>
#include <vector>

// All positions are in the fixed coastline photograph's top-left coordinates.
// Motion uses an approximate local ground scale, not a free 3D village model.
struct PersonSprite {
    std::array<float,4> place; // u,v,standing height,opacity
    std::array<float,4> style; // frame,atlas (adult/child/waiter/idle),flip,warmth
};
struct PeopleDebug {
    int id,role,state;
    double u,v,speed,distance,opacity;
};
class People {
public:
    People();
    void update(double seconds,float localHour,bool restaurant=true);
    std::vector<PersonSprite> sprites(float size=1.8f) const;
    std::vector<PeopleDebug> inspect() const;
private:
    struct Point {double u,v;};
    enum State { Home,Walk,Chat,Serve,Dine,Rest };
    struct Stop {Point point;double hold=0;State action=Rest;};
    struct Actor {
        int id=0,role=0,visit=0,index=0;
        State state=Home;
        Point home{},position{},previous{},heading{1,0};
        std::vector<Stop> route;
        double speed=0,walked=0,legWalked=0,hold=0,holdTotal=0,next=0,opacity=0;
        double baseSpeed=1.1,lane=0;
        bool right=false;
    };
    std::vector<Actor> actors;
    double elapsed=0;
    bool initialized=false;
    void step(double dt,float hour,bool restaurant);
    void plan(Actor &actor,double hour,bool restaurant);
    static double distance(Point a,Point b);
    static Point terrace(double u,double v);
};
