#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdlib>
std::string read(const char *path){
    FILE *f=std::fopen(path,"r");if(!f)throw std::runtime_error(std::string("Cannot open native configurator: ")+path);
    std::string s;char buf[4096];size_t n;while((n=std::fread(buf,1,sizeof(buf),f)))s.append(buf,n);std::fclose(f);return s;
}
int main(){
    const char *preload=std::getenv("LD_PRELOAD");
    if(preload&&std::string(preload).find("libgreek-coast-xs-config.so")!=std::string::npos)
        throw std::runtime_error("Settings adapter would leak into preview subprocesses");
    const auto xml=read("/usr/share/xscreensaver/config/greek-coast.xml");
    if(xml.find("arg-set=\"--speed 1000\"")==std::string::npos||xml.find("--cycle accelerated")==std::string::npos)
        throw std::runtime_error("Native configurator lacks cycle or 1000x speed control");
    if(read("/usr/share/xscreensaver/config/greek_coast.xml")!=xml)
        throw std::runtime_error("Native title alias does not resolve");
    if(read("/usr/share/xscreensaver/config/glschool.xml").find("glschool")==std::string::npos)
        throw std::runtime_error("Other system configurators were affected");
    std::cout<<"Native XScreenSaver XML lookup and 1000x controls verified\n";
}
