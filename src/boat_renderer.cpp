#include "boat_renderer.h"
#include <png.h>
#include <epoxy/glx.h>
#include <epoxy/egl.h>
#include <fstream>
#include <stdexcept>
#include <cstddef>
#include <cmath>
#include <algorithm>

namespace {
GLuint compile(GLenum type,const std::string &path){std::ifstream f(path);if(!f)throw std::runtime_error("Missing boat shader "+path);std::string s{std::istreambuf_iterator<char>(f),{}};const char *p=s.c_str();GLuint id=glCreateShader(type);glShaderSource(id,1,&p,nullptr);glCompileShader(id);int ok;glGetShaderiv(id,GL_COMPILE_STATUS,&ok);if(!ok){char log[4096];glGetShaderInfoLog(id,sizeof(log),nullptr,log);throw std::runtime_error(log);}return id;}
GLuint material(const std::string &path){png_image p{};p.version=PNG_IMAGE_VERSION;if(!png_image_begin_read_from_file(&p,path.c_str()))throw std::runtime_error("Cannot load boat materials");p.format=PNG_FORMAT_RGBA;std::vector<unsigned char> data(PNG_IMAGE_SIZE(p));if(!png_image_finish_read(&p,nullptr,data.data(),0,nullptr))throw std::runtime_error(p.message);GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,p.width,p.height,0,GL_RGBA,GL_UNSIGNED_BYTE,data.data());glGenerateMipmap(GL_TEXTURE_2D);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);png_image_free(&p);return id;}
}
BoatRenderer::BoatRenderer(const std::string &assets,const std::string &shaders){
    auto vs=compile(GL_VERTEX_SHADER,shaders+"/boat.vert"),fs=compile(GL_FRAGMENT_SHADER,shaders+"/boat.frag");program=glCreateProgram();glAttachShader(program,vs);glAttachShader(program,fs);glLinkProgram(program);glDeleteShader(vs);glDeleteShader(fs);int ok;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok)throw std::runtime_error("Boat program could not link");
    std::vector<BoatVertex> mesh;
    for(int kind=0;kind<boatKinds;++kind){
        auto part=makeGreekBoat(BoatKind(kind));
        std::stable_partition(part.begin(),part.end(),[](const BoatVertex &v){return v.material!=4;});
        ranges[kind]={int(mesh.size()),int(part.size()),int(std::count_if(part.begin(),part.end(),[](const BoatVertex &v){return v.material!=4;}))};
        mesh.insert(mesh.end(),part.begin(),part.end());
    }
    glGenVertexArrays(1,&vao);glBindVertexArray(vao);glGenBuffers(1,&vbo);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,mesh.size()*sizeof(BoatVertex),mesh.data(),GL_STATIC_DRAW);
    const int sizes[]={3,3,2,1};const size_t offsets[]={0,3*sizeof(float),6*sizeof(float),8*sizeof(float)};
    for(int i=0;i<4;++i){glEnableVertexAttribArray(i);glVertexAttribPointer(i,sizes[i],GL_FLOAT,GL_FALSE,sizeof(BoatVertex),reinterpret_cast<void *>(offsets[i]));}
    materialTextures={material(assets+"/boat-materials.png"),material(assets+"/ferry-materials.png"),material(assets+"/sailing-materials.png")};glGenFramebuffers(1,&fbo);glGenFramebuffers(1,&msaaFbo);glGenRenderbuffers(1,&msaaColor);glGenRenderbuffers(1,&depth);glGenTextures(1,&colorTex);glGenTextures(1,&reflectedTex);
}
BoatRenderer::~BoatRenderer(){if(!glXGetCurrentContext()&&eglGetCurrentContext()==EGL_NO_CONTEXT)return;glDeleteTextures(boatKinds,materialTextures.data());glDeleteTextures(1,&colorTex);glDeleteTextures(1,&reflectedTex);glDeleteRenderbuffers(1,&depth);glDeleteRenderbuffers(1,&msaaColor);glDeleteFramebuffers(1,&fbo);glDeleteFramebuffers(1,&msaaFbo);glDeleteBuffers(1,&vbo);glDeleteVertexArrays(1,&vao);glDeleteProgram(program);}
void BoatRenderer::resize(int w,int h){
    if(width==w&&height==h)return;
    width=w;height=h;
    GLint supported=0;glGetIntegerv(GL_MAX_SAMPLES,&supported);samples=std::min(4,supported);
    for(GLuint id:{colorTex,reflectedTex}){glBindTexture(GL_TEXTURE_2D,id);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);}
    glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorageMultisample(GL_RENDERBUFFER,samples,GL_DEPTH_COMPONENT24,w,h);
    glBindRenderbuffer(GL_RENDERBUFFER,msaaColor);glRenderbufferStorageMultisample(GL_RENDERBUFFER,samples,GL_RGBA8,w,h);
}
std::array<BoatState,boatKinds> BoatRenderer::render(int w,int h,double seconds,const SunState &sun,const Ocean &ocean,GLuint heightTexture,bool drift,bool enabled){
    resize(w,h);std::array<BoatState,boatKinds> states;for(int i=0;i<boatKinds;++i)states[i]=displayedGreekBoatAt(BoatKind(i),seconds,ocean);
    const int active=activeGreekBoat(seconds);
    glBindFramebuffer(GL_FRAMEBUFFER,msaaFbo);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,msaaColor);
    glEnable(GL_MULTISAMPLE);
    glViewport(0,0,w,h);glUseProgram(program);glBindVertexArray(vao);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LESS);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,heightTexture);glUniform1i(glGetUniformLocation(program,"heightMap"),0);
    glActiveTexture(GL_TEXTURE8);glUniform1i(glGetUniformLocation(program,"materials"),8);
    glUniform1f(glGetUniformLocation(program,"animationTime"),seconds);
    glUniform3f(glGetUniformLocation(program,"camera"),drift?std::sin(seconds*.045)*.065:0.,2.7,10.);
    glUniform2f(glGetUniformLocation(program,"resolution"),w,h);
    glUniform1f(glGetUniformLocation(program,"patchLength"),ocean.length());
    glUniform3fv(glGetUniformLocation(program,"sunDirection"),1,sun.direction.data());glUniform3fv(glGetUniformLocation(program,"moonDirection"),1,sun.moonDirection.data());
    glUniform1f(glGetUniformLocation(program,"daylight"),sun.daylight);glUniform1f(glGetUniformLocation(program,"lights"),sun.lights);glUniform1f(glGetUniformLocation(program,"moonIllumination"),sun.moonIllumination);
    auto select=[&](int kind){
        const auto &state=states[kind];
        glUniformMatrix4fv(glGetUniformLocation(program,"model"),1,GL_FALSE,state.model.constData());
        glUniform1f(glGetUniformLocation(program,"waterLevel"),state.position.y());
        const auto cabin=state.model.map(kind==1?QVector3D(3.5,3.35,0):kind==2?QVector3D(-1.1,1.48,0):QVector3D(-.14,2.16,0));
        glUniform3f(glGetUniformLocation(program,"cabinLamp"),cabin.x(),cabin.y(),cabin.z());
        glUniform3f(glGetUniformLocation(program,"lampPosition"),state.lamp.x(),state.lamp.y(),state.lamp.z());
        glActiveTexture(GL_TEXTURE8);glBindTexture(GL_TEXTURE_2D,materialTextures[kind]);
    };
    for(int pass=0;pass<2;++pass){
        glBindFramebuffer(GL_FRAMEBUFFER,msaaFbo);glDrawBuffer(GL_COLOR_ATTACHMENT0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Boat framebuffer is incomplete");
        glClearColor(0,0,0,0);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glUniform1i(glGetUniformLocation(program,"mirrored"),pass);
        if(enabled){
            for(int kind=0;kind<boatKinds;++kind){if(kind!=active)continue;select(kind);const auto &r=ranges[kind];glDrawArrays(GL_TRIANGLES,r.first,r.opaque);}
            glEnable(GL_BLEND);glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
            for(int kind=boatKinds-1;kind>=0;--kind){if(kind!=active)continue;select(kind);const auto &r=ranges[kind];glDrawArrays(GL_TRIANGLES,r.first+r.opaque,r.count-r.opaque);}
            glDepthMask(GL_TRUE);glDisable(GL_BLEND);
        }
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,pass?reflectedTex:colorTex,0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER,msaaFbo);glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0,0,w,h,0,0,w,h,GL_COLOR_BUFFER_BIT,GL_NEAREST);
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);glDisable(GL_DEPTH_TEST);return states;
}
