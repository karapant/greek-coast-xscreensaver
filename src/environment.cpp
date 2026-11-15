#include "environment.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTimeZone>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr double pi=3.14159265358979323846;
double radians(double x){return x*pi/180.;}
double smooth(double a,double b,double x){double t=std::clamp((x-a)/(b-a),0.,1.);return t*t*(3-2*t);}
double sinDeg(double d){return std::sin(radians(std::remainder(d,360.)));}
double cosDeg(double d){return std::cos(radians(std::remainder(d,360.)));}
std::array<float,3> viewDirection(double east,double up,double north,double heading){
    double h=radians(heading);
    return {float(east*std::cos(h)-north*std::sin(h)),float(up),float(-east*std::sin(h)-north*std::cos(h))};
}
void moonAt(SunState &s,qint64 utcMillis,double latitude,double longitude,double heading){
    // Low-precision lunar orbital elements and principal perturbations,
    // following Paul Schlyter. Epoch is 1999-12-31 00:00 UTC.
    const double jd=utcMillis/86400000.+2440587.5,d=jd-2451543.5;
    const double node=125.1228-.0529538083*d, inclination=5.1454;
    const double perigee=318.0634+.1643573223*d, anomaly=115.3654+13.0649929509*d;
    constexpr double eccentricity=.0549;
    const double M=radians(std::remainder(anomaly,360.));
    double E=M+eccentricity*std::sin(M)*(1+eccentricity*std::cos(M));
    for(int i=0;i<5;++i)E-=(E-eccentricity*std::sin(E)-M)/(1-eccentricity*std::cos(E));
    double x=60.2666*(std::cos(E)-eccentricity),y=60.2666*std::sqrt(1-eccentricity*eccentricity)*std::sin(E);
    double radius=std::hypot(x,y),v=std::atan2(y,x)*180/pi;
    double lon=std::atan2(sinDeg(node)*cosDeg(v+perigee)+cosDeg(node)*sinDeg(v+perigee)*cosDeg(inclination),
                          cosDeg(node)*cosDeg(v+perigee)-sinDeg(node)*sinDeg(v+perigee)*cosDeg(inclination))*180/pi;
    double lat=std::asin(sinDeg(v+perigee)*sinDeg(inclination))*180/pi;
    const double solarAnomaly=356.0470+.9856002585*d,solarPerigee=282.9404+.0000470935*d;
    const double Ls=solarAnomaly+solarPerigee,Lm=anomaly+perigee+node,D=Lm-Ls,F=Lm-node;
    lon+=-1.274*sinDeg(anomaly-2*D)+.658*sinDeg(2*D)-.186*sinDeg(solarAnomaly)
        -.059*sinDeg(2*anomaly-2*D)-.057*sinDeg(anomaly-2*D+solarAnomaly)
        +.053*sinDeg(anomaly+2*D)+.046*sinDeg(2*D-solarAnomaly)+.041*sinDeg(anomaly-solarAnomaly)
        -.035*sinDeg(D)-.031*sinDeg(anomaly+solarAnomaly)-.015*sinDeg(2*F-2*D)+.011*sinDeg(anomaly-4*D);
    lat+=-.173*sinDeg(F-2*D)-.055*sinDeg(anomaly-F-2*D)-.046*sinDeg(anomaly+F-2*D)
        +.033*sinDeg(F+2*D)+.017*sinDeg(2*anomaly+F);
    radius+=-.58*cosDeg(anomaly-2*D)-.46*cosDeg(2*D);
    x=radius*cosDeg(lon)*cosDeg(lat);y=radius*sinDeg(lon)*cosDeg(lat);
    double z=radius*sinDeg(lat),tilt=radians(23.4393-.0000003563*d);
    const double ye=y*std::cos(tilt)-z*std::sin(tilt),ze=y*std::sin(tilt)+z*std::cos(tilt);
    const double rightAscension=std::atan2(ye,x),declination=std::atan2(ze,std::hypot(x,ye));
    const double sidereal=radians(std::remainder(280.46061837+360.98564736629*(jd-2451545.)+longitude,360.));
    const double hourAngle=sidereal-rightAscension,phi=radians(latitude);
    double east=-std::cos(declination)*std::sin(hourAngle);
    double north=std::cos(phi)*std::sin(declination)-std::sin(phi)*std::cos(declination)*std::cos(hourAngle);
    double up=std::sin(phi)*std::sin(declination)+std::cos(phi)*std::cos(declination)*std::cos(hourAngle);
    const auto geocentric=viewDirection(east,up,north,heading);
    double separation=0;for(int i=0;i<3;++i)separation+=geocentric[i]*s.direction[i];
    s.moonIllumination=float(std::clamp((1-separation)*.5,0.,1.));
    // Topocentric parallax: subtract the observer (approximately one Earth radius).
    east*=radius;north*=radius;up=up*radius-1.;
    const double distance=std::sqrt(east*east+north*north+up*up);
    east/=distance;north/=distance;up/=distance;
    s.moonDirection=viewDirection(east,up,north,heading);
    s.moonElevation=float(std::asin(std::clamp(up,-1.,1.))*180/pi);
    s.moonAngularRadius=float(std::asin(.2725076/distance));
}
}
QString CycleSettings::path(){return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/greek-coast/settings.ini";}
CycleSettings CycleSettings::load(){
    QSettings ini(path(),QSettings::IniFormat);CycleSettings s;
    s.mode=ini.value("cycle/mode",s.mode).toString();
    s.speed=ini.value("cycle/speed",s.speed).toDouble();
    s.startNow=ini.value("cycle/start_now",s.startNow).toBool();
    s.startUtc=ini.value("cycle/start_utc",QDateTime::currentMSecsSinceEpoch()).toLongLong();
    s.latitude=ini.value("location/latitude",s.latitude).toDouble();
    s.longitude=ini.value("location/longitude",s.longitude).toDouble();
    s.heading=ini.value("location/heading",s.heading).toDouble();
    s.validate();return s;
}
void CycleSettings::save() const {
    validate();QDir().mkpath(QFileInfo(path()).absolutePath());
    QSettings ini(path(),QSettings::IniFormat);
    ini.setValue("cycle/mode",mode);ini.setValue("cycle/speed",speed);
    ini.setValue("cycle/start_now",startNow);ini.setValue("cycle/start_utc",startUtc);
    ini.setValue("location/latitude",latitude);ini.setValue("location/longitude",longitude);
    ini.setValue("location/heading",heading);
    ini.sync();if(ini.status()!=QSettings::NoError)throw std::runtime_error("Cannot save Greek Coast settings");
}
void CycleSettings::validate() const {
    if(mode!="real"&&mode!="accelerated")throw std::invalid_argument("Cycle mode must be real or accelerated");
    if(!std::isfinite(speed)||speed<.1||speed>1000)throw std::invalid_argument("Cycle speed must be between 0.1 and 1000 times real time");
    if(!std::isfinite(latitude)||latitude<34||latitude>42||!std::isfinite(longitude)||longitude<19||longitude>30)throw std::invalid_argument("Location must be within Greece (34–42 N, 19–30 E)");
    if(!std::isfinite(heading)||heading<0||heading>=360)throw std::invalid_argument("Viewing direction must be between 0 and 359.9 degrees from north");
    const auto date=QDateTime::fromMSecsSinceEpoch(startUtc,QTimeZone("UTC")).date();
    if(!date.isValid()||date.year()<1900||date.year()>2200)throw std::invalid_argument("Simulation date must be between 1900 and 2200");
}
EnvironmentClock::EnvironmentClock(CycleSettings s,qint64 now):settings(s),start(s.startNow?now:s.startUtc){settings.validate();}
qint64 EnvironmentClock::timestamp(double elapsed,qint64 actualNow) const {
    if(!std::isfinite(elapsed)||elapsed<0)throw std::invalid_argument("Elapsed cycle time must be finite and nonnegative");
    if(settings.mode=="real")return actualNow;
    return start+qint64(elapsed*settings.speed*1000.);
}
qint64 parseGreeceDate(const QString &text){
    auto date=QDateTime::fromString(text,Qt::ISODate);
    if(!date.isValid())throw std::invalid_argument("Date must use ISO format: YYYY-MM-DDTHH:MM:SS, optionally with a UTC offset");
    if(date.timeSpec()==Qt::LocalTime)date.setTimeZone(QTimeZone("Europe/Athens"));
    return date.toMSecsSinceEpoch();
}
SunState sunAt(qint64 utcMillis,double latitude,double longitude,double heading){
    // NOAA's approximate fractional-year solar model. UTC plus longitude
    // avoids a hard-coded DST rule; civil dates use IANA Europe/Athens.
    const auto date=QDateTime::fromMSecsSinceEpoch(utcMillis,QTimeZone("UTC"));
    const auto tod=date.time();
    double hour=tod.hour()+tod.minute()/60.+tod.second()/3600.+tod.msec()/3600000.;
    double gamma=2*pi/(date.date().isLeapYear(date.date().year())?366:365)*(date.date().dayOfYear()-1+(hour-12)/24);
    double eq=229.18*(.000075+.001868*std::cos(gamma)-.032077*std::sin(gamma)-.014615*std::cos(2*gamma)-.040849*std::sin(2*gamma));
    double decl=.006918-.399912*std::cos(gamma)+.070257*std::sin(gamma)-.006758*std::cos(2*gamma)+.000907*std::sin(2*gamma)-.002697*std::cos(3*gamma)+.00148*std::sin(3*gamma);
    double solarMinutes=std::fmod(hour*60+eq+4*longitude+1440.,1440.);
    double angle=radians(solarMinutes/4-180),lat=radians(latitude);
    double east=-std::cos(decl)*std::sin(angle);
    double north=std::cos(lat)*std::sin(decl)-std::sin(lat)*std::cos(decl)*std::cos(angle);
    double up=std::sin(lat)*std::sin(decl)+std::cos(lat)*std::cos(decl)*std::cos(angle);
    SunState s;s.direction=viewDirection(east,up,north,heading);
    s.elevation=float(std::asin(std::clamp(up,-1.,1.))*180/pi);
    s.daylight=float(smooth(-6,12,s.elevation));
    s.twilight=float(smooth(-10,-1,s.elevation)*(1-smooth(3,18,s.elevation)));
    s.lights=float(1-smooth(-5,1,s.elevation));
    const auto civil=QDateTime::fromMSecsSinceEpoch(utcMillis,QTimeZone("Europe/Athens")).time();
    s.localHour=civil.hour()+civil.minute()/60.f+civil.second()/3600.f;
    moonAt(s,utcMillis,latitude,longitude,heading);
    return s;
}
