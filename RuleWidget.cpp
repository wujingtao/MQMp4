#include "RuleWidget.h"
#include "ui_rulewidget.h"

#include "qmyglobal.h"

#include <qmessagebox.h>

RuleWidget::RuleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RuleWidget)
{
    ui->setupUi(this);

    m_ipcid="";

    connect(ui->pbAdd, SIGNAL(clicked()), this, SLOT(slotAddTime()));
    connect(ui->pbDel, SIGNAL(clicked()), this, SLOT(slotDelTime()));
    connect(ui->cbAlarmType, SIGNAL(currentTextChanged(QString)), this, SLOT(slotRuleName(QString)));
}

RuleWidget::~RuleWidget()
{
    delete ui;
}

void RuleWidget::init()
{
    //1.
    QStringList enumNames;
    enumNames.insert(0,"请选择");

    QMap<int,mDictionary>::const_iterator iter;
    for(iter=map_AlarmType.constBegin();iter!=map_AlarmType.constEnd(); ++iter)
    {
        mDictionary dictionary=iter.value();
        enumNames.insert(dictionary.code.trimmed().toInt(), dictionary.name);
    }
    ui->cbAlarmType->addItems(enumNames);

    //2.
    enumNames.clear();
    enumNames.insert(0,"请选择");

    for(iter=map_LineType.constBegin();iter!=map_LineType.constEnd(); ++iter)
    {
        mDictionary dictionary=iter.value();
        enumNames.insert(dictionary.code.trimmed().toInt(), dictionary.name);
    }
    ui->cbLineType->addItems(enumNames);

    //
    ui->teStart->setTime(QTime(0, 0, 0));
    ui->teEnd->setTime(QTime(23, 59, 59));
}

void RuleWidget::setIpcID(const QString &ipcID)
{
    m_ipcid=ipcID;
}

void RuleWidget::setLineType(int type)
{
    ui->cbLineType->setCurrentIndex(type);
}
int RuleWidget::getLineType()
{
    return ui->cbLineType->currentIndex();
}

void RuleWidget::setAlarmType(int type)
{
    ui->cbAlarmType->setCurrentIndex(type);
}

int RuleWidget::getAlarmType()
{
    return ui->cbAlarmType->currentIndex();
}

void RuleWidget::setRuleName(QString name)
{
    ui->leName->setText(name);
}

QString RuleWidget::getRuleName()
{
    return ui->leName->text();
}

void RuleWidget::setPersonMin(int person)
{
    ui->sbPersonMin->setValue(person);
}

int RuleWidget::getPersonMin()
{
    return ui->sbPersonMin->value();
}

void RuleWidget::setPersonMax(int person)
{
    ui->sbPersonMax->setValue(person);
}

int RuleWidget::getPersonMax()
{
    return ui->sbPersonMax->value();
}


void RuleWidget::setAlarmLoop(int loop)
{
    ui->sbAlarmLoop->setValue(loop);
}

int RuleWidget::getAlarmLoop()
{
    return ui->sbAlarmLoop->value();
}

void RuleWidget::setAlarmDelay(int delay)
{
    ui->sbDelay->setValue(delay);
}

int RuleWidget::getAlarmDelay()
{
    return ui->sbDelay->value();
}

void RuleWidget::setAlarmPercent(int percent)
{
    ui->sbPercent->setValue(percent);
}

int RuleWidget::getAlarmPercent()
{
    return ui->sbPercent->value();
}

void RuleWidget::setTimes(const QStringList &times)
{
    ui->lwTimes->clear();
    ui->lwTimes->addItems(times);
}

QStringList RuleWidget::getTimes()
{
    QStringList strlist;

    for(int i=0;i<ui->lwTimes->count();i++)
        strlist.append(ui->lwTimes->item(i)->text());

    return strlist;
}

void RuleWidget::slotAddTime()
{
    QTime start = ui->teStart->time();
    QTime end = ui->teEnd->time();
    int allowCount = ui->sbAllowCount->value();

    if(start > end)
    {
        QMessageBox::about(NULL, "温馨提示", "时间选择不正确，结束时间需大于开始时间。");
        return;
    }

    bool iscontains=false;
    for(int i=0;i<ui->lwTimes->count();i++)
    {
        QStringList timelist=ui->lwTimes->item(i)->text().split("-");
        QTime startTime=QTime::fromString(timelist[0],"hh:mm:ss");
        QTime stopTime=QTime::fromString(timelist[1],"hh:mm:ss");
        //时间段a->b ,c->d  ,(a-d)*(b-c)<0 则有重复
        double a=start.secsTo(stopTime);
        double c=end.secsTo(startTime);
        if (a*c<0)
        {
            iscontains=true;
        }
    }

    if(!iscontains)
    {
        QString time=QString("%1-%2-%3").arg(start.toString("hh:mm:ss")).arg(end.toString("hh:mm:ss")).arg(allowCount+1);
        ui->lwTimes->addItem(time);
    }
    else
        QMessageBox::about(NULL, "温馨提示", "您新增的时间段与之前时间段有重复，请更改！");
}

void RuleWidget::slotDelTime()
{
    int row= ui->lwTimes->currentRow();
    ui->lwTimes->takeItem(row);
}

void RuleWidget::slotRuleName(const QString &alarmTypeName)
{
    if(m_ipcid!="")
    {
        QString areaName=map_Area[0].AreaName;
        QString alarmName = areaName + m_ipcid + alarmTypeName;

        ui->leName->setText(alarmName);
    }
}
//
