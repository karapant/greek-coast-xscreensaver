#include "renderer.h"
#include "environment.h"
#include "termination.h"
#include <QCoreApplication>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLocale>
#include <epoxy/egl.h>
#include <epoxy/glx.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
volatile std::sig_atomic_t running=1;
volatile std::sig_atomic_t redrawRequested=0;
void continued(int){redrawRequested=1;}
void stop(int){running=0;}
struct Options {
    CycleSettings cycle=CycleSettings::load();
    int width=1280,height=720,fps=60,frames=0,grid=128;
    float wind=5,detailScale=1.8f;double startTime=0;
    bool root=false,fullscreen=false,headless=false,drift=true;
    bool celestials=true,skyInfo=false,clouds=true,activity=true,fishingBoat=true,taverna=true,fish=true;
    unsigned long window=0;
    std::string snapshot,display,assets,shaders;
    Options(){
        auto app=std::filesystem::path(QCoreApplication::applicationDirPath().toStdString());
        auto data=(app/GREEK_COAST_DATA_RELATIVE).lexically_normal();
        if(!std::filesystem::exists(data/"assets/coast.png"))data=app/"share/greek-coast";
        assets=(data/"assets").string();shaders=(data/"shaders").string();
    }
};
double decimalOption(const std::string &value){
    // QCoreApplication adopts the desktop locale. Command lines emitted by
    // XScreenSaver/QString::number still use a dot, so std::stod can silently
    // read "0.1" as zero in el_GR. Never depend on LC_NUMERIC for options.
    QString text=QString::fromStdString(value);
    if(text.count(',')==1&&!text.contains('.'))text.replace(',','.');
    QLocale numeric(QLocale::C);numeric.setNumberOptions(QLocale::RejectGroupSeparator);
    bool valid=false;double result=numeric.toDouble(text,&valid);
    if(!valid||!std::isfinite(result))throw std::invalid_argument("Invalid numeric option: "+value);
    return result;
}
Options parse(int argc,char **argv){
    Options o;bool modeProvided=false,speedProvided=false,headingProvided=false;
    auto value=[&](int &i){if(++i>=argc)throw std::runtime_error(std::string("Missing value after ")+argv[i-1]);return std::string(argv[i]);};
    for(int i=1;i<argc;++i){
        const std::string a=argv[i];
        if(a=="--help"||a=="-help"||a=="-h"){
            std::cout<<"Greek Coast — live spectral waves on a Greek shoreline\n"
                "  --root / -root          Render in XScreenSaver's window\n"
                "  --window-id ID          Embed in an X11 window (hex or decimal)\n"
                "  --fullscreen            Standalone fullscreen; input exits\n"
                "  --wind N                Wind speed 1–20 m/s (default 5)\n"
                "  --fps N                 Frame cap 1–120 (default 60)\n"
                "  --quality low|medium|high  Spectral grid 64/128/256\n"
                "  --no-drift              Lock camera position\n"
                "  --width N --height N    Standalone/ snapshot size\n"
                "  --headless              Offscreen EGL, no display required\n"
                "  --snapshot FILE         Save a PNG and exit\n"
                "  --time SECONDS          Initial animation time\n"
                "  --cycle real|accelerated  Solar clock mode\n"
                "  --speed N               Solar time multiplier, 0.1–1000\n"
                "  --heading DEGREES       View direction from north (default 270, west)\n"
                "  --no-celestials         Hide sun and moon discs\n"
                "  --no-clouds             Disable drifting clouds\n"
                "  --no-activity           Disable distant villagers and cars\n"
                "  --activity-scale N      Village detail size, 0.75–2.5 (default 1.8)\n"
                "  --no-boat               Disable the Greek boat rotation\n"
                "  --no-fish               Disable small underwater fish shoals\n"
                "  --no-taverna            Disable the hillside taverna\n"
                "  --sky-info              Print calculated sky positions and exit\n"
                "  --date ISO_DATE         Start at a chosen Greece date/time\n"
                "  --settings              Open Greek Coast Settings\n"
                "  --frames N              Exit after N frames\n"
                "  --assets DIR --shaders DIR  Override asset paths\n";std::exit(0);
        }else if(a=="--root"||a=="-root")o.root=true;
        else if(a=="--window-id"||a=="-window-id")o.window=std::stoul(value(i),nullptr,0);
        else if(a=="--fullscreen")o.fullscreen=true;
        else if(a=="--headless")o.headless=true;
        else if(a=="--no-drift")o.drift=false;
        else if(a=="--wind")o.wind=float(decimalOption(value(i)));
        else if(a=="--fps")o.fps=std::stoi(value(i));
        else if(a=="--width")o.width=std::stoi(value(i));
        else if(a=="--height")o.height=std::stoi(value(i));
        else if(a=="--frames")o.frames=std::stoi(value(i));
        else if(a=="--time")o.startTime=decimalOption(value(i));
        else if(a=="--cycle"){o.cycle.mode=QString::fromStdString(value(i));modeProvided=true;}
        else if(a=="--speed"){o.cycle.speed=decimalOption(value(i));speedProvided=true;}
        else if(a=="--heading"){o.cycle.heading=decimalOption(value(i));headingProvided=true;}
        else if(a=="--no-celestials")o.celestials=false;
        else if(a=="--sky-info")o.skyInfo=true;
        else if(a=="--no-clouds")o.clouds=false;
        else if(a=="--no-activity")o.activity=false;
        else if(a=="--no-boat")o.fishingBoat=false;
        else if(a=="--no-fish")o.fish=false;
        else if(a=="--no-taverna")o.taverna=false;
        else if(a=="--activity-scale")o.detailScale=float(decimalOption(value(i)));
        else if(a=="--date"){
            o.cycle.startUtc=parseGreeceDate(QString::fromStdString(value(i)));
            o.cycle.startNow=false;o.cycle.mode="accelerated";modeProvided=true;
        }
        else if(a=="--snapshot")o.snapshot=value(i);
        else if(a=="--display"||a=="-display")o.display=value(i);
        else if(a=="--assets")o.assets=value(i);
        else if(a=="--shaders")o.shaders=value(i);
        else if(a=="--quality"){
            auto q=value(i);if(q=="low")o.grid=64;else if(q=="medium")o.grid=128;else if(q=="high")o.grid=256;else throw std::runtime_error("Quality must be low, medium, or high");
        }else throw std::runtime_error("Unknown option: "+a);
    }
    if(o.width<64||o.width>7680||o.height<64||o.height>4320||o.fps<1||o.fps>120||o.frames<0||!std::isfinite(o.startTime))throw std::runtime_error("Invalid size, frame count, frame cap, or time");
    if(!o.snapshot.empty()&&!o.frames)o.frames=1;
    if(o.headless&&!o.frames)o.frames=1;
    if(o.headless&&(o.root||o.window))throw std::runtime_error("Headless rendering cannot embed in an X11 window");
    // Native XML default menu entries intentionally emit no command option.
    // Keep those defaults independent of optional standalone preferences.
    if(o.root||o.window){
        if(!modeProvided)o.cycle.mode="real";
        if(!speedProvided)o.cycle.speed=1.;
        if(!headingProvided)o.cycle.heading=270.;
    }
    o.cycle.validate();
    if(!std::isfinite(o.detailScale)||o.detailScale<.75f||o.detailScale>2.5f)throw std::runtime_error("Activity scale must be between 0.75 and 2.5");
    return o;
}
class Context {
    inline static Context *active=nullptr;
    XErrorHandler previousErrorHandler=nullptr;
    int glxErrorBase=-100;
    bool drawableLost=false,drawableReady=true,needsRestore=false;
    Atom saverStatus=0;
    bool ownsWindow=false,doubleBuffered=true,visible=true;
    std::chrono::steady_clock::time_point lastVisibilityProbe{};
    void restoreDrawable(){
        if(!window||drawableLost)return;
        needsRestore=false;drawableReady=true;
        glXMakeCurrent(display,None,nullptr);
        if(!glXMakeCurrent(display,window,glx)){drawableReady=false;needsRestore=true;return;}
        XSync(display,False);
        if(!drawableReady)return;
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glDrawBuffer(doubleBuffered?GL_BACK:GL_FRONT);
        glReadBuffer(doubleBuffered?GL_BACK:GL_FRONT);
        glViewport(0,0,std::max(width,1),std::max(height,1));
    }
    static int errorHandler(Display *d,XErrorEvent *event){
        auto *context=active;
        if(context&&d==context->display){
            const int code=event->error_code;
            // A host destroying its preview window is a normal shutdown race.
            if(code==BadWindow){context->drawableLost=true;running=0;return 0;}
            // A live host may temporarily lose its GLX drawable while the
            // compositor/fullscreen preview redirects it. Retry after return.
            if(code==BadDrawable||code==context->glxErrorBase+2||
               code==context->glxErrorBase+5||code==context->glxErrorBase+11||code==context->glxErrorBase+12){
                context->drawableReady=false;context->needsRestore=true;return 0;
            }
            if(context->previousErrorHandler)return context->previousErrorHandler(d,event);
        }
        return 0;
    }
public:
    Display *display=nullptr;Window window=0,parent=0;GLXContext glx=nullptr;
    EGLDisplay egl=EGL_NO_DISPLAY;EGLContext eglContext=EGL_NO_CONTEXT;EGLSurface surface=EGL_NO_SURFACE;
    Colormap colormap=0;Atom wmDelete=0;bool embedded=false;
    int width,height;
    explicit Context(const Options &o):width(o.width),height(o.height){
        if(o.headless){
            auto getPlatformDisplay=reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
            if(!getPlatformDisplay)throw std::runtime_error("EGL platform display extension is unavailable");
            egl=getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA,EGL_DEFAULT_DISPLAY,nullptr);
            if(egl==EGL_NO_DISPLAY||!eglInitialize(egl,nullptr,nullptr))throw std::runtime_error("Cannot initialize offscreen EGL");
            if(!eglBindAPI(EGL_OPENGL_API))throw std::runtime_error("EGL desktop OpenGL is unavailable");
            EGLint attrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
            EGLConfig config;EGLint count;
            if(!eglChooseConfig(egl,attrs,&config,1,&count)||!count)throw std::runtime_error("No EGL pbuffer config");
            EGLint size[]={EGL_WIDTH,width,EGL_HEIGHT,height,EGL_NONE};surface=eglCreatePbufferSurface(egl,config,size);
            EGLint version[]={EGL_CONTEXT_MAJOR_VERSION,3,EGL_CONTEXT_MINOR_VERSION,3,EGL_NONE};eglContext=eglCreateContext(egl,config,EGL_NO_CONTEXT,version);
            if(surface==EGL_NO_SURFACE||eglContext==EGL_NO_CONTEXT||!eglMakeCurrent(egl,surface,surface,eglContext))throw std::runtime_error("Cannot create OpenGL 3.3 offscreen context");
            return;
        }
        display=XOpenDisplay(o.display.empty()?nullptr:o.display.c_str());
        if(!display)throw std::runtime_error("Cannot open X11 display; use --headless --snapshot FILE for an offscreen preview");
        active=this;previousErrorHandler=XSetErrorHandler(errorHandler);
        int glxEventBase=0;glXQueryExtension(display,&glxErrorBase,&glxEventBase);
        int screen=DefaultScreen(display);
        parent=o.window;
        if(!parent&&o.root){
            const char *id=std::getenv("XSCREENSAVER_WINDOW");
            parent=id?std::stoul(id,nullptr,0):RootWindow(display,screen);
        }
        embedded=parent!=0;
        if(embedded){
            saverStatus=XInternAtom(display,"_SCREENSAVER_STATUS",False);
            XWindowAttributes rootAttributes{};Window root=RootWindow(display,screen);
            if(XGetWindowAttributes(display,root,&rootAttributes))XSelectInput(display,root,rootAttributes.your_event_mask|PropertyChangeMask);
        }
        XVisualInfo *visual=nullptr;
        if(embedded){
            XWindowAttributes a{};if(!XGetWindowAttributes(display,parent,&a))throw std::runtime_error("Invalid host window");
            width=a.width;height=a.height;screen=XScreenNumberOfScreen(a.screen);
            visible=a.map_state==IsViewable;
            XSelectInput(display,parent,a.your_event_mask|StructureNotifyMask|ExposureMask|VisibilityChangeMask);
            XVisualInfo query{};query.visualid=XVisualIDFromVisual(a.visual);int count=0;
            visual=XGetVisualInfo(display,VisualIDMask,&query,&count);
            int useGL=0,rgba=0,buffers=0;
            if(visual&&glXGetConfig(display,visual,GLX_USE_GL,&useGL)==0&&useGL&&
               glXGetConfig(display,visual,GLX_RGBA,&rgba)==0&&rgba){
                glXGetConfig(display,visual,GLX_DOUBLEBUFFER,&buffers);
                doubleBuffered=buffers!=0;window=parent;
            }else {if(visual)XFree(visual);visual=nullptr;}
        }else parent=RootWindow(display,screen);
        if(o.fullscreen&&!embedded){width=DisplayWidth(display,screen);height=DisplayHeight(display,screen);}
        if(!visual){
            int visualAttrs[]={GLX_RGBA,GLX_DOUBLEBUFFER,GLX_RED_SIZE,8,GLX_GREEN_SIZE,8,GLX_BLUE_SIZE,8,None};
            visual=glXChooseVisual(display,screen,visualAttrs);
        }
        if(!visual)throw std::runtime_error("No double-buffered GLX visual");
        if(!window){
        colormap=XCreateColormap(display,parent,visual->visual,AllocNone);
        XSetWindowAttributes attr{};attr.colormap=colormap;attr.border_pixel=0;
        attr.event_mask=StructureNotifyMask|ExposureMask;
        if(!embedded)attr.event_mask|=KeyPressMask|ButtonPressMask|PointerMotionMask;
        attr.override_redirect=o.fullscreen&&!embedded;
        window=XCreateWindow(display,parent,0,0,width,height,0,visual->depth,InputOutput,visual->visual,CWColormap|CWBorderPixel|CWEventMask|CWOverrideRedirect,&attr);
        ownsWindow=true;
        }
        glx=glXCreateContext(display,visual,nullptr,True);XFree(visual);
        if(!glx||!glXMakeCurrent(display,window,glx))throw std::runtime_error("Cannot create GLX rendering context");
        glDrawBuffer(doubleBuffered?GL_BACK:GL_FRONT);
        if(ownsWindow){
        XStoreName(display,window,"Greek Coast");
        wmDelete=XInternAtom(display,"WM_DELETE_WINDOW",False);XSetWMProtocols(display,window,&wmDelete,1);
        if(embedded||o.fullscreen){
            Pixmap blank=XCreateBitmapFromData(display,window,"\0",1,1);XColor black{};
            Cursor cursor=XCreatePixmapCursor(display,blank,blank,&black,&black,0,0);XDefineCursor(display,window,cursor);XFreeCursor(display,cursor);XFreePixmap(display,blank);
        }
        XMapRaised(display,window);
        }
        XFlush(display);
        std::cerr<<"X11 render target: "<<(ownsWindow?"owned child":"host window")<<"\n";
        if(doubleBuffered&&epoxy_has_glx_extension(display,screen,"GLX_EXT_swap_control"))glXSwapIntervalEXT(display,window,embedded?0:1);
        else if(doubleBuffered&&embedded&&epoxy_has_glx_extension(display,screen,"GLX_MESA_swap_control"))glXSwapIntervalMESA(0);
    }
    ~Context(){
        if(display){
            XSync(display,False);
            glXMakeCurrent(display,None,nullptr);if(glx)glXDestroyContext(display,glx);
            if(window&&ownsWindow&&!drawableLost)XDestroyWindow(display,window);
            if(colormap)XFreeColormap(display,colormap);
            XCloseDisplay(display);XSetErrorHandler(previousErrorHandler);active=nullptr;
        }
        if(egl!=EGL_NO_DISPLAY){eglMakeCurrent(egl,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);if(eglContext!=EGL_NO_CONTEXT)eglDestroyContext(egl,eglContext);if(surface!=EGL_NO_SURFACE)eglDestroySurface(egl,surface);eglTerminate(egl);}
    }
    void events(bool fullscreen){
        if(!display)return;
        if(redrawRequested){redrawRequested=0;needsRestore=true;}
        while(XPending(display)){
            XEvent e;XNextEvent(display,&e);
            if(e.type==ConfigureNotify){
                width=e.xconfigure.width;height=e.xconfigure.height;
                if(embedded&&ownsWindow&&e.xconfigure.window==parent)XResizeWindow(display,window,std::max(width,1),std::max(height,1));
            }else if(e.type==MapNotify){
                visible=true;
                if(embedded&&ownsWindow&&e.xmap.window==parent)XMapRaised(display,window);
                restoreDrawable();
            }else if(e.type==UnmapNotify){visible=false;needsRestore=true;
            }else if(e.type==VisibilityNotify){
                bool wasVisible=visible;visible=e.xvisibility.state!=VisibilityFullyObscured;
                if(visible&&!wasVisible)restoreDrawable();
            }else if(e.type==Expose&&embedded&&ownsWindow&&e.xexpose.window==parent){
                XMapRaised(display,window);visible=true;
            }else if(e.type==Expose&&embedded&&!ownsWindow&&e.xexpose.count==0){
                visible=true;restoreDrawable();
            }else if(e.type==PropertyNotify&&e.xproperty.atom==saverStatus){needsRestore=true;
            }else if(e.type==DestroyNotify){running=0;window=0;drawableLost=true;}
            else if(e.type==ClientMessage&&Atom(e.xclient.data.l[0])==wmDelete)running=0;
            else if(!embedded&&(e.type==KeyPress||e.type==ButtonPress||(fullscreen&&e.type==MotionNotify)))running=0;
        }
        // Remapping an ancestor does not necessarily produce MapNotify on the
        // preview itself. Probe a hidden host so it resumes without reselection.
        if(embedded&&!drawableLost&&std::chrono::steady_clock::now()-lastVisibilityProbe>std::chrono::milliseconds(250)){
            lastVisibilityProbe=std::chrono::steady_clock::now();
            XWindowAttributes a{};
            if(XGetWindowAttributes(display,parent,&a)&&a.map_state==IsViewable){
                bool geometryChanged=width!=a.width||height!=a.height;
                bool returned=!visible;visible=true;width=a.width;height=a.height;
                if(ownsWindow&&(returned||needsRestore))XMapRaised(display,window);
                if(returned||geometryChanged||needsRestore)restoreDrawable();
            }
        }
    }
    bool canRender() const{return !display||(!drawableLost&&drawableReady&&window&&visible);}
    void swap(){if(display){if(window&&!drawableLost){if(doubleBuffered)glXSwapBuffers(display,window);else glFlush();}}else glFinish();}
};
}
int main(int argc,char **argv){
    QCoreApplication application(argc,argv);
    try{
        if(application.arguments().contains("--settings")){
            return QProcess::execute(QCoreApplication::applicationDirPath()+"/greek-coast-settings",{});
        }
        Options o=parse(argc,argv);
        configureTermination(o.root||o.window!=0,stop);
        if(!o.headless)std::signal(SIGCONT,continued);
        EnvironmentClock clock(o.cycle,QDateTime::currentMSecsSinceEpoch());
        if(o.skyInfo){
            const auto sky=sunAt(clock.timestamp(0,QDateTime::currentMSecsSinceEpoch()),o.cycle.latitude,o.cycle.longitude,o.cycle.heading);
            auto vector=[](const std::array<float,3>&v){return QJsonArray{v[0],v[1],v[2]};};
            QJsonObject info{{"settings",QJsonObject{{"mode",o.cycle.mode},{"speed",o.cycle.speed},{"activity_scale",o.detailScale},{"wind",o.wind},{"heading",o.cycle.heading},{"time",o.startTime}}},{"sun",vector(sky.direction)},{"moon",vector(sky.moonDirection)},
                {"sun_elevation",sky.elevation},{"moon_elevation",sky.moonElevation},{"moon_illumination",sky.moonIllumination}};
            std::cout<<QJsonDocument(info).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
        }
        Context context(o);
        Renderer renderer(o.grid,o.wind,o.assets,o.shaders);
        auto started=std::chrono::steady_clock::now(),next=started;
        int frames=0;
        while(running&&(!o.frames||frames<o.frames)){
            context.events(o.fullscreen);if(!running)break;
            if(!context.canRender()){std::this_thread::sleep_for(std::chrono::milliseconds(20));continue;}
            const double elapsed=o.headless?frames/double(o.fps):std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
            const double t=o.startTime+elapsed;
            const auto sun=sunAt(clock.timestamp(elapsed,QDateTime::currentMSecsSinceEpoch()),o.cycle.latitude,o.cycle.longitude,o.cycle.heading);
            renderer.draw(std::max(context.width,1),std::max(context.height,1),t,sun,o.drift,o.celestials,o.clouds,o.activity,o.detailScale,o.fishingBoat,o.taverna,o.fish);
            if(!o.snapshot.empty()&&(!o.frames||frames==o.frames-1))renderer.snapshot(o.snapshot,context.width,context.height);
            context.swap();++frames;
            if(!o.headless){next+=std::chrono::microseconds(1000000/o.fps);std::this_thread::sleep_until(next);if(next<std::chrono::steady_clock::now()-std::chrono::milliseconds(200))next=std::chrono::steady_clock::now();}
        }
        std::cerr<<"Rendered "<<frames<<" frames\n";
        return 0;
    }catch(const std::exception &e){std::cerr<<"Greek Coast: "<<e.what()<<'\n';return 1;}
}
