#include "renderer.h"
#include "road.h"
#include <png.h>
#include <epoxy/glx.h>
#include <epoxy/egl.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
std::string read(const std::string &path){
    std::ifstream in(path);if(!in)throw std::runtime_error("Cannot read "+path);
    return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
}
GLuint shader(GLenum type,const std::string &source){
    GLuint s=glCreateShader(type);const char *p=source.c_str();glShaderSource(s,1,&p,nullptr);glCompileShader(s);
    int ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[8192];glGetShaderInfoLog(s,sizeof(log),nullptr,log);glDeleteShader(s);throw std::runtime_error(log);}
    return s;
}
GLuint texture(const std::string &path,bool repeat){
    png_image image{};image.version=PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_file(&image,path.c_str()))throw std::runtime_error("Cannot load texture "+path+": "+image.message);
    image.format=PNG_FORMAT_RGBA;std::vector<unsigned char> pixels(PNG_IMAGE_SIZE(image));
    if(!png_image_finish_read(&image,nullptr,pixels.data(),0,nullptr)){
        std::string error=image.message;png_image_free(&image);throw std::runtime_error(error);
    }
    GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);
    png_image_free(&image);return id;
}
GLuint skyline(const std::string &path){
    png_image image{};image.version=PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_file(&image,path.c_str()))throw std::runtime_error("Cannot derive shoreline silhouette");
    image.format=PNG_FORMAT_RGBA;std::vector<unsigned char> pixels(PNG_IMAGE_SIZE(image));
    if(!png_image_finish_read(&image,nullptr,pixels.data(),0,nullptr))throw std::runtime_error(image.message);
    std::vector<float> edge(image.width,.40f);
    auto luminance=[&](unsigned x,unsigned y){auto *p=pixels.data()+(size_t(y)*image.width+x)*4;return (.2126*p[0]+.7152*p[1]+.0722*p[2])/255.;};
    for(unsigned x=0;x<image.width;++x){
        for(unsigned y=image.height/10;y<unsigned(image.height*.4);++y){
            const auto *p=pixels.data()+(size_t(y)*image.width+x)*4;
            bool blueSky=p[2]>p[0]+9&&p[2]>p[1]+2&&luminance(x,y)>.4;
            bool edgeBelow=luminance(x,y)-luminance(x,std::min(y+4,image.height-1))>.045;
            if(!blueSky||edgeBelow){edge[x]=float(y)/image.height;break;}
        }
    }
    GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R32F,image.width,1,0,GL_RED,GL_FLOAT,edge.data());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    png_image_free(&image);return id;
}
std::vector<float> spriteBounds(const std::string &path,unsigned columns=3,unsigned rows=2){
    png_image image{};image.version=PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_file(&image,path.c_str()))throw std::runtime_error("Cannot read activity atlas bounds");
    image.format=PNG_FORMAT_RGBA;std::vector<unsigned char> pixels(PNG_IMAGE_SIZE(image));
    if(!png_image_finish_read(&image,nullptr,pixels.data(),0,nullptr))throw std::runtime_error(image.message);
    std::vector<float> result(columns*rows*4);
    for(unsigned row=0;row<rows;++row)for(unsigned col=0;col<columns;++col){
        unsigned left=(col+1)*image.width/columns,right=col*image.width/columns;
        unsigned top=(row+1)*image.height/rows,bottom=row*image.height/rows;
        for(unsigned y=row*image.height/rows;y<(row+1)*image.height/rows;++y)
            for(unsigned x=col*image.width/columns;x<(col+1)*image.width/columns;++x)
                if(pixels[(size_t(y)*image.width+x)*4+3]>96){left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);}
        if(right<=left||bottom<=top)throw std::runtime_error("Missing person or vehicle in activity atlas");
        size_t at=(row*columns+col)*4;
        result[at]=(left+.5f)/image.width;result[at+1]=(top+.5f)/image.height;
        result[at+2]=(right+.5f)/image.width;result[at+3]=(bottom+.5f)/image.height;
    }
    png_image_free(&image);return result;
}
// Identical crop coordinates within every animation cell prevent a bent
// serving pose from stretching back to standing height or feet from sliding.
std::vector<float> animationBounds(const std::string &path,unsigned columns,unsigned rows){
    auto bounds=spriteBounds(path,columns,rows);
    for(unsigned row=0;row<rows;++row){
        float left=1,right=0,top=1,bottom=0;
        for(unsigned col=0;col<columns;++col){size_t n=(row*columns+col)*4;
            left=std::min(left,bounds[n]*columns-col);right=std::max(right,bounds[n+2]*columns-col);
            top=std::min(top,bounds[n+1]*rows-row);bottom=std::max(bottom,bounds[n+3]*rows-row);}
        for(unsigned col=0;col<columns;++col){size_t n=(row*columns+col)*4;
            bounds[n]=(col+left)/columns;bounds[n+2]=(col+right)/columns;
            bounds[n+1]=(row+top)/rows;bounds[n+3]=(row+bottom)/rows;}
    }
    return bounds;
}
struct DetailAtlas {GLuint id;std::vector<float> idle,fish;};
DetailAtlas detailAtlas(const std::string &assets){
    struct Pixels {unsigned w,h;std::vector<unsigned char> rgba;};
    auto read=[](const std::string &path){png_image im{};im.version=PNG_IMAGE_VERSION;
        if(!png_image_begin_read_from_file(&im,path.c_str()))throw std::runtime_error("Missing detail atlas "+path);
        im.format=PNG_FORMAT_RGBA;Pixels p{im.width,im.height,std::vector<unsigned char>(PNG_IMAGE_SIZE(im))};
        if(!png_image_finish_read(&im,nullptr,p.rgba.data(),0,nullptr))throw std::runtime_error(im.message);
        png_image_free(&im);return p;};
    auto idle=read(assets+"/people-idle.png"),fish=read(assets+"/fish.png");
    unsigned w=std::max(idle.w,fish.w),h=idle.h+fish.h+16;
    std::vector<unsigned char> empty(size_t(w)*h*4,0);GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,empty.data());
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,idle.w,idle.h,GL_RGBA,GL_UNSIGNED_BYTE,idle.rgba.data());
    glTexSubImage2D(GL_TEXTURE_2D,0,0,idle.h+16,fish.w,fish.h,GL_RGBA,GL_UNSIGNED_BYTE,fish.rgba.data());
    glGenerateMipmap(GL_TEXTURE_2D);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    auto a=spriteBounds(assets+"/people-idle.png",3,2),b=spriteBounds(assets+"/fish.png",1,3);
    for(size_t n=0;n<a.size();n+=4){a[n]*=float(idle.w)/w;a[n+2]*=float(idle.w)/w;a[n+1]*=float(idle.h)/h;a[n+3]*=float(idle.h)/h;}
    for(size_t n=0;n<b.size();n+=4){b[n]*=float(fish.w)/w;b[n+2]*=float(fish.w)/w;b[n+1]=(b[n+1]*fish.h+idle.h+16)/h;b[n+3]=(b[n+3]*fish.h+idle.h+16)/h;}
    return {id,a,b};
}
GLuint volumeNoise(){
    constexpr int size=64;std::vector<unsigned char> data(size*size*size*3);
    uint32_t state=0x91da37u;for(auto &v:data){state^=state<<13;state^=state>>17;state^=state<<5;v=unsigned(state>>24);}
    GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_3D,id);glTexImage3D(GL_TEXTURE_3D,0,GL_RGB8,size,size,size,0,GL_RGB,GL_UNSIGNED_BYTE,data.data());
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    for(auto p:{GL_TEXTURE_WRAP_S,GL_TEXTURE_WRAP_T,GL_TEXTURE_WRAP_R})glTexParameteri(GL_TEXTURE_3D,p,GL_REPEAT);
    return id;
}
}
Renderer::Renderer(int grid,float wind,const std::string &assets,const std::string &shaders):ocean(grid,wind),boat(assets,shaders){
    if(!epoxy_is_desktop_gl()||epoxy_gl_version()<33)throw std::runtime_error("Greek Coast needs desktop OpenGL 3.3 or newer");
    std::cerr<<"OpenGL "<<glGetString(GL_VERSION)<<" · "<<glGetString(GL_RENDERER)<<'\n';
    GLuint vs=shader(GL_VERTEX_SHADER,read(shaders+"/scene.vert"));
    GLuint fs=shader(GL_FRAGMENT_SHADER,read(shaders+"/scene.frag"));
    program=glCreateProgram();glAttachShader(program,vs);glAttachShader(program,fs);glLinkProgram(program);
    glDeleteShader(vs);glDeleteShader(fs);
    int ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);
    if(!ok){char log[8192];glGetProgramInfoLog(program,sizeof(log),nullptr,log);throw std::runtime_error(log);}
    glGenVertexArrays(1,&vao);glBindVertexArray(vao);
    glGenTextures(1,&heightTex);glBindTexture(GL_TEXTURE_2D,heightTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R32F,grid,grid,0,GL_RED,GL_FLOAT,ocean.height().data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    coastTex=texture(assets+"/coast.png",false);seabedTex=texture(assets+"/seabed.png",true);
    nightTex=texture(assets+"/coast-night.png",false);
    skylineTex=skyline(assets+"/coast.png");
    activityTex=texture(assets+"/activity.png",false);
    celestialTex=texture(assets+"/celestial.png",false);
    cloudTex=texture(assets+"/clouds.png",false);
    volumeNoiseTex=volumeNoise();
    tavernaDayTex=texture(assets+"/taverna-hill-day.png",false);
    tavernaNightTex=texture(assets+"/taverna-hill-night.png",false);
    waiterTex=texture(assets+"/waiters.png",false);
    const auto details=detailAtlas(assets);idleTex=details.id;
    peopleTex=texture(assets+"/people.png",false);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program,"heightMap"),0);
    glUniform1i(glGetUniformLocation(program,"coastMap"),1);
    glUniform1i(glGetUniformLocation(program,"seabedMap"),2);
    glUniform1i(glGetUniformLocation(program,"nightMap"),3);
    glUniform1i(glGetUniformLocation(program,"skylineMap"),4);
    glUniform1i(glGetUniformLocation(program,"activityMap"),5);
    glUniform1i(glGetUniformLocation(program,"celestialMap"),6);
    glUniform1i(glGetUniformLocation(program,"cloudMap"),7);
    glUniform1i(glGetUniformLocation(program,"boatColorMap"),8);
    glUniform1i(glGetUniformLocation(program,"boatReflectionMap"),9);
    glUniform1i(glGetUniformLocation(program,"volumeNoiseMap"),10);
    glUniform1i(glGetUniformLocation(program,"cloudSteps"),grid<=64?20:grid>=256?40:28);
    glUniform1i(glGetUniformLocation(program,"peopleMap"),11);
    glUniform1i(glGetUniformLocation(program,"tavernaDayMap"),12);
    glUniform1i(glGetUniformLocation(program,"tavernaNightMap"),13);
    glUniform1i(glGetUniformLocation(program,"waiterMap"),14);
    const auto waiters=animationBounds(assets+"/waiters.png",4,2);
    glUniform4fv(glGetUniformLocation(program,"waiterBounds"),8,waiters.data());
    const auto people=animationBounds(assets+"/people.png",4,2);
    glUniform4fv(glGetUniformLocation(program,"peopleBounds"),8,people.data());
    glUniform1i(glGetUniformLocation(program,"idleMap"),15);
    glUniform4fv(glGetUniformLocation(program,"idleBounds"),6,details.idle.data());
    glUniform4fv(glGetUniformLocation(program,"fishBounds"),3,details.fish.data());
    const auto bounds=spriteBounds(assets+"/activity.png");
    glUniform4fv(glGetUniformLocation(program,"activityBounds"),6,bounds.data());
    const auto road=villageRoadSamples();
    static_assert(sizeof(RoadSample)==sizeof(float)*4);
    glUniform4fv(glGetUniformLocation(program,"roadSamples"),roadSampleCount,reinterpret_cast<const float *>(road.data()));
    glUniform1f(glGetUniformLocation(program,"patchLength"),ocean.length());
    glUniform1f(glGetUniformLocation(program,"wind"),wind);
}
Renderer::~Renderer(){
    if(!glXGetCurrentContext()&&eglGetCurrentContext()==EGL_NO_CONTEXT)return;
    glDeleteTextures(1,&heightTex);glDeleteTextures(1,&coastTex);glDeleteTextures(1,&seabedTex);
    glDeleteTextures(1,&nightTex);
    glDeleteTextures(1,&skylineTex);
    glDeleteTextures(1,&activityTex);
    glDeleteTextures(1,&celestialTex);glDeleteTextures(1,&cloudTex);
    glDeleteTextures(1,&volumeNoiseTex);
    glDeleteTextures(1,&peopleTex);glDeleteTextures(1,&tavernaDayTex);glDeleteTextures(1,&tavernaNightTex);glDeleteTextures(1,&waiterTex);glDeleteTextures(1,&idleTex);
    glDeleteVertexArrays(1,&vao);glDeleteProgram(program);
}
void Renderer::draw(int width,int height,double seconds,const SunState &sun,bool drift,bool celestials,bool clouds,bool activity,float activityScale,bool fishingBoat,bool taverna,bool fish){
    glUseProgram(program);glUniform1i(glGetUniformLocation(program,"showTaverna"),taverna);
    people.update(seconds,sun.localHour,taverna);
    const auto residents=people.sprites(activityScale);
    glUniform1i(glGetUniformLocation(program,"personCount"),int(residents.size()));
    std::vector<float> places,styles;
    for(const auto &person:residents){places.insert(places.end(),person.place.begin(),person.place.end());styles.insert(styles.end(),person.style.begin(),person.style.end());}
    if(!residents.empty()){
        glUniform4fv(glGetUniformLocation(program,"personPlaces"),int(residents.size()),places.data());
        glUniform4fv(glGetUniformLocation(program,"personStyles"),int(residents.size()),styles.data());
    }
    school.update(seconds);
    glUniform1i(glGetUniformLocation(program,"showFish"),fish);
    std::vector<float> fishPlaces,fishHeading;
    for(const auto &f:school.states()){fishPlaces.insert(fishPlaces.end(),{f.x,f.y,f.z,f.length});fishHeading.insert(fishHeading.end(),{f.vx,f.vz,f.kind,f.phase});}
    glUniform4fv(glGetUniformLocation(program,"fishPlaces"),fishCount,fishPlaces.data());
    glUniform4fv(glGetUniformLocation(program,"fishHeading"),fishCount,fishHeading.data());
    ocean.update(seconds);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,heightTex);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,ocean.size(),ocean.size(),GL_RED,GL_FLOAT,ocean.height().data());
    glGenerateMipmap(GL_TEXTURE_2D);
    const auto vessel=boat.render(width,height,seconds,sun,ocean,heightTex,drift,fishingBoat);
    glViewport(0,0,width,height);glUseProgram(program);glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,heightTex);
    glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,coastTex);
    glActiveTexture(GL_TEXTURE2);glBindTexture(GL_TEXTURE_2D,seabedTex);
    glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_2D,nightTex);
    glActiveTexture(GL_TEXTURE4);glBindTexture(GL_TEXTURE_2D,skylineTex);
    glActiveTexture(GL_TEXTURE5);glBindTexture(GL_TEXTURE_2D,activityTex);
    glActiveTexture(GL_TEXTURE6);glBindTexture(GL_TEXTURE_2D,celestialTex);
    glActiveTexture(GL_TEXTURE7);glBindTexture(GL_TEXTURE_2D,cloudTex);
    glActiveTexture(GL_TEXTURE8);glBindTexture(GL_TEXTURE_2D,boat.color());
    glActiveTexture(GL_TEXTURE9);glBindTexture(GL_TEXTURE_2D,boat.reflection());
    glActiveTexture(GL_TEXTURE10);glBindTexture(GL_TEXTURE_3D,volumeNoiseTex);
    glActiveTexture(GL_TEXTURE11);glBindTexture(GL_TEXTURE_2D,peopleTex);
    glActiveTexture(GL_TEXTURE12);glBindTexture(GL_TEXTURE_2D,tavernaDayTex);
    glActiveTexture(GL_TEXTURE13);glBindTexture(GL_TEXTURE_2D,tavernaNightTex);
    glActiveTexture(GL_TEXTURE14);glBindTexture(GL_TEXTURE_2D,waiterTex);
    glActiveTexture(GL_TEXTURE15);glBindTexture(GL_TEXTURE_2D,idleTex);
    glUniform1i(glGetUniformLocation(program,"showBoat"),fishingBoat);
    glUniform1i(glGetUniformLocation(program,"activeBoat"),activeGreekBoat(seconds));
    for(int i=0;i<boatKinds;++i){
        const auto &ship=vessel[i];auto prefix=std::to_string(i)+"]";
        glUniform1f(glGetUniformLocation(program,("boatPhases["+prefix).c_str()),ship.phase);
        glUniform3f(glGetUniformLocation(program,("boatPositions["+prefix).c_str()),ship.position.x(),ship.position.y(),ship.position.z());
        glUniform3f(glGetUniformLocation(program,("boatLamps["+prefix).c_str()),ship.lamp.x(),ship.lamp.y(),ship.lamp.z());
    }
    glUniform3fv(glGetUniformLocation(program,"sunDirection"),1,sun.direction.data());
    glUniform1f(glGetUniformLocation(program,"daylight"),sun.daylight);
    glUniform1f(glGetUniformLocation(program,"twilight"),sun.twilight);
    glUniform1f(glGetUniformLocation(program,"lights"),sun.lights);
    glUniform1f(glGetUniformLocation(program,"localHour"),sun.localHour);
    glUniform3fv(glGetUniformLocation(program,"moonDirection"),1,sun.moonDirection.data());
    glUniform1f(glGetUniformLocation(program,"moonIllumination"),sun.moonIllumination);
    glUniform1f(glGetUniformLocation(program,"moonAngularRadius"),sun.moonAngularRadius);
    glUniform1i(glGetUniformLocation(program,"showCelestials"),celestials);
    glUniform1i(glGetUniformLocation(program,"showClouds"),clouds);
    glUniform1i(glGetUniformLocation(program,"showActivity"),activity);
    glUniform1f(glGetUniformLocation(program,"activityScale"),activityScale);
    glUniform2f(glGetUniformLocation(program,"resolution"),float(width),float(height));
    glUniform1f(glGetUniformLocation(program,"time"),float(seconds));
    glUniform1i(glGetUniformLocation(program,"drift"),drift);
    glDisable(GL_DEPTH_TEST);glDrawArrays(GL_TRIANGLES,0,3);
    GLenum error=glGetError();if(error!=GL_NO_ERROR)throw std::runtime_error("OpenGL error "+std::to_string(error));
}
void Renderer::snapshot(const std::string &path,int width,int height){
    std::vector<unsigned char> pixels(size_t(width)*height*4),flipped(pixels.size());
    glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    for(int y=0;y<height;++y)std::copy_n(pixels.data()+size_t(height-y-1)*width*4,width*4,flipped.data()+size_t(y)*width*4);
    png_image image{};image.version=PNG_IMAGE_VERSION;image.width=width;image.height=height;image.format=PNG_FORMAT_RGBA;
    if(!png_image_write_to_file(&image,path.c_str(),0,flipped.data(),0,nullptr))throw std::runtime_error("Cannot write snapshot "+path);
    std::cerr<<"Snapshot: "<<path<<'\n';
}
