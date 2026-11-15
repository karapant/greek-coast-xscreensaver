#include "termination.h"
#include <cerrno>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <sys/wait.h>
#include <unistd.h>
void deferredStop(int){}
int main(){
    int ready[2];if(pipe(ready))throw std::runtime_error("pipe failed");
    pid_t child=fork();if(child<0)throw std::runtime_error("fork failed");
    if(!child){
        close(ready[0]);configureTermination(true,deferredStop);
        char value=1;if(write(ready[1],&value,1)!=1)_exit(2);
        // Model a blocked rendering call: no frame-loop progress or cleanup.
        for(;;)pause();
    }
    close(ready[1]);char value;
    if(read(ready[0],&value,1)!=1)throw std::runtime_error("child did not start");
    close(ready[0]);kill(child,SIGTERM);
    int status=0;bool exited=false;
    for(int i=0;i<100;++i){
        if(waitpid(child,&status,WNOHANG)==child){exited=true;break;}
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    if(!exited){kill(child,SIGKILL);waitpid(child,&status,0);throw std::runtime_error("Host was left waiting for blocked preview");}
    if(!WIFSIGNALED(status)||WTERMSIG(status)!=SIGTERM)throw std::runtime_error("Preview did not honor immediate host termination");
    std::cout<<"Host termination exits blocked preview without a frame-loop dependency\n";
}
