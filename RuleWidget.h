#ifndef RULEWIDGET_H
#define RULEWIDGET_H

#include <QWidget>

namespace Ui {
class RuleWidget;
}

class RuleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RuleWidget(QWidget *parent = 0);
    ~RuleWidget();

    void init();
    void setIpcID(const QString &ipcid);

public:
    void setLineType(int type);
    int getLineType();

    void setAlarmType(int type);
    int getAlarmType();

    void setRuleName(QString name);
    QString getRuleName();

    void setPersonMin(int person);
    int getPersonMin();

    void setPersonMax(int person);
    int getPersonMax();

    void setAlarmLoop(int loop);
    int getAlarmLoop();

    void setAlarmDelay(int delay);
    int getAlarmDelay();

    void setAlarmPercent(int percent);
    int getAlarmPercent();

    void setTimes(const QStringList &times);
    QStringList getTimes();

public slots:
    void slotAddTime();
    void slotDelTime();
    void slotRuleName(const QString &alarmTypeName);

private:
    Ui::RuleWidget *ui;

    QString m_ipcid;
};

#endif // RULEWIDGET_H
