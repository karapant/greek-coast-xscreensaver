#pragma once
#include <QDateTime>
#include <QString>
#include <array>

struct CycleSettings {
    QString mode="real";
    double speed=1.;
    bool startNow=true;
    qint64 startUtc=0;
    double latitude=37.1,longitude=25.1,heading=270.;
    static QString path();
    static CycleSettings load();
    void save() const;
    void validate() const;
};
struct SunState {
    std::array<float,3> direction{};
    std::array<float,3> moonDirection{};
    float moonElevation=0,moonIllumination=0,moonAngularRadius=.0045f;
    float elevation=0,daylight=0,twilight=0,lights=0;
    float localHour=12;
};
class EnvironmentClock {
public:
    EnvironmentClock(CycleSettings settings,qint64 realStart);
    qint64 timestamp(double elapsed,qint64 actualNow) const;
private:
    CycleSettings settings;
    qint64 start;
};
SunState sunAt(qint64 utcMillis,double latitude,double longitude,double heading=270.);
qint64 parseGreeceDate(const QString &text);
