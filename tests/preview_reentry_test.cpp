#include <epoxy/glx.h>
#include <X11/Xutil.h>
#include <chrono>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <sys/wait.h>
#include <unistd.h>

void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char **argv){
    if(argc!=2)return 1;
    Display *display=XOpenDisplay(nullptr);
    if(!display){std::cout<<"Skipped: desktop X11 display is unavailable\n";return 77;}
    int attrs[]={GLX_RGBA,GLX_DOUBLEBUFFER,GLX_RED_SIZE,8,GLX_GREEN_SIZE,8,GLX_BLUE_SIZE,8,None};
    XVisualInfo *visual=glXChooseVisual(display,DefaultScreen(display),attrs);
    if(!visual){XCloseDisplay(display);return 77;}
    Window root=RootWindow(display,visual->screen);
    Colormap cmap=XCreateColormap(display,root,visual->visual,AllocNone);
    XSetWindowAttributes a{};a.colormap=cmap;a.background_pixel=0;a.border_pixel=0;
    Window frame=XCreateSimpleWindow(display,root,0,0,640,360,0,0,0);XMapWindow(display,frame);
    Window host=XCreateWindow(display,frame,0,0,640,360,0,visual->depth,InputOutput,visual->visual,CWColormap|CWBackPixel|CWBorderPixel,&a);
    XFree(visual);XMapWindow(display,host);XSync(display,False);
    pid_t child=-1;
    try{
        std::string id=std::to_string(host);child=fork();check(child>=0,"cannot fork preview");
        if(!child){execl(argv[1],argv[1],"--window-id",id.c_str(),"--fps","30","--date","2026-06-21T13:30:00+03:00",nullptr);_exit(2);}
        auto painted=[&]{
            for(int attempt=0;attempt<30;++attempt){
                int status=0;check(waitpid(child,&status,WNOHANG)!=child,"preview exited during host reentry");
                XImage *image=XGetImage(display,host,300,160,40,40,AllPlanes,ZPixmap);
                bool nonblack=false;
                if(image){for(int y=0;y<40;y+=4)for(int x=0;x<40;x+=4)nonblack|=XGetPixel(image,x,y)!=0;XDestroyImage(image);}
                if(nonblack)return true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            return false;
        };
        check(painted(),"preview did not initially render");
        Window rr,pp,*children=nullptr;unsigned count=0;
        XQueryTree(display,host,&rr,&pp,&children,&count);if(children)XFree(children);
        check(count==0,"compatible preview should render directly in host, without a child window");
        for(int cycle=0;cycle<3;++cycle){
            XUnmapWindow(display,host);XSync(display,False);std::this_thread::sleep_for(std::chrono::milliseconds(100));
            XMapWindow(display,host);XClearArea(display,host,0,0,0,0,True);XSync(display,False);
            check(painted(),"preview did not resume after fullscreen/preview transition");
        }
        // A fullscreen saver can obscure the whole settings window without
        // unmapping the embedded drawing area itself.
        Window cover=XCreateSimpleWindow(display,root,0,0,1024,768,0,0,0);
        XMapRaised(display,cover);XSync(display,False);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        kill(child,SIGSTOP);int stopped=0;check(waitpid(child,&stopped,WUNTRACED)==child&&WIFSTOPPED(stopped),"preview exited while fullscreen was obscuring the host");
        XUnmapWindow(display,cover);XClearArea(display,host,0,0,0,0,True);XSync(display,False);
        kill(child,SIGCONT);check(painted(),"preview did not resume after fullscreen obscuring and SIGCONT");
        XDestroyWindow(display,cover);
        // Ancestor remapping may send no MapNotify to the drawing-area child.
        XUnmapWindow(display,frame);XSync(display,False);std::this_thread::sleep_for(std::chrono::milliseconds(150));
        XMapWindow(display,frame);XClearArea(display,host,0,0,0,0,True);XSync(display,False);
        check(painted(),"preview did not recover after ancestor remapping");
        XResizeWindow(display,frame,800,450);XResizeWindow(display,host,800,450);XSync(display,False);
        XClearArea(display,host,0,0,0,0,True);check(painted(),"preview did not recover after resized backing buffers");
        XDestroyWindow(display,host);host=0;XSync(display,False);
        int status=0;bool done=false;
        for(int i=0;i<200;++i){if(waitpid(child,&status,WNOHANG)==child){done=true;child=-1;break;}std::this_thread::sleep_for(std::chrono::milliseconds(10));}
        check(done,"preview did not exit when host was destroyed");
        check(WIFEXITED(status)&&WEXITSTATUS(status)==0,"host destruction produced an abnormal preview exit");
        XDestroyWindow(display,frame);XFreeColormap(display,cmap);XCloseDisplay(display);
        std::cout<<"Direct host drawing, three preview reentries and host destruction passed\n";return 0;
    }catch(const std::exception &e){
        if(child>0){kill(child,SIGKILL);waitpid(child,nullptr,0);}if(host)XDestroyWindow(display,host);
        XDestroyWindow(display,frame);XFreeColormap(display,cmap);XCloseDisplay(display);std::cerr<<e.what()<<'\n';return 1;
    }
}
