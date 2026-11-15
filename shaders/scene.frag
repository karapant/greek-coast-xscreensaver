#version 330 core
in vec2 uv;
out vec4 frag;
uniform sampler2D heightMap, coastMap, seabedMap, nightMap, skylineMap, activityMap;
uniform sampler2D celestialMap,cloudMap;
uniform sampler2D boatColorMap,boatReflectionMap;
uniform sampler3D volumeNoiseMap;
uniform sampler2D idleMap;
uniform bool showFish;
uniform vec4 fishPlaces[24],fishHeading[24],fishBounds[3];
uniform vec4 idleBounds[6],personPlaces[24],personStyles[24];
uniform int personCount;
uniform sampler2D peopleMap,tavernaDayMap,tavernaNightMap,waiterMap;
uniform vec4 waiterBounds[8];
uniform bool showTaverna;
uniform vec4 peopleBounds[8];
uniform float localHour;
uniform bool showBoat;
uniform int activeBoat;
uniform vec3 boatPositions[3],boatLamps[3];
uniform float boatPhases[3];
uniform int cloudSteps;
uniform vec4 activityBounds[6];
uniform vec4 roadSamples[64];
uniform vec2 resolution;
uniform float time, patchLength, wind;
uniform bool drift;
uniform vec3 sunDirection;
uniform float daylight,twilight,lights;
uniform vec3 moonDirection;
uniform float moonIllumination,moonAngularRadius;
uniform bool showCelestials;
uniform bool showClouds,showActivity;
uniform float activityScale;
float moonlight();

float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float noise(vec2 p){
    vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);
    return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),mix(hash(i+vec2(0,1)),hash(i+vec2(1)),f.x),f.y);
}
float cloudDensity(vec3 p,vec3 center,vec3 extent,float cell){
    vec3 q=(p-center)/extent;
    if(any(greaterThan(abs(q),vec3(1))))return 0.;
    // Rounded overlapping 3D lobes, eroded by a trilinear noise field.
    // The generated cloud alpha adds subtle wispy variation, not a flat shell.
    float field=0.;
    for(int lobe=0;lobe<7;++lobe){
        float id=float(lobe),seed=id+cell*11.;
        vec3 c=vec3((id-3.)*.205,(lobe>1&&lobe<5)?hash(vec2(seed,9.))*.25:-.12,hash(vec2(seed,4.))*.34-.17);
        vec3 radius=vec3(.32,.30+hash(vec2(seed,8.))*.20,.57);
        vec3 d=(q-c)/radius;field+=exp(-dot(d,d)*1.8);
    }
    vec2 local=vec2(q.x,-q.y)*.5+.5;
    float wisps=textureLod(cloudMap,vec2((cell+local.x)/3.,local.y),2.).a;
    float erosion=texture(volumeNoiseMap,p*.00011).r*.25+texture(volumeNoiseMap,p*.00031).g*.12;
    return smoothstep(.16,.72,field-erosion+wisps*.04);

}
vec3 cloudLayer(vec3 background,vec3 ray){
    if(!showClouds)return background;
    vec3 origin=vec3(drift?sin(time*.045)*.065:0.,2.7,10.);
    for(int i=0;i<3;++i){
        float id=float(i),lifetime=900.+id*90.;
        float age=(time+id*240.+360.)/lifetime;
        float epoch=floor(age),seed=id*17.+epoch*3.1;
        vec3 center=vec3(mix(-8200.,8200.,fract(age)),900.+hash(vec2(seed,7.))*280.,-5400.-id*900.);
        vec3 extent=vec3(1000.+hash(vec2(seed,2.))*300.,650.,750.);
        vec3 inverse=1./(ray+vec3(.000001));
        vec3 a=(center-extent-origin)*inverse,b=(center+extent-origin)*inverse;
        vec3 lo=min(a,b),hi=max(a,b);
        float start=max(0.,max(max(lo.x,lo.y),lo.z)),finish=min(min(hi.x,hi.y),hi.z);
        if(finish<=start)continue;
        float cell=mod(id+epoch,3.);
        float stepSize=(finish-start)/float(cloudSteps),transmission=1.;vec3 scattered=vec3(0.);
        float jitter=hash(floor(gl_FragCoord.xy));float shadow=1.;
        vec3 source=normalize(sunDirection.y>0.?sunDirection:moonDirection);
        for(int step=0;step<40;++step){
            if(step>=cloudSteps||transmission<.015)break;
            vec3 p=origin+ray*(start+(float(step)+jitter)*stepSize);
            float density=cloudDensity(p,center,extent,cell);if(density<.005)continue;
            {float optical=0.;for(int k=1;k<=3;++k)optical+=cloudDensity(p+source*(float(k)*140.),center,extent,cell);shadow=exp(-optical*.65);}
            vec3 ambient=mix(vec3(.016,.022,.035),vec3(.52,.57,.64),daylight);
            vec3 sunlight=mix(vec3(1.,.68,.43),vec3(1.,.98,.93),smoothstep(0.,.20,sunDirection.y));
            float phase=.75+.25*pow(max(dot(ray,source),0.),3.);
            vec3 illumination=ambient+sunlight*shadow*.48*smoothstep(-.10,.10,sunDirection.y)*phase+vec3(.10,.12,.17)*moonlight()*shadow;
            vec3 q=(p-center)/extent;
            vec4 detail=textureLod(cloudMap,vec2((cell+q.x*.5+.5)/3.,.5-q.y*.5),1.);
            illumination*=mix(vec3(.86),detail.rgb,detail.a*.65);
            float opacity=1.-exp(-density*stepSize*.006);
            scattered+=transmission*opacity*illumination;transmission*=1.-opacity;
        }
        background=background*transmission+scattered;
    }
    return background;
}
vec4 roadSprite(vec2 p,vec2 feet,vec2 size,float column,float row,bool reverse){
    vec2 local=(p-(feet-vec2(size.x*.5,size.y)))/size;
    if(any(lessThan(local,vec2(0)))||any(greaterThan(local,vec2(1))))return vec4(0);
    if(reverse)local.x=1.-local.x;
    vec4 bounds=activityBounds[int(row)*3+int(column)];
    vec2 cell=mix(bounds.xy,bounds.zw,local);
    vec4 sprite=texture(activityMap,cell);
    sprite.rgb*=mix(vec3(.13,.17,.24),vec3(1.),daylight);
    return sprite;
}
// Distant hillside terrace, fixed to the photographic village.
vec3 tavernaComposite(vec3 color,vec2 plateUV){
    if(!showTaverna)return color;
    // Fixed photograph coordinates keep the building attached to the hill at
    // every window aspect ratio; original plate is approximately 16:9.
    vec2 size=vec2(.082,.049);
    vec2 p=(plateUV-vec2(.294,.272))/size;
    if(any(lessThan(p,vec2(0)))||any(greaterThan(p,vec2(1))))return color;
    float dinner=smoothstep(.05,.85,lights);
    vec4 day=texture(tavernaDayMap,p),night=texture(tavernaNightMap,p);
    day.rgb*=mix(vec3(.29,.33,.43),vec3(1.),daylight);
    vec4 terrace=mix(day,night,dinner);
    terrace.a=smoothstep(.08,.94,terrace.a);
    color=mix(color,terrace.rgb,terrace.a);
    return color;
}
// CPU agents supply grounded, depth-sorted poses. Walking frames follow
// distance travelled; stationary agents use planted-foot idle/serving artwork.
vec3 peopleLayer(vec3 color,vec2 p){
    if(!showActivity||p.x>.40||p.y<.25||p.y>.34)return color;
    float photoAspect=16./9.;
    for(int i=0;i<24;++i){
        if(i>=personCount)break;
        vec4 place=personPlaces[i],style=personStyles[i];
        float h=place.z;vec2 size=vec2(h*.47/photoAspect,h);
        vec2 q=(p-place.xy+vec2(size.x*.5,size.y))/size;
        // A contact shadow anchors feet without bobbing the ground position.
        vec2 shadow=(p-place.xy-vec2(.00015,.00008))/vec2(h*.20/photoAspect,h*.035);
        color*=1.-exp(-dot(shadow,shadow))*place.w*mix(.10,.20,daylight);
        if(any(lessThan(q,vec2(0)))||any(greaterThan(q,vec2(1))))continue;
        if(style.z>.5)q.x=1.-q.x;
        int pose=int(style.x),atlas=int(style.y);vec4 person;
        if(atlas==3){vec4 b=idleBounds[pose];person=texture(idleMap,mix(b.xy,b.zw,q));}
        else if(atlas==2){vec4 b=waiterBounds[pose];person=texture(waiterMap,mix(b.xy,b.zw,q));}
        else {vec4 b=peopleBounds[atlas*4+pose];person=texture(peopleMap,mix(b.xy,b.zw,q));}
        person.a=smoothstep(.22,.88,person.a)*place.w;
        vec3 nightTint=mix(vec3(.32,.26,.20),vec3(.72,.58,.40),style.w);
        person.rgb*=mix(nightTint,vec3(1.),daylight);
        color=mix(color,person.rgb,person.a);
    }
    return color;
}

vec4 roadAt(float t){
    // Smoothly interpolated, equal-distance samples of the photographed lane.
    float k=clamp(t,0.,.99999)*63.;int n=int(k);
    return mix(roadSamples[n],roadSamples[n+1],fract(k));
}
vec2 villageRoad(float t){return roadAt(t).xy;}
vec3 villageLife(vec3 base,vec2 p){
    if(!showActivity||p.x>.40||p.y<.27||p.y>.35)return base;
    float detail=activityScale*clamp(360./resolution.y,1.,1.5);
    // One passing car at a time, separated by quiet intervals.
    float age=(time+12.)/60.,epoch=floor(age),progress=fract(age);
    if(progress<.60&&(epoch<1.||hash(vec2(epoch,11.))<.90)){
        bool reverse=hash(vec2(epoch,6.))>.5;
        float trip=progress/.60;
        float path=trip<.44?.42*smoothstep(0.,.36,trip):.42+.58*smoothstep(.44,1.,trip);
        float braking=smoothstep(.28,.36,trip)*(1.-smoothstep(.44,.49,trip));
        if(!reverse)path=1.-path;
        vec4 route=roadAt(path);vec2 feet=route.xy;
        float aspect=resolution.x/resolution.y;
        vec2 tangent=normalize(route.zw*vec2(aspect,1.));
        vec2 across=vec2(-tangent.y,tangent.x);
        vec2 delta=(p-feet)*vec2(aspect,1.);
        // Rotate both the body and its lamps to the local road heading.
        vec2 aligned=feet+vec2(dot(delta,tangent)/aspect,dot(delta,across));
        float visible=smoothstep(0.,.025,trip)*(1.-smoothstep(.975,1.,trip));
        vec4 car=roadSprite(aligned,feet,vec2(.0095,.0062)*detail,floor(hash(vec2(epoch,4.))*3.),1.,reverse);
        vec2 contact=(aligned-feet+vec2(0,.0001)*detail)/(vec2(.0043,.00045)*detail);
        base*=1.-exp(-dot(contact,contact))*visible*.16;
        base=mix(base,car.rgb,car.a*visible);
        float side=reverse?1.:-1.;
        vec2 head=feet+vec2(side*.0036,-.0017)*detail;
        vec2 tail=feet-vec2(side*.0036,.0017)*detail;
        vec2 beam=(aligned-head-vec2(side*.003,.0006)*detail)/(vec2(.0045,.0012)*detail);
        base+=vec3(.55,.46,.28)*exp(-dot(beam,beam))*lights*.065*visible;
        vec2 d=(aligned-head)/(vec2(.0013,.0010)*detail);
        base+=vec3(.95,.79,.45)*exp(-dot(d,d)*2.)*lights*.42*visible;
        d=(aligned-tail)/(vec2(.0008,.0008)*detail);
        base+=vec3(.30,.012,.005)*exp(-dot(d,d)*2.)*lights*(1.+braking*2.2)*visible;
    }
    return base;
}
float waves(vec2 p){
    float h=texture(heightMap,p/patchLength).r;
    h+=.012*sin(dot(p,vec2(2.7,1.4))-time*4.1);
    h+=.006*sin(dot(p,vec2(-4.9,3.8))-time*6.5);
    return h;
}
float boatWake(vec2 p){
    if(!showBoat)return 0.;
    float wake=0.;
    for(int i=0;i<3;++i){
        if(i!=activeBoat)continue;
        float speed=i==1?3.:i==2?1.85:1.22;
        float stern=i==1?7.:i==2?3.9:2.7;
        float age=((i==1?p.x-boatPositions[i].x:boatPositions[i].x-p.x)-stern)/speed;
        if(age<0.||age>38.||age>boatPhases[i])continue;
        float d=age*speed;
        float period=i==1?170.:i==2?240.:210.;
        float center=(i==1?-100.:i==2?-140.:-54.)-(i==1?9.:7.)*cos((boatPhases[i]-age)*6.2831853/period);
        float side=abs(p.y-center),beam=i==1?1.5:i==2?.95:.65;
        float arm=exp(-pow((side-(beam+d*.354))/(.18+d*.022),2.));
        float propeller=exp(-pow(side/((i==1?.85:.38)+d*.045),2.));
        float breakup=.45+.55*noise(p*3.7+vec2(time*.6,0));
        float strength=i==2?.26:.60;
        wake=max(wake,(arm*strength+propeller*(i==2?.10:.85))*exp(-age*.115)*smoothstep(0.,1.8,d)*breakup);
    }
    return clamp(wake,0.,.8);
}
vec3 boatComposite(vec3 color){if(!showBoat)return color;vec4 boat=texture(boatColorMap,uv);return mix(color,boat.rgb/max(boat.a,.0001),boat.a);}
vec3 normalAt(vec2 p,float distance){
    float e=.075;
    vec2 slope=vec2(waves(p+vec2(e,0))-waves(p-vec2(e,0)),waves(p+vec2(0,e))-waves(p-vec2(0,e)))/(2.*e);
    float fade=1./(1.+distance*.006);
    return normalize(vec3(-slope.x*fade,1.,-slope.y*fade));
}
float moonlight(){return showCelestials?pow(moonIllumination,1.7)*smoothstep(0.,.25,moonDirection.y)*(1.-daylight):0.;}
float directionalGlint(vec3 n,vec3 view,vec3 source){
    float nl=max(dot(n,source),0.),nv=max(dot(n,view),0.);
    if(nl<=0.||nv<=0.)return 0.;
    vec3 halfway=normalize(view+source);
    float nh=max(dot(n,halfway),0.),vh=max(dot(view,halfway),0.);
    float variance=max(dot(dFdx(n),dFdx(n)),dot(dFdy(n),dFdy(n)));
    float a2=.0016+clamp(variance,0.,.12)*.6,denominator=nh*nh*(a2-1.)+1.;
    float distribution=a2/(3.14159265*denominator*denominator);
    float geometry=(nl/(nl*.83+.17))*(nv/(nv*.83+.17));
    float fresnel=.0204+.9796*pow(1.-vh,5.);
    return min(distribution*geometry*fresnel/(4.*max(nv,.025)),4.);
}
vec3 sunColor(){return mix(vec3(1.,.29,.055),vec3(1.,.96,.83),smoothstep(0.,.40,sunDirection.y));}
vec3 celestialSky(vec3 base,vec3 r){
    if(!showCelestials)return base;
    vec3 sun=normalize(sunDirection),moon=normalize(moonDirection);
    float sunDistance=length(cross(r,sun));
    float sunAA=max(fwidth(sunDistance),.00012);
    float sunFront=step(0.,dot(r,sun))*step(0.,r.y);
    float sunDisc=(1.-smoothstep(.00465-sunAA,.00465+sunAA,sunDistance))*sunFront;
    float sunHalo=exp(-pow(sunDistance/.014,1.3))*.20*sunFront*smoothstep(-.009,.02,sun.y);
    base+=sunColor()*sunHalo;
    if(sunDisc>0.){
        vec3 right=normalize(cross(sun,vec3(0,1,0))),up=cross(right,sun);
        vec2 p=vec2(dot(r,right),dot(r,up))/sin(.00465);
        vec2 local=clamp(vec2(.5+.38*p.x,.5-.38*p.y),vec2(.001),vec2(.999));
        vec4 image=texture(celestialMap,vec2(local.x*.5,local.y));
        float exposure=mix(3.5,9.,smoothstep(0.,.12,sun.y));
        vec3 emission=1.-exp(-image.rgb*sunColor()*exposure);
        base=mix(base,emission,sunDisc*image.a);
    }
    float moonDistance=length(cross(r,moon));
    float moonAA=max(fwidth(moonDistance),.00012);
    float moonDisc=(1.-smoothstep(moonAngularRadius-moonAA,moonAngularRadius+moonAA,moonDistance))*step(0.,dot(r,moon))*step(0.,r.y);
    if(moonDisc>0.){
        vec3 right=normalize(cross(moon,vec3(0,1,0)));
        vec3 up=cross(right,moon);
        vec2 p=vec2(dot(r,right),dot(r,up))/sin(moonAngularRadius);
        vec3 surface=right*p.x+up*p.y-moon*sqrt(max(0.,1.-dot(p,p)));
        float illuminated=smoothstep(-.015,.04,dot(surface,sun));
        vec2 local=clamp(vec2(.5+.38*p.x,.5-.38*p.y),vec2(.001),vec2(.999));
        vec4 image=texture(celestialMap,vec2(.5+local.x*.5,local.y));
        vec3 atmosphere=mix(vec3(.95,.82,.67),vec3(1.),smoothstep(0.,.20,moon.y));
        vec3 lit=image.rgb*atmosphere*(.70+.30*max(dot(surface,sun),0.));
        // The lit hemisphere points toward the calculated sun, so crescents
        // rotate naturally; no fixed left/right phase mask is imposed.
        base=mix(base,lit,moonDisc*illuminated*image.a);
    }
    float moonHalo=exp(-pow(moonDistance/.012,1.4))*.035*moonlight()*step(0.,dot(r,moon));
    return base+vec3(.68,.76,.91)*moonHalo;
}
vec3 sky(vec3 r){
    float h=clamp(r.y,0.,1.);
    vec3 c=mix(vec3(.72,.85,.92),vec3(.22,.50,.79),pow(h,.45));
    vec3 sun=normalize(sunDirection);
    c+=vec3(1.,.89,.65)*pow(max(dot(r,sun),0.),180.)*.8*smoothstep(-.03,.08,sun.y);
    vec3 night=mix(vec3(.025,.040,.075),vec3(.006,.013,.035),pow(h,.45));
    vec3 result=mix(night,c,daylight);
    float sunsetGlow=pow(max(1.-abs(r.y),0.),6.)*(.3+.7*max(dot(normalize(vec3(sun.x,0.,sun.z)),normalize(vec3(r.x,0.,r.z))),0.));
    result+=vec3(.40,.12,.045)*twilight*sunsetGlow;
    result+=vec3(.35,.42,.55)*pow(max(dot(r,normalize(moonDirection)),0.),1500.)*moonlight();
    return result;
}
float caustics(vec2 p,float depth){
    // Light concentration from the curvature of the actual spectral surface.
    // Broad finite differences smooth the FFT lattice; no crossing sine grids.
    float e=patchLength/float(textureSize(heightMap,0).x)*1.6;
    vec2 u=p/patchLength,d=vec2(e/patchLength,0.);
    float h=texture(heightMap,u).r;
    float xx=(texture(heightMap,u+d).r+texture(heightMap,u-d).r-2.*h)/(e*e);
    float zz=(texture(heightMap,u+d.yx).r+texture(heightMap,u-d.yx).r-2.*h)/(e*e);
    float xz=(texture(heightMap,u+d+d.yx).r-texture(heightMap,u+d-d.yx).r-texture(heightMap,u-d+d.yx).r+texture(heightMap,u-d-d.yx).r)/(4.*e*e);
    float distance=depth*.25;
    float jacobian=(1.-distance*xx)*(1.-distance*zz)-distance*distance*xz*xz;
    return clamp(1./max(abs(jacobian),.55)-1.,-.25,.8);
}
float waterMask(vec3 c){return smoothstep(.015,.10,max(c.g,c.b)-c.r);}
vec3 coast(vec2 p){
    vec3 day=texture(coastMap,p).rgb,night=texture(nightMap,p).rgb;
    vec3 color=pow(mix(pow(night,vec3(2.2)),pow(day,vec3(2.2)),daylight),vec3(1./2.2));
    float nearHorizon=exp(-abs(p.y-.40)*9.);
    float sunsetSide=.4+.6*clamp(.5+sunDirection.x*(p.x-.5),0.,1.);
    color+=vec3(.23,.065,.018)*twilight*nearHorizon*sunsetSide;
    return color;
}
vec3 villageReflection(vec3 n){
    vec3 reflectedLights=vec3(0.);
    float jitter=n.x*.055+sin(time*1.8+uv.y*135.)*.002;
    float stretch=1.+(.60-uv.y)*.5;
    float x=.20+(uv.x-.20)/stretch+jitter;
    // Sample real lit buildings and street lamps from the aligned night plate.
    for(int i=0;i<12;++i){
        vec3 lamp=texture(nightMap,vec2(x,.215+float(i)*.010)).rgb;
        float warm=smoothstep(.08,.24,lamp.r-max(lamp.b,lamp.g*.6));
        reflectedLights+=max(lamp-vec3(.12,.10,.08),0.)*warm;
    }
    float breakup=noise(vec2(uv.x*38.+n.x*8.,uv.y*245.+time*.7));
    float ripple=.08+.92*smoothstep(.46,.76,breakup);
    return reflectedLights*.16*lights*ripple*exp(-(.60-uv.y)*8.5);
}
vec3 fishLayer(vec3 below,vec3 surface,vec3 ray,vec3 normal,float depth,vec3 ambient,float caustic){
    if(!showFish)return below;
    vec3 bent=refract(ray,normal,1./1.333);
    for(int i=0;i<24;++i){
        vec4 fish=fishPlaces[i],heading=fishHeading[i];
        if(-fish.y>depth-.15)continue;
        float distance=(fish.y-surface.y)/min(bent.y,-.0001);
        if(distance<0.)continue;
        vec2 delta=surface.xz+bent.xz*distance-fish.xz;
        if(dot(delta,delta)>fish.w*fish.w)continue;
        vec2 direction=normalize(heading.xy),across=vec2(-direction.y,direction.x);
        vec2 q=vec2(dot(delta,direction)/fish.w+.5,dot(delta,across)/(fish.w*.34)+.5);
        q.y+=sin(time*(4.+length(heading.xy)*7.)+heading.w)*.16*pow(max(0.,1.-q.x/.38),2.);
        if(any(lessThan(q,vec2(0)))||any(greaterThan(q,vec2(1))))continue;
        vec4 bounds=fishBounds[int(heading.z)],texel=texture(idleMap,mix(bounds.xy,bounds.zw,q));
        float alpha=smoothstep(.18,.85,texel.a);
        vec3 absorption=exp(vec3(.44,.12,.075)*fish.y);
        vec3 color=texel.rgb*ambient*absorption*(.72+caustic*.24);
        below=mix(below,color,alpha*.96);
    }
    return below;
}
void main(){
    vec2 plateUV=vec2(uv.x,1.-uv.y);
    vec3 originalPlate=texture(coastMap,plateUV).rgb;
    vec3 plate=coast(plateUV);
    plate=tavernaComposite(plate,plateUV);
    plate=villageLife(plate,plateUV);
    plate=peopleLayer(plate,plateUV);
    // The photographic plate supplies the distant land and sky. The sea below
    // the horizon is replaced with a three-dimensional spectral water surface.
    float horizon=.60;
    float aspect=resolution.x/resolution.y;
    float sway=drift?sin(time*.045)*.065:0.;
    vec3 origin=vec3(sway,2.7,10.);
    vec3 ray=normalize(vec3((uv.x-.5)*aspect*1.12,(uv.y-horizon)*1.05,-1.));
    if(uv.y>=horizon){
        float edge=texture(skylineMap,vec2(uv.x,.5)).r;
        float skyMask=1.-smoothstep(edge-.0008,edge+.0008,plateUV.y);
        vec3 bodies=celestialSky(plate,ray);
        bodies=cloudLayer(bodies,ray);
        frag=vec4(boatComposite(mix(plate,bodies,skyMask)),1);return;
    }
    float travel=-origin.y/min(ray.y,-.0001);
    for(int i=0;i<4;++i){
        vec3 p=origin+ray*travel;
        travel=(waves(p.xz)-origin.y)/min(ray.y,-.0001);
    }
    travel=clamp(travel,0.,3000.);
    vec3 pos=origin+ray*travel;
    vec3 n=normalAt(pos.xz,travel);
    vec3 reflected=reflect(ray,n);
    float fresnel=.0204+.9796*pow(1.-max(dot(n,-ray),0.),5.);
    float depth=1.3+2.5*noise(pos.xz*.032)+.4*sin(pos.x*.07);
    depth=mix(depth,16.,smoothstep(20.,160.,travel));
    vec2 refractedUV=(pos.xz+n.xz*depth*.25)*.18;
    vec3 sand=texture(seabedMap,refractedUV).rgb;
    sand=mix(sand,vec3(.60,.61,.43),smoothstep(.68,.92,noise(pos.xz*.16))*.32);
    vec3 transmission=exp(-vec3(.44,.12,.075)*depth);
    float light=caustics(pos.xz+n.xz*.5,depth)*exp(-depth*.16)*(1.-smoothstep(18.,65.,travel))*daylight;
    vec3 underwaterLight=mix(vec3(.018,.032,.055)+vec3(.02,.028,.04)*moonlight(),vec3(1.),daylight);
    vec3 below=sand*(.78+light*.40)*transmission*underwaterLight;
    if(travel<55.)below=fishLayer(below,pos,ray,n,depth,underwaterLight,light);
    vec3 water=below+mix(vec3(.001,.009,.016),vec3(.012,.34,.40),daylight)*(1.-transmission);
    water=mix(water,sky(reflected),clamp(fresnel,0.,1.));
    vec3 sun=normalize(sunDirection);
    float sparkle=pow(max(dot(reflect(-sun,n),-ray),0.),320.);
    water+=mix(vec3(1.,.55,.28),vec3(1.,.92,.73),daylight)*(sparkle*1.2+directionalGlint(n,-ray,sun)*.45)*smoothstep(-.03,.08,sun.y);
    water+=villageReflection(n);
    float moonSparkle=pow(max(dot(reflect(-normalize(moonDirection),n),-ray),0.),540.);
    water+=vec3(.32,.40,.56)*(moonSparkle+directionalGlint(n,-ray,normalize(moonDirection))*.35)*moonlight();
    if(showBoat){
        vec4 reflectedBoat=texture(boatReflectionMap,uv+n.xz*vec2(.010,.007));
        water=mix(water,reflectedBoat.rgb/max(reflectedBoat.a,.0001),reflectedBoat.a*(.18+.65*fresnel));
        for(int i=0;i<3;++i){
            if(i!=activeBoat)continue;
            vec2 lightDelta=pos.xz-boatLamps[i].xz;
            float pool=exp(-dot(lightDelta,lightDelta)/(i==0?8.:12.))*lights;
            water+=vec3(.24,.24,.13)*pool*(i==0?1.:.45)*(.72+noise(pos.xz*2.+time*.2)*.28);
        }
    }
    // Sparse foam on short, steep crests rather than a constant white overlay.
    float crest=smoothstep(.24,.37,pos.y)*smoothstep(.13,.30,length(n.xz));
    float foam=crest*noise(pos.xz*3.2+time*.22)*.35;
    vec3 foamColor=mix(vec3(.04,.06,.10),vec3(.91,.98,.95),daylight);
    water=mix(water,foamColor,foam);
    float wake=boatWake(pos.xz);
    water=mix(water,foamColor,wake);
    water=mix(water,mix(vec3(.012,.025,.055),vec3(.42,.65,.77),daylight),1.-exp(-travel*.00075));
    // Preserve limestone promontories in the plate and blend a narrow live surf edge.
    float waterRegion=mix(1.,waterMask(originalPlate),smoothstep(.43,.52,uv.y));
    float nearShore=1.-waterMask(texture(coastMap,plateUV-vec2(0,.004)).rgb);
    float shoreFoam=nearShore*waterRegion*smoothstep(.48,.54,uv.y)*(.35+.3*sin(time*1.7+uv.x*44.));
    water=mix(water,foamColor,shoreFoam*.3);
    float highlight=max(max(water.r,water.g),water.b);
    if(highlight>.85)water*=(.85+.15*(1.-exp(-(highlight-.85)/.15)))/highlight;
    float horizonFade=1.-smoothstep(horizon-.015,horizon,uv.y);
    vec3 color=mix(plate,water,waterRegion*horizonFade);
    color*=1.-.08*pow(length((uv-.5)*vec2(.8,1.)),2.);
    color=boatComposite(color);
    frag=vec4(clamp(color,0.,1.),1.);
}
