// Allow the unmodified native XScreenSaver Settings application to read this
// user's extra configurator XML without writing into /usr/share.
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <cstdlib>
#include <string>
#include <filesystem>

namespace {
const char *configPath(const char *path,const char *mode){
    constexpr const char prefix[]="/usr/share/xscreensaver/config/";
    if(mode[0]=='r'&&std::strncmp(path,prefix,sizeof(prefix)-1)==0){
        const char *file=path+sizeof(prefix)-1;
        if(std::strcmp(file,"greek-coast.xml")==0||std::strcmp(file,"greek_coast.xml")==0)
            {
            if(const char *configured=std::getenv("GREEK_COAST_CONFIG"))return configured;
            static const std::string located=[](){
                Dl_info info{};dladdr(reinterpret_cast<void *>(&configPath),&info);
                auto directory=std::filesystem::path(info.dli_fname?info.dli_fname:".").parent_path();
                auto installed=(directory/GREEK_COAST_BRIDGE_DATA_RELATIVE/"greek-coast.xml").lexically_normal();
                if(std::filesystem::exists(installed))return installed.string();
                return (directory/"share/greek-coast/greek-coast.xml").string();
            }();
            return located.c_str();
        }
    }
    return path;
}
using Open=FILE *(*)(const char *,const char *);
// Keep this adapter in the Settings process only. It is already loaded here;
// remove just its own preload token before the GUI spawns previews/helpers.
__attribute__((constructor)) void scopeAdapterToSettings(){
    const char *preload=std::getenv("LD_PRELOAD");
    if(!preload)return;
    Dl_info info{};
    if(!dladdr(reinterpret_cast<void *>(&scopeAdapterToSettings),&info)||!info.dli_fname)return;
    std::string list(preload),kept;
    size_t at=0;
    while(at<list.size()){
        size_t end=list.find_first_of(": \t",at);
        if(end==std::string::npos)end=list.size();
        auto token=list.substr(at,end-at);
        if(!token.empty()&&token!=info.dli_fname){if(!kept.empty())kept+=':';kept+=token;}
        at=end+1;
    }
    if(kept.empty())unsetenv("LD_PRELOAD");else setenv("LD_PRELOAD",kept.c_str(),1);
}
}
extern "C" FILE *fopen(const char *path,const char *mode){
    static auto original=reinterpret_cast<Open>(dlsym(RTLD_NEXT,"fopen"));
    return original(configPath(path,mode),mode);
}
extern "C" FILE *fopen64(const char *path,const char *mode){
    static auto original=reinterpret_cast<Open>(dlsym(RTLD_NEXT,"fopen64"));
    return original(configPath(path,mode),mode);
}
