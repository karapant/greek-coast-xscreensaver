#pragma once
#include "ocean.h"
#include "environment.h"
#include "boat_renderer.h"
#include "people.h"
#include "fish.h"
#include <epoxy/gl.h>
#include <string>

class Renderer {
public:
    Renderer(int grid,float wind,const std::string &assets,const std::string &shaders);
    ~Renderer();
    void draw(int width,int height,double seconds,const SunState &sun,bool drift=true,bool celestials=true,bool clouds=true,bool activity=true,float activityScale=1.8f,bool fishingBoat=true,bool taverna=true,bool fish=true);
    void snapshot(const std::string &path,int width,int height);
private:
    Ocean ocean;
    People people;
    FishSchool school;
    BoatRenderer boat;
    GLuint program=0,vao=0,heightTex=0,coastTex=0,seabedTex=0,nightTex=0,skylineTex=0,activityTex=0,celestialTex=0,cloudTex=0;
    GLuint volumeNoiseTex=0;
    GLuint peopleTex=0,tavernaDayTex=0,tavernaNightTex=0,waiterTex=0,idleTex=0;
};
