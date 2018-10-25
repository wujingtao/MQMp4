#ifndef WIDGET1_H
#define WIDGET1_H
/***************************************************************************
 * //配置/画图界面
 * ************************************************************************/
#include <QWidget>
#include <QTreeWidget>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QTimeEdit>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include "picturebox.h"
#include <QAction>
#include "qtpropertymanager.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"
#include "qtbuttonpropertybrowser.h"
#include "qtgroupboxpropertybrowser.h"
#include <QMap>
#include "ffmpegdecode.h"
#include "RuleWidget.h"

class QtVariantProperty;
class QtProperty;

class QtBrowserIndex;



class ConfigWidget :public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = 0);

    //窗口布局相关函数
    void initWindow();          //初始化窗口元素，分为四部分：1.左边的树形列表 2.中间上部的画图工具栏 3. 中间下部的图片展示窗口 4.右边的属性窗口
    void initCameraTree();      //初始化树形列表
    void initPainTool();        //初始化画图控件
    void initPictureMap();      //初始化图片展示窗口
    void initProperWindow();    //初始化规则属性窗口

    //事件类函数
    void mousePressEvent(QMouseEvent *e) ;



    //窗口布局类属性
    int leftWindowWidth;        //左边窗口的宽度
    int middleWindowWidth;      //中间窗口的宽度
    int rightWindowWidth;       //右边窗口的宽度
    int middleUpWindowheight;   //中间上面窗口高度
    int middleDownWindowheight; //中间下面窗口高度
    QRect location;   //本窗口区域
    int windowWidth;  //本窗口宽度
    int windowHeight; //本窗口高度
    int configheight; //配置控件的高度

    QPushButton * refreshTreeAndConfig; //刷新树列表和配置
    QTreeWidget * videoTreewidget; //摄像头列表
    QLabel * labloading;     //加载
    PictureBox * m_video;    //截图窗口
    QLabel * labAlarmTimes;   //报警的时间区间
    QLabel * labelstar;         //报警开始时间
    QLabel * labelstop;         //结束时间
    QLabel * labelAlarmNumDuringTime;   //单位时间内报警多少次则记录为报警
    QLineEdit * lineEditAlarmNumDuringTime;
    QTimeEdit * timeEdStart;  //开始时间
    QTimeEdit * timeEdStop;   //结束时间
    QListWidget * lvtAlarmTimes;//时间条目
    QPushButton * addlist;    //增加时间
    QPushButton * dellist;    //删除时间

    vector<QPoint>  points;
    //QMap<QString,QVector<QPoint>> pointslist;
    QVector<QPoint> pointDeleted;

    //QImage m_image;
    QImage imgtemp;
    double r;
    int offsetX, offsetY;
    QPushButton * buttonCancelDraw;    //新建规则可以开始画线
    QPushButton * buttonCreateRule;   //新建规则
    QPushButton * buttonRestLast;     //撤销上步
    QPushButton * buttonRedo;         //恢复上步
    QPushButton * buttonEndDraw;      //结束画线
    QString pointsKey;
    QString currentPointKey;
    //
    QPushButton * buttonConfirm;    //确认提交
    QPushButton *buttonDel; //删除
    QPushButton *buttonEdit;//编辑

    bool ishighlight;               //是否被选中
    //ObjectDetector  *detect;
    //Detector *detect;

    bool loadCameraRule(QImage &image);
    void getScaleAndoffsetXY();    
    void getCameraInfo();
    void insertPoint(QVector<QPoint> points);

signals:
    void itemClicked(QString item);
    void callLogin();
public slots:
    void loadCameraTree();      //加载树形列表的元素
    void slotOpenMp4();

    void on_videoTreewidget_doubleClicked(const QModelIndex &index);

    void slot_createNewRule();
    void slot_cancelDraw();
    void slot_undolastline();
    void slot_redolastline();
    void slot_enddraw();

    void slot_addtime();
    void slot_deltime();    

    void slotChooseRule(const QString &ipcID, const QString &ruleID);
    void slotSave();                //保存按钮
    void slotEdit();
    void slotDel();

private:
    void configValue(const QString &ipcid, const QString &ruleid);

private:
    QAction *deleteAction;
    void addProperty(QtVariantProperty *property, const QString &id);
//    void setPropertEnable(QString key1,QString key2,bool alertEnable,bool alarmEnable,int alertMinute,int alarmMinute);
    void setPropertEnable(QString keyAlert,QString keyAlarm,QString keyAlert_threshold,QString keyAlarm_threshold,bool alertEnable,bool alarmEnable,int alertMinute,int alarmMinute,float alertThresholdValue,float alarmThresholdValue);
    void updateExpandState();
    bool saveRule(const QString &ipcid, const QString &ruleid, bool isUpdate=false);         //保存规则

private:
    void setRuleWidgetVisible(bool visible);
    void setRuleButtonVisible(bool visible);


private:
    /*
    class QtVariantPropertyManager *variantManager;
    class QtTreePropertyBrowser *propertyEditor;

    QMap<QtProperty *, QString> propertyToId;
    QMap<QString, QtVariantProperty *> idToProperty;
    QMap<QString, bool> idToExpanded;
    */

    QString m_ipcid;   //摄像头ID
    QString m_ruleid;

    //ffmpegDecode *m_pFfm;
    bool m_isGetImage;

    RuleWidget *m_ruleWidget;

    bool m_isEdit;
};

#endif // WIDGET1_H
