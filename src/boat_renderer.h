#pragma once
#include "boat_model.h"
#include "environment.h"
#include <epoxy/gl.h>
class BoatRenderer {
public:
    BoatRenderer(const std::string &assets,const std::string &shaders);
    ~BoatRenderer();
    std::array<BoatState,boatKinds> render(int width,int height,double seconds,const SunState &sun,const Ocean &ocean,GLuint heightTexture,bool drift,bool enabled);
    GLuint color() const{return colorTex;}
    GLuint reflection() const{return reflectedTex;}
private:
    GLuint program=0,vao=0,vbo=0,fbo=0,depth=0,msaaFbo=0,msaaColor=0,colorTex=0,reflectedTex=0;
    struct Range {int first=0,count=0,opaque=0;};
    std::array<Range,boatKinds> ranges{};
    std::array<GLuint,boatKinds> materialTextures{};
    int width=0,height=0,samples=4;
    void resize(int w,int h);
};
