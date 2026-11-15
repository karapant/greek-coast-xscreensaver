#version 330 core
in vec3 world,unmirrored,norm;
in vec2 uv;
flat in int material;
out vec4 frag;
uniform sampler2D materials,heightMap;
uniform vec3 camera,sunDirection,moonDirection,lampPosition,cabinLamp;
uniform float daylight,lights,moonIllumination,patchLength;
uniform bool mirrored;
void main(){
    float sea=texture(heightMap,unmirrored.xz/patchLength).r;
    if(unmirrored.y<sea-.025)discard;
    vec3 n=normalize(norm);if(!gl_FrontFacing)n=-n;
    vec3 albedo;
    if(material<4){vec2 st=fract(uv*vec2(2.,1.5));albedo=texture(materials,vec2((float(material)+.02+st.x*.96)/4.,st.y)).rgb;}
    else if(material==16)albedo=texture(materials,vec2(.76+fract(uv.x)*.23,fract(uv.y))).rgb;
    else if(material==4)albedo=vec3(.055,.13,.16);
    else if(material==5)albedo=vec3(.17,.15,.12);
    else if(material==9)albedo=vec3(.23,.28,.19)*( .7+.3*sin(uv.x*150.)*sin(uv.y*160.));
    else if(material==12)albedo=vec3(.67,.42,.27);
    else if(material==13)albedo=vec3(.82,.78,.63);
    else if(material==14)albedo=vec3(.06,.14,.13);
    else if(material==15)albedo=vec3(.09,.23,.34);
    else if(material==11)albedo=vec3(.84,.30,.07);
    else albedo=vec3(.84,.80,.68);
    float sun=max(dot(n,normalize(sunDirection)),0.)*smoothstep(-.03,.10,sunDirection.y);
    float moon=max(dot(n,normalize(moonDirection)),0.)*moonIllumination*smoothstep(0.,.15,moonDirection.y)*(1.-daylight);
    vec3 light=mix(vec3(.035,.055,.085),vec3(.46,.49,.50),daylight)+vec3(.75,.68,.52)*sun+vec3(.04,.05,.075)*moon;
    vec3 toLamp=lampPosition-unmirrored;
    float lamp=lights*max(dot(n,normalize(toLamp)),.08)/(1.+dot(toLamp,toLamp)*1.7);
    vec3 color=albedo*light+albedo*vec3(1.,.78,.42)*lamp*3.0;
    // Local ceiling lamp lights the captain and helm; indirect fill avoids
    // a black daytime cabin while preserving visible spatial depth at night.
    vec3 cabinDelta=unmirrored-cabinLamp;
    color+=albedo*vec3(1.,.72,.40)*lights*.85/(1.+dot(cabinDelta,cabinDelta)*3.);
    if(material==14)color+=vec3(.035,.14,.08)*lights;
    if(material==4){
        vec3 r=reflect(normalize(world-camera),n);
        vec3 glass=mix(vec3(.06,.09,.12),vec3(.39,.54,.64),daylight);
        glass+=vec3(.12,.16,.20)*pow(max(r.y,0.),3.)*daylight;
        frag=vec4(glass,.10+.16*pow(1.-abs(dot(normalize(camera-world),n)),3.));return;
    }

    if(material==6)color=mix(albedo,vec3(1.,.88,.60),lights);
    if(material==7)color=mix(vec3(.14,.015,.01),vec3(.75,.025,.012),lights);
    if(material==8)color=mix(vec3(.01,.12,.035),vec3(.015,.60,.13),lights);
    if(mirrored)color*=.87;
    frag=vec4(clamp(color,0.,1.),1.);
}
