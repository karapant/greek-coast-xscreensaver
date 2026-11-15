#include "environment.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTimeZone>
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char **argv){
    QTemporaryDir config;qputenv("XDG_CONFIG_HOME",config.path().toUtf8());QCoreApplication app(argc,argv);
    const auto summer=parseGreeceDate("2026-06-21T13:30:00+03:00");
    const auto winter=parseGreeceDate("2026-12-21T12:30:00+02:00");
    const auto midnight=parseGreeceDate("2026-06-21T00:00:00+03:00");
    auto day=sunAt(summer,37.1,25.1),night=sunAt(midnight,37.1,25.1),cold=sunAt(winter,37.1,25.1);
    check(day.elevation>70&&day.daylight>.99&&day.lights==0,"summer noon is bright with lamps off");
    check(cold.elevation>25&&cold.elevation<35,"winter solar elevation differs from summer");
    check(night.elevation< -20&&night.daylight==0&&night.lights>.99,"night is dark with lamps on");
    check(parseGreeceDate("2026-06-21T13:30:00")==summer,"Athens summer daylight saving time");
    check(parseGreeceDate("2026-12-21T12:30:00")==winter,"Athens winter standard time");
    int summerHours=0,winterHours=0;
    for(int h=0;h<24;++h){
        if(sunAt(parseGreeceDate(QString("2026-06-21T%1:00:00+03:00").arg(h,2,10,QChar('0'))),37.1,25.1).elevation>0)++summerHours;
        if(sunAt(parseGreeceDate(QString("2026-12-21T%1:00:00+02:00").arg(h,2,10,QChar('0'))),37.1,25.1).elevation>0)++winterHours;
    }
    check(summerHours>winterHours+3,"seasonal day length follows Greece date");
    const auto newMoon=sunAt(parseGreeceDate("2024-04-08T18:21:00Z"),37.1,25.1);
    const auto fullMoon=sunAt(parseGreeceDate("2024-03-25T07:00:00Z"),37.1,25.1);
    check(newMoon.moonIllumination<.01,"new moon has an almost entirely dark visible hemisphere");
    check(fullMoon.moonIllumination>.99,"full moon has a lit visible hemisphere");
    const auto west=sunAt(parseGreeceDate("2026-06-21T17:10:00Z"),37.1,25.1,270);
    const auto east=sunAt(parseGreeceDate("2026-06-21T17:10:00Z"),37.1,25.1,90);
    check(west.direction[2]<0&&east.direction[2]>0,"sunset appears in west view and is behind east view");
    check(std::abs(west.direction[1]-east.direction[1])<.00001,"view heading does not change solar elevation");
    double norm=0;for(float v:west.moonDirection)norm+=v*v;
    check(std::abs(norm-1)<.0001,"topocentric moon direction is normalized");
    check(west.moonAngularRadius>.004&&west.moonAngularRadius<.0055,"moon keeps realistic angular size");
    CycleSettings s;s.startUtc=summer;s.startNow=false;s.mode="accelerated";
    for(double rate:{.1,1.,50.,200.,1000.}){s.speed=rate;EnvironmentClock clock(s,midnight);check(std::abs(clock.timestamp(120,midnight)-(summer+qint64(120000*rate)))<=1,"exact acceleration multiplier");}
    s.mode="real";EnvironmentClock real(s,summer);check(real.timestamp(10000,midnight)==midnight,"real mode tracks actual wall clock");
    s.speed=50;s.save();auto loaded=CycleSettings::load();check(loaded.speed==50&&loaded.startUtc==summer,"settings round trip");
    bool rejected=false;try{s.speed=1000.1;s.validate();}catch(const std::invalid_argument&){rejected=true;}check(rejected,"invalid acceleration rejected");
    std::cout<<"Solar position, seasons, Greek timezone, cycle rates and settings passed\n";
}
