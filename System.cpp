#include "System.h"
#include "ui_System.h"
#include <qfile.h>
#include <qsettings.h>
#include "qmyglobal.h"
#include "Logger.h"

System::System(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::System)
{
    ui->setupUi(this);

    connect(ui->pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
    connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    //read

    QString dir=QApplication::applicationDirPath();

    QSettings settings(dir+"/system.ini", QSettings::IniFormat);
    float ConfThresh=settings.value("SystemConfig/ConfThresh").toFloat();
    int SingleRecordLoopSecond=settings.value("SystemConfig/SingleRecordLoopSecond").toInt();

    ui->sbHead->setValue(ConfThresh);
    ui->sbLoop->setValue(SingleRecordLoopSecond);

}

System::~System()
{
    delete ui;
}

void System::slotOK()
{
    QString confThresh=ui->sbHead->text();
    QString singleRecordLoopSecond=ui->sbLoop->text();

    QString dir=QApplication::applicationDirPath();

    QSettings settings(dir+"/system.ini", QSettings::IniFormat); // 当前目录的INI文件
    settings.beginGroup("SystemConfig");
    settings.setValue("ConfThresh", QVariant(confThresh));
    settings.setValue("SingleRecordLoopSecond", QVariant(singleRecordLoopSecond));
    settings.endGroup();

    close();

    g_ConfThresh=confThresh.toFloat();
    g_SingleRecordLoopSecond=singleRecordLoopSecond.toInt();

    LOG_DEBUG("write %f,%d.\n", g_ConfThresh, g_SingleRecordLoopSecond);
}

void System::slotCancel()
{
    close();
}
