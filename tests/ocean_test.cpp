#include "ocean.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
int main(){
    Ocean a(64,5,1975),b(64,5,1975);
    const auto start=a.height();
    check(start==b.height(),"deterministic wave seed");
    a.update(1.0);double energy=0,change=0,mean=0;
    for(size_t i=0;i<start.size();++i){
        float h=a.height()[i];check(std::isfinite(h),"finite wave displacement");
        energy+=h*h;mean+=h;change+=std::abs(h-start[i]);
    }
    check(energy>1&&energy<1000,"bounded nonzero wave energy");
    check(std::abs(mean/start.size())<.001,"zero mean sea level");
    check(change>1,"wave field evolves with time");
    a.update(1000000);for(float h:a.height())check(std::isfinite(h),"long-running simulation remains finite");
    bool rejected=false;try{Ocean bad(63);}catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"invalid grid rejected");
    std::cout<<"Spectral wave behavior passed\n";
}
