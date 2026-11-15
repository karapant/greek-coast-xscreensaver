#version 330 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;
layout(location=3) in float materialId;
uniform mat4 model;
uniform vec3 camera;
uniform vec2 resolution;
uniform bool mirrored;
uniform float waterLevel,animationTime;
out vec3 world,unmirrored,norm;
out vec2 uv;
flat out int material;
void main(){
    vec3 local=position;
    if(materialId>15.5)local.z+=.035*sin(animationTime*1.3+local.y*1.7)*sin(clamp((local.y-2.5)/8.3,0.,1.)*3.14159);
    vec4 p=model*vec4(local,1.);
    unmirrored=p.xyz;norm=normalize(mat3(model)*normal);
    if(mirrored){p.y=2.*waterLevel-p.y;norm.y=-norm.y;}
    world=p.xyz;uv=texcoord;material=int(materialId+.5);
    vec3 view=p.xyz-camera;float w=-view.z;
    gl_Position=vec4(2.*view.x/(resolution.x/resolution.y*1.12),2.*view.y/1.05+.20*w,1.0001*w-.20,w);
}
