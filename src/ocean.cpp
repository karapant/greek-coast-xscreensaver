#include "ocean.h"
#include "pocketfft_hdronly.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

Ocean::Ocean(int size,float wind,uint32_t seed):n(size) {
    if(n<16||n>512||(n&(n-1)))throw std::invalid_argument("Ocean grid must be a power of two between 16 and 512");
    if(!std::isfinite(wind)||wind<1||wind>20)throw std::invalid_argument("Wind must be between 1 and 20 m/s");
    initial.resize(n*n);evolved.resize(n*n);spatial.resize(n*n);
    omega.resize(n*n);heights.resize(n*n);
    std::mt19937 random(seed);std::normal_distribution<float> gaussian(0,1);
    const float pi=3.14159265358979323846f, L=wind*wind/9.81f;
    for(int z=0;z<n;++z)for(int x=0;x<n;++x) {
        const int i=z*n+x;
        const float kx=2*pi*(x<=n/2?x:x-n)/length();
        const float kz=2*pi*(z<=n/2?z:z-n)/length();
        const float k2=kx*kx+kz*kz, k=std::sqrt(k2);
        if(k<.00001f){initial[i]=0;omega[i]=0;continue;}
        const float direction=(kx*.8f+kz*.6f)/k;
        // Directional Phillips spectrum, suppressed at tiny wavelengths.
        const float spectrum=std::exp(-1/(k2*L*L))*direction*direction*
            std::exp(-k2*.06f)/(k2*k2);
        initial[i]=std::complex<float>(gaussian(random),gaussian(random))*std::sqrt(spectrum*.5f);
        omega[i]=std::sqrt(9.81f*k*std::tanh(k*18.f));
    }
    update(0);
    double energy=0;for(float h:heights)energy+=h*h;
    const float rms=float(std::sqrt(energy/heights.size()));
    scale=(.035f+.011f*wind)/std::max(rms,.000001f);
    update(0);
}
void Ocean::update(double seconds) {
    if(!std::isfinite(seconds))throw std::invalid_argument("Ocean time must be finite");
    for(int z=0;z<n;++z)for(int x=0;x<n;++x){
        const int i=z*n+x, opposite=((n-z)%n)*n+(n-x)%n;
        const float phase=float(std::remainder(double(omega[i])*seconds,6.283185307179586));
        const std::complex<float> rotation(std::cos(phase),std::sin(phase));
        evolved[i]=initial[i]*rotation+std::conj(initial[opposite])*std::conj(rotation);
    }
    const pocketfft::shape_t shape{size_t(n),size_t(n)},axes{0,1};
    const pocketfft::stride_t strides{ptrdiff_t(n*sizeof(std::complex<float>)),ptrdiff_t(sizeof(std::complex<float>))};
    pocketfft::c2c(shape,strides,strides,axes,pocketfft::BACKWARD,evolved.data(),spatial.data(),1.f/(n*n),1);
    for(int i=0;i<n*n;++i)heights[i]=spatial[i].real()*scale;
}
float Ocean::sampleHeight(float x,float z) const {
    auto wrap=[&](int i){return (i%n+n)%n;};
    float u=x/length()*n,v=z/length()*n;int a=int(std::floor(u)),b=int(std::floor(v));
    float fx=u-a,fz=v-b;
    float p=heights[wrap(b)*n+wrap(a)]*(1-fx)+heights[wrap(b)*n+wrap(a+1)]*fx;
    float q=heights[wrap(b+1)*n+wrap(a)]*(1-fx)+heights[wrap(b+1)*n+wrap(a+1)]*fx;
    return p*(1-fz)+q*fz;
}
