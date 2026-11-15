#include "environment.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTimeZone>
#include <QVBoxLayout>
#include <cmath>

int main(int argc,char **argv){
    QApplication app(argc,argv);app.setApplicationName("Greek Coast Settings");
    try {
        CycleSettings s=CycleSettings::load();
        QDialog dialog;dialog.setWindowTitle("Greek Coast Settings");dialog.resize(740,390);
        QVBoxLayout layout(&dialog);
        QLabel heading("Day and night follow the sun over Greece.");layout.addWidget(&heading);
        QHBoxLayout columns;layout.addLayout(&columns);
        QGroupBox timeGroup("Time in Greece"),locationGroup("Location and view");
        columns.addWidget(&timeGroup,1);columns.addWidget(&locationGroup,1);
        QFormLayout form(&timeGroup),locationForm(&locationGroup);
        QComboBox mode;mode.addItem("Greece clock","real");mode.addItem("Accelerated","accelerated");mode.setCurrentIndex(s.mode=="real"?0:1);
        form.addRow("Time mode",&mode);
        QComboBox speed;
        for(double rate:{.1,.5,1.,2.,5.,10.,25.,50.,100.,200.,500.,1000.}){double seconds=86400./rate;QString length=seconds>=3600?QString("%1h %2m").arg(int(seconds/3600)).arg(int(std::fmod(seconds/60,60))):QString("%1m %2s").arg(int(seconds/60)).arg(QString::number(std::fmod(seconds,60),'f',1));speed.addItem(QString::number(rate)+"× — "+length+"/day",rate);}
        int saved=-1;for(int i=0;i<speed.count();++i)if(std::abs(speed.itemData(i).toDouble()-s.speed)<1e-6)saved=i;
        if(saved<0){speed.addItem(QString::number(s.speed)+" × (saved)",s.speed);saved=speed.count()-1;}
        speed.setCurrentIndex(saved);form.addRow("Cycle speed",&speed);
        QLabel duration;duration.setWordWrap(true);form.addRow("",&duration);
        QCheckBox now("Start from the current date and time in Greece");now.setChecked(s.startNow);form.addRow("",&now);
        const QTimeZone athens("Europe/Athens");
        QDateTimeEdit start;
#if QT_VERSION >= QT_VERSION_CHECK(6,7,0)
        start.setTimeZone(athens);
#else
        start.setTimeSpec(Qt::UTC);
#endif
        start.setCalendarPopup(true);start.setDisplayFormat("dd MMM yyyy  HH:mm");
        auto wall=QDateTime::fromMSecsSinceEpoch(s.startNow?QDateTime::currentMSecsSinceEpoch():s.startUtc,athens);
#if QT_VERSION >= QT_VERSION_CHECK(6,7,0)
        start.setDateTime(wall);
#else
        start.setDateTime(QDateTime(wall.date(),wall.time(),Qt::UTC));
#endif
        start.setMinimumDate(QDate(1900,1,1));start.setMaximumDate(QDate(2200,12,31));form.addRow("Starting Greece time",&start);
        QDoubleSpinBox latitude,longitude;latitude.setRange(34,42);longitude.setRange(19,30);latitude.setDecimals(3);longitude.setDecimals(3);
        latitude.setValue(s.latitude);longitude.setValue(s.longitude);latitude.setSuffix("° N");longitude.setSuffix("° E");
        locationForm.addRow("Latitude",&latitude);locationForm.addRow("Longitude",&longitude);
        QDoubleSpinBox view;view.setRange(0,359.9);view.setDecimals(1);view.setSuffix("° from north");view.setValue(s.heading);locationForm.addRow("View direction",&view);
        QLabel place("Default location: the Cyclades. Seasons, twilight, and Greek daylight saving time are handled automatically.");place.setWordWrap(true);layout.addWidget(&place);
        QLabel note("Save, then restart the screensaver preview to apply changes. Wave motion always runs at natural speed.");note.setWordWrap(true);layout.addWidget(&note);
        QDialogButtonBox buttons(QDialogButtonBox::Save|QDialogButtonBox::Cancel);auto *preview=buttons.addButton("Save and preview",QDialogButtonBox::ActionRole);layout.addWidget(&buttons);
        auto refresh=[&]{
            bool accelerated=mode.currentIndex()==1;speed.setEnabled(accelerated);now.setEnabled(accelerated);start.setEnabled(accelerated&&!now.isChecked());
            duration.setText("Acceleration changes day/night lighting. Water and scene activity stay at normal speed.");
        };
        QObject::connect(&mode,&QComboBox::currentIndexChanged,&dialog,[&]{refresh();});
        QObject::connect(&speed,&QComboBox::currentIndexChanged,&dialog,[&]{refresh();});
        QObject::connect(&now,&QCheckBox::toggled,&dialog,[&]{refresh();});refresh();
        auto save=[&]{s.mode=mode.currentData().toString();s.speed=speed.currentData().toDouble();s.startNow=now.isChecked();s.startUtc=QDateTime(start.date(),start.time(),athens).toMSecsSinceEpoch();s.latitude=latitude.value();s.longitude=longitude.value();s.heading=view.value();s.save();};
        QObject::connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        QObject::connect(&buttons,&QDialogButtonBox::accepted,&dialog,[&]{try{save();dialog.accept();}catch(const std::exception&e){QMessageBox::critical(&dialog,"Cannot save",e.what());}});
        QObject::connect(preview,&QPushButton::clicked,&dialog,[&]{try{save();QProcess::startDetached(QCoreApplication::applicationDirPath()+"/greek-coast",{});}catch(const std::exception&e){QMessageBox::critical(&dialog,"Cannot preview",e.what());}});
        if(app.arguments().contains("--self-test")){dialog.show();app.processEvents();return 0;}
        dialog.exec();return 0;
    }catch(const std::exception &e){QMessageBox::critical(nullptr,"Greek Coast Settings",e.what());return 1;}
}
