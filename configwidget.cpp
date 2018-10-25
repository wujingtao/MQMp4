      #include "configwidget.h"
#include "qjudgeposition.h"
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include "qmyglobal.h"
#include <QMessageBox>
#include "qdrawline.h"
#include "qtvariantproperty.h"
#include <QPixmap>
#include "qttreepropertybrowser.h"
#include <QDockWidget>
#include <QDebug>
#include <QDir>
#include "datatranslate.h"
#include "qffmpeg_rtsp_image.h"
//#include "windows.h"
#include <QApplication>
#include <QMovie>
#include"videowidget.h"
using namespace cv;
using namespace std;

#include <QFileDialog>
#include "AlarmManager.h"
#include "GlobalData.h"
#include "RuleWidget.h"


ConfigWidget::ConfigWidget(QWidget *parent) :
    QWidget(parent)
{
    initWindow();

    ishighlight=false;

    pointsKey="0";
    currentPointKey="0";
    buttonCreateRule->setEnabled(false);
    buttonCancelDraw->setEnabled(false);
    buttonRestLast->setEnabled(false);
    buttonRedo->setEnabled(false);
    buttonEndDraw->setEnabled(false);

    m_isGetImage=false;
    m_isEdit=false;
}

//初始化窗口元素
void ConfigWidget::initWindow()
{
    setMaximumSize(screenX,screenY*0.91);
    resize(screenX,screenY*0.91);

    location=this->geometry();
    leftWindowWidth=location.width()*0.16;
    middleWindowWidth=location.width()*0.64;
    rightWindowWidth=location.width()*0.2;
    middleUpWindowheight=middleWindowWidth/20*1;
    middleDownWindowheight=middleWindowWidth/20*18;
    windowWidth=location.width();
    windowHeight=location.height();
    initCameraTree();
    initPainTool();
    initPictureMap();
    initProperWindow();
}

//初始化树形列表
void ConfigWidget::initCameraTree()
{

    refreshTreeAndConfig=new QPushButton(this);
    refreshTreeAndConfig->setFixedSize(screenX*0.09143,screenY*0.04);
    refreshTreeAndConfig->setText("打开MP4");
    refreshTreeAndConfig->setObjectName("refreshTreeAndConfig");
    refreshTreeAndConfig->setStyleSheet("#refreshTreeAndConfig{border-image: url(:/image/button2.png);}"
                               "#refreshTreeAndConfig:hover{border-image: url(:/image/button.png);}"
                               "#refreshTreeAndConfig:pressed{border-image: url(:/image/button3.png);}");
    refreshTreeAndConfig->setGeometry(5,5,screenX*0.08,screenY*0.04);
    //--------------------------------------------摄像头树
    videoTreewidget=new QTreeWidget(this);
    videoTreewidget->setFixedSize(screenX*0.157,screenY*0.91);
    videoTreewidget->setWindowTitle("摄像头列表");
    videoTreewidget->header()->setVisible(false);
    videoTreewidget->setStyleSheet("background:transparent; font-size:17px; color: white;");
    videoTreewidget->setWindowFlags(Qt::FramelessWindowHint);
    QHeaderView *head=videoTreewidget->header();
    head->setSectionResizeMode(QHeaderView::ResizeToContents );
    videoTreewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    videoTreewidget->setGeometry(0,screenY*0.04,screenX*0.157,screenY*0.91);
    //connect(refreshTreeAndConfig,SIGNAL(clicked()),this,SLOT(loadCameraTree()));
    connect(refreshTreeAndConfig,SIGNAL(clicked()),this,SLOT(slotOpenMp4()));
    connect(videoTreewidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(on_videoTreewidget_doubleClicked(QModelIndex)));
}

//初始化画图控件
void ConfigWidget::initPainTool()
{
    buttonCreateRule=new QPushButton(this);
    buttonCreateRule->setFixedSize(screenX*0.09143,screenY*0.04);
    buttonCreateRule->setText("新建规则");
    buttonCreateRule->setGeometry(leftWindowWidth,windowHeight*0.01,screenX*0.09143,screenY*0.04);
    /*
    buttonCreateRule->setObjectName("buttonCreateRule");
    buttonCreateRule->setStyleSheet("#buttonCreateRule{border-image: url(:/image/button.png);font-size:17px; color: black;}"
                               "#buttonCreateRule:hover{border-image: url(:/image/button2.png);}"
                               "#buttonCreateRule:pressed{border-image: url(:/image/button3.png);}");*/

    buttonCancelDraw=new QPushButton(this);
    buttonCancelDraw->setFixedSize(screenX*0.09143,screenY*0.04);
    buttonCancelDraw->setText("取消规则");
    buttonCancelDraw->setGeometry(leftWindowWidth+buttonCreateRule->width(),windowHeight*0.01,screenX*0.09143,screenY*0.04);
    buttonCancelDraw->setObjectName("buttonStartDraw");
    /*
    buttonCancelDraw->setStyleSheet("#buttonStartDraw{border-image: url(:/image/button.png);font-size:17px; color: black;}"
                               "#buttonStartDraw:hover{border-image: url(:/image/button2.png);}"
                               "#buttonStartDraw:pressed{border-image: url(:/image/button3.png);}");*/


    buttonRestLast=new QPushButton(this);
    buttonRestLast->setFixedSize(screenX*0.09143,screenY*0.04);
    buttonRestLast->setText("上一步");
    buttonRestLast->setGeometry(leftWindowWidth+buttonCreateRule->width()+buttonCancelDraw->width(),windowHeight*0.01,screenX*0.09143,screenY*0.04);
    /*buttonRestLast->setObjectName("buttonRestLast");
    buttonRestLast->setStyleSheet("#buttonRestLast{border-image: url(:/image/button.png);font-size:17px; color: black;}"
                               "#buttonRestLast:hover{border-image: url(:/image/button2.png);}"
                               "#buttonRestLast:pressed{border-image: url(:/image/button3.png);}");*/

    buttonRedo=new QPushButton(this);
    buttonRedo->setFixedSize(screenX*0.09143,screenY*0.04);
    buttonRedo->setText("下一步");
    buttonRedo->setGeometry(leftWindowWidth+buttonCreateRule->width()+buttonCancelDraw->width()+buttonRestLast->width(),windowHeight*0.01,screenX*0.09143,screenY*0.04);
    /*buttonRedo->setObjectName("buttonRedo");
    buttonRedo->setStyleSheet("#buttonRedo{border-image: url(:/image/button.png);font-size:17px; color: black;}"
                               "#buttonRedo:hover{border-image: url(:/image/button2.png);}"
                               "#buttonRedo:pressed{border-image: url(:/image/button3.png);}");*/

    buttonEndDraw=new QPushButton(this);
    buttonEndDraw->setFixedSize(screenX*0.09143,screenY*0.04);
    buttonEndDraw->setText("保存画线");
    buttonEndDraw->setGeometry(leftWindowWidth+buttonCreateRule->width()+buttonCancelDraw->width()+buttonRestLast->width()+buttonRedo->width(),windowHeight*0.01,screenX*0.09143,screenY*0.04);
    /*buttonEndDraw->setObjectName("buttonEndDraw");
    buttonEndDraw->setStyleSheet("#buttonEndDraw{border-image: url(:/image/button.png);font-size:17px; color: black;}"
                               "#buttonEndDraw:hover{border-image: url(:/image/button2.png);}"
                               "#buttonEndDraw:pressed{border-image: url(:/image/button3.png);}");*/

    connect(buttonCreateRule,SIGNAL(clicked()),this,SLOT(slot_createNewRule()));
    connect(buttonCancelDraw,SIGNAL(clicked()),this,SLOT(slot_cancelDraw()));
    connect(buttonRestLast,SIGNAL(clicked()),this,SLOT(slot_undolastline()));
    connect(buttonRedo,SIGNAL(clicked()),this,SLOT(slot_redolastline()));
    connect(buttonEndDraw,SIGNAL(clicked()),this,SLOT(slot_enddraw()));

    setRuleButtonVisible(false);
}

//初始化图片展示窗口
void ConfigWidget::initPictureMap()
{
    labloading=new QLabel(this);
    labloading->setFixedSize(middleWindowWidth,middleDownWindowheight*0.82);
    labloading->setGeometry(leftWindowWidth,middleUpWindowheight,labloading->width(),labloading->height()*0.95);
    labloading->setVisible(false);

    //--------------------------------------------图片展示区
    m_video=new PictureBox(this);
    m_video->setFixedSize(middleWindowWidth, middleDownWindowheight*0.82);
    m_video->setGeometry(leftWindowWidth, middleUpWindowheight, m_video->width(), m_video->height()*0.95);

    connect(m_video, SIGNAL(chooseRule(QString,QString)), this, SLOT(slotChooseRule(QString, QString)) );   
}


//初始化规则属性窗口
void ConfigWidget::initProperWindow()
{

    m_ruleWidget = new RuleWidget(this);
    m_ruleWidget->setFixedSize(rightWindowWidth, windowHeight*0.8);
    m_ruleWidget->setGeometry(leftWindowWidth+middleWindowWidth, 10, m_ruleWidget->width(), m_ruleWidget->height()*0.95);


    buttonConfirm=new QPushButton(this);
    buttonConfirm->setFixedSize(80,40);
    buttonConfirm->setText("保存规则");
    buttonConfirm->setGeometry(leftWindowWidth+middleWindowWidth+10, windowHeight*0.8+20, 200, 50);
    buttonConfirm->setObjectName("buttonConfirm");
    buttonConfirm->setStyleSheet("#buttonConfirm{border-image: url(:/image/button2.png);}"
                               "#buttonConfirm:hover{border-image: url(:/image/button.png);}"
                               "#buttonConfirm:pressed{border-image: url(:/image/button3.png);}");

    buttonEdit=new QPushButton(this);
    buttonEdit->setFixedSize(80,40);
    buttonEdit->setText("编辑规则");
    buttonEdit->setGeometry(leftWindowWidth+middleWindowWidth+100, windowHeight*0.8+20, 200, 50);
    buttonEdit->setObjectName("buttonEdit");
    buttonEdit->setStyleSheet("#buttonEdit{border-image: url(:/image/button2.png);}"
                               "#buttonEdit:hover{border-image: url(:/image/button.png);}"
                               "#buttonEdit:pressed{border-image: url(:/image/button3.png);}");

    buttonDel=new QPushButton(this);
    buttonDel->setFixedSize(80,40);
    buttonDel->setText("删除规则");
    buttonDel->setGeometry(leftWindowWidth+middleWindowWidth+190, windowHeight*0.8+20, 200, 50);
    buttonDel->setObjectName("buttonDel");
    buttonDel->setStyleSheet("#buttonDel{border-image: url(:/image/button2.png);}"
                               "#buttonDel:hover{border-image: url(:/image/button.png);}"
                               "#buttonDel:pressed{border-image: url(:/image/button3.png);}");


    connect(buttonConfirm,SIGNAL(clicked()),this,SLOT(slotSave()));
    connect(buttonEdit,SIGNAL(clicked()),this,SLOT(slotEdit()));
    connect(buttonDel,SIGNAL(clicked()),this,SLOT(slotDel()));

    setRuleWidgetVisible(false);
}

void ConfigWidget::slotOpenMp4()
{
    g_ConfigOpenPath = QFileDialog::getExistingDirectory(NULL, "please choose mp4 file path!", g_ConfigOpenPath);
    if(g_ConfigOpenPath=="")
    {
        videoTreewidget->clear();
        return;
    }

    //1.mp4文件夹
    vector<QString> vecMp4;
    QDir dir(g_ConfigOpenPath);
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    for(int i=0;i<file_list.size();i++)
    {
        QFileInfo fileInfo=file_list[i];
        if(fileInfo.fileName().toLower().contains(".mp4"))
        {
            QString fileName = fileInfo.fileName();
            vecMp4.push_back(fileName.left(fileName.length()-4));
        }
    }

    //2.all rule
    ALARM_MANAGER->ReadConfigRuleFile(vecMp4);

    //3.树控件数据
    videoTreewidget->clear();

    for(size_t i=0; i<vecMp4.size(); i++)
    {
        QTreeWidgetItem *itemIPC = new QTreeWidgetItem(videoTreewidget, QStringList(QString(vecMp4[i])) );
        itemIPC->setIcon(0, QIcon(":/image/ipc_normal.png"));
        itemIPC->setToolTip(0,QString(vecMp4[i]) );
    }
}

//加载摄像头列表[AreaID],[AreaName],[AreaIP],[IPCID],[IPCName],[IPCRtspAddrMain]
void ConfigWidget::loadCameraTree()
{
    GLOBAL_DATA->ClearChannel();
    getCameraInfo();
    m_ruleWidget->init();

/*
    videoTreewidget->clear();
    IPCMap mapChannles = GLOBAL_DATA->GetChannel();

    QMap<QString,mArea>::const_iterator iterArea;
    for(iterArea = map_Area.constBegin();iterArea != map_Area.constEnd();iterArea++)
    {
        QTreeWidgetItem *itemArea = new QTreeWidgetItem
                (videoTreewidget, QStringList(iterArea.value().AreaName + "[" + iterArea.value().AreaAdress + "]"));
        itemArea->setIcon(0, QIcon(":/image/Area.png"));
        itemArea->setToolTip(0,QString(iterArea.value().AreaName + "[" + iterArea.value().AreaAdress + "]"));

        for(IPCMap::iterator itchannel = mapChannles.begin(); itchannel!=mapChannles.end(); ++itchannel)
        {
            QTreeWidgetItem *itemChannel = new QTreeWidgetItem
                    (itemArea, QStringList("通道[" + QString("%1").arg(itchannel->first) + "]"));

            vector<mIPC> &vecIPC = itchannel->second;
            for(size_t i=0; i<vecIPC.size(); i++)
            {
                QTreeWidgetItem *itemIPC = new QTreeWidgetItem(itemChannel, QStringList(QString(vecIPC[i].IPCName + "---" + vecIPC[i].IPCId)));
                itemIPC->setIcon(0, QIcon(":/image/ipc_normal.png"));
                itemIPC->setToolTip(0,QString(vecIPC[i].IPCName + "---"+vecIPC[i].IPCId));
                itemChannel->addChild(itemIPC);
            }
            itemArea->addChild(itemChannel);
        }
    }

    videoTreewidget->expandAll();
*/
}


void ConfigWidget::getCameraInfo()
{
    map_Area.clear();
    map_AlarmType.clear();
    map_LineType.clear();
    QString retCode="";
    QString server_id=g_AnalyseServerID;

    DATA_TRANSLATE->getCameraConfig(server_id,retCode);

    QString cameraId="";
    //DATA_TRANSLATE->getCameraRule(cameraId,retCode);
    DATA_TRANSLATE->getAlarmType(retCode);
    DATA_TRANSLATE->getLineType(retCode);
    //NETWORK_MANAGER->getCameraRTSPList();
}

void ConfigWidget::mousePressEvent(QMouseEvent *event)
{

}


//加载单个摄像头的配置规则
bool ConfigWidget::loadCameraRule(QImage &image)
{
    QString ipcid = videoTreewidget->currentIndex().data().toString();

    //1.image
    static int count = 0;

    ++count;
    if(m_ipcid != ipcid)
    {
        m_ipcid=ipcid;        
        count = 1;

        m_ruleWidget->setIpcID(ipcid);
    }    

    //start
    m_video->setVisible(false);

    labloading->setVisible(true);
    QMovie* movie = new QMovie(":/image/loading.gif");
    labloading->setAlignment(Qt::AlignCenter);
    labloading->setMovie(movie);
    movie->start();
    labloading->show();    

    //QString rtsp=QString("rtsp://admin:hnmqet123@192.168.1.252:554/h264/ch1/main/av_strea5");
    QString rtsp = g_ConfigOpenPath + "/" + m_ipcid + ".mp4";

    bool success=false;    
    cv::Mat mat;
    ffmpegDecode   *pFfm = new ffmpegDecode(PIC_WIDTH, PIC_HEIGHT);
    if(pFfm!=NULL && pFfm->init(rtsp))
    {
        int readcount=0;
        while(pFfm->readOneFrame())
        {
            if(pFfm->getOneFrame(mat) && !mat.empty())
            {                
                ++readcount;
                if(readcount>=count)
                {
                    success=true;
                    break;
                }
            }

        }
    }

    //stop
    movie->stop();
    delete movie;
    movie=NULL;
    labloading->setVisible(false);
    m_video->setVisible(true);


    if(success)
    {
        image = MatToQImage(mat);
    }
    else
    {
        image.load(":/image/imageError.jpg");
    }
    delete pFfm;
    return success;
}

void ConfigWidget::getScaleAndoffsetXY()
{
    double window_width, window_height;
    double image_width, image_height;
    double r1, r2;
    window_width = m_video->width();
    window_height = m_video->height();

    r1 = window_width / image_width;
    r2 = window_height / image_height;
    r = qMin(r1, r2);
    offsetX = (window_width - r * image_width) / 2;
    offsetY = (window_height - r * image_height) / 2;
}

void ConfigWidget::on_videoTreewidget_doubleClicked(const QModelIndex &index)
{    
    m_video->reset();
    m_video->clearRulePoint();
    setRuleWidgetVisible(false);

    QImage image;
    if(loadCameraRule(image))
    {
        m_video->setPixmap(QPixmap::fromImage(image));

        g_Offset_x = (m_video->width() - m_video->pixmap()->width())/2;
        g_Offset_y = (m_video->height() - m_video->pixmap()->height())/2;
        if(g_Offset_x<=0) g_Offset_x=0;
        if(g_Offset_y<=0) g_Offset_y=0;

        m_video->loadRule(m_ipcid);

        //test
        //image.save("/home/mq/wjt/test.jpg");
        setRuleButtonVisible(true);
        buttonCreateRule->setEnabled(true);
        buttonCancelDraw->setEnabled(false);
        buttonRestLast->setEnabled(false);
        buttonRedo->setEnabled(false);
        buttonEndDraw->setEnabled(false);

    }
    else
    {
        setRuleButtonVisible(false);
        m_video->setPixmap(QPixmap::fromImage(image));
        QMessageBox::about(NULL, "获取监控场景失败", "请重新双击摄像头尝试获取");
    }
}

void ConfigWidget::configValue(const QString &ipcID, const QString &ruleID)
{
    setRuleWidgetVisible(true);

    QString rulename="";
    //QString ruleid="0";
    int lineType=2;
    int alarmType=1;
    //QString ipcid=m_ipcid;
    int alarmDelay=10;

    int personMin=1;
    int personMax = 1;
    int alarm_threshold = 80;
    int two_remind_time = 60;
    QStringList timelist;

    //如果包含点的规则,则属性表加载此规则    
    mRule rule;
    if(ALARM_MANAGER->HasConfigRule(ipcID, ruleID, rule))
    {        
        rulename=rule.rule_name;
        lineType=rule.line_type;
        alarmType=rule.alarm_type;
        alarmDelay=rule.alarm_delay;
        personMin=rule.alarm_person_min;
        personMax = rule.alarm_person_max;
        alarm_threshold = rule.alarm_threshold;
        two_remind_time= rule.two_remind_time;
        timelist=rule.times;
    }


    m_ruleWidget->setRuleName(rulename);
    m_ruleWidget->setLineType(lineType);
    m_ruleWidget->setAlarmType(alarmType);
    m_ruleWidget->setPersonMin(personMin);
    m_ruleWidget->setPersonMax(personMax);
    m_ruleWidget->setAlarmLoop(two_remind_time);
    m_ruleWidget->setAlarmDelay(alarmDelay);
    m_ruleWidget->setAlarmPercent(alarm_threshold);
    m_ruleWidget->setTimes(timelist);
}

bool ConfigWidget::saveRule(const QString &ipcID, const QString &ruleID, bool isUpdate/*=false*/)
{

    QString rulename = m_ruleWidget->getRuleName();
    int lineType = m_ruleWidget->getLineType();
    int alarmType = m_ruleWidget->getAlarmType();
    int personMin = m_ruleWidget->getPersonMin();
    int personMax = m_ruleWidget->getPersonMax();
    int alarmDelay = m_ruleWidget->getAlarmDelay();
    int alarmLoop = m_ruleWidget->getAlarmLoop();
    int percent = m_ruleWidget->getAlarmPercent();
    QStringList strTimes = m_ruleWidget->getTimes();

    if(rulename.isEmpty())
    {
        QMessageBox::about(NULL, "配置不完整", "请输入规则名称");
        return false;
    }
    if(lineType<=0)
    {
        QMessageBox::about(NULL, "配置不完整", "请选择画线类型");
        return false;
    }
    if(alarmType<=0)
    {
        QMessageBox::about(NULL, "配置不完整", "请选择报警类型");
        return false;
    }

    if(personMax < personMin)
    {
        QMessageBox::about(NULL, "配置不完整", "最多报警人数<最少报警人数");
        return false;
    }
    if(strTimes.size()==0)
    {
        QMessageBox::about(NULL, "配置不完整", "请选择监控时间段，可多选");
        return false;
    }

    mRule rule;
    rule.rule_name = rulename;
    rule.rule_id = ipcID+ruleID;
    rule.alarm_type = alarmType;
    rule.camera_id = ipcID;
    rule.line_type=(emLineType)lineType;
    rule.alarm_person_min = personMin;
    rule.alarm_person_max = personMax;
    rule.alarm_threshold = percent;
    rule.alarm_delay = alarmDelay;
    rule.two_remind_time = alarmLoop;
    rule.times = strTimes;
    rule.points = m_video->getDrawPoint();

    bool success=false;
    if(isUpdate)
    {
        rule.rule_id = ruleID;
        success=ALARM_MANAGER->UpdateConfigRule(rule);
    }
    else
    {
        rule.rule_id = ipcID+ruleID;
        success=ALARM_MANAGER->SaveConfigRule(rule);
    }

    return success;
}

void ConfigWidget::slot_createNewRule()
{
    if(m_ipcid!="")
    {
        m_video->setIsNewRule(true);

        m_ruleid = QString("%1").arg(ALARM_MANAGER->getConfigRuleID(m_ipcid));
        configValue(m_ipcid, m_ruleid);                       

        buttonCreateRule->setEnabled(false);
        buttonCancelDraw->setEnabled(true);
        buttonRestLast->setEnabled(true);
        buttonRedo->setEnabled(true);
        buttonEndDraw->setEnabled(true);

        buttonConfirm->setEnabled(true);
        buttonEdit->setEnabled(false);
        buttonDel->setEnabled(false);
    }
}

void ConfigWidget::slot_cancelDraw()
{    
    m_isEdit=false;
    m_video->reset();
    m_video->loadRule(m_ipcid);

    setRuleWidgetVisible(false);
    buttonCreateRule->setEnabled(true);
    buttonCancelDraw->setEnabled(false);
    buttonRestLast->setEnabled(false);
    buttonRedo->setEnabled(false);
    buttonEndDraw->setEnabled(false);
}

void ConfigWidget::slot_undolastline()
{
    m_video->lastDrawPoint();
}

void ConfigWidget::slot_redolastline()
{
    m_video->nextDrawPoint();
}

void ConfigWidget::slot_enddraw()
{
    if(!m_video->endDrawLine())
    {
        return;
    }

    buttonCreateRule->setEnabled(false);
    buttonCancelDraw->setEnabled(false);
    buttonRestLast->setEnabled(false);
    buttonRedo->setEnabled(false);
    buttonEndDraw->setEnabled(false);
}

void ConfigWidget::slotChooseRule(const QString &ipcID, const QString &ruleID)
{
    if(ipcID=="" || ruleID=="")
    {
        setRuleWidgetVisible(false);
    }
    else
    {
        if(m_ipcid!=ipcID)
        {
            QMessageBox::about(NULL, "规则异常", "规则CameraID异常");
            return;
        }
        m_ruleid=ruleID;

        configValue(ipcID, ruleID);

        buttonConfirm->setEnabled(false);
        buttonDel->setEnabled(true);
        buttonEdit->setEnabled(true);
    }
}

void ConfigWidget::slotSave()
{
    if(!m_video->IsDraw() && m_video->getDrawPointSize()>0)
    {       
        if(saveRule(m_ipcid, m_ruleid, m_isEdit))
        {            
            m_video->reset();
            m_video->loadRule(m_ipcid);

            setRuleWidgetVisible(false);
            buttonCreateRule->setEnabled(true);
            buttonCancelDraw->setEnabled(false);
            buttonRestLast->setEnabled(false);
            buttonRedo->setEnabled(false);
            buttonEndDraw->setEnabled(false);

            QMessageBox::about(this, "温馨提示", "报警规则保存成功！");
        }
        else
        {
            QMessageBox::about(this, "温馨提示", "报警规则保存失败！");
        }
    }
    else
    {
         QMessageBox::about(this, "温馨提示", "请绘制规则！");
    }
}

void ConfigWidget::slotEdit()
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("是否编辑当前规则"),
                                   QString(tr("放弃请按\"No\",确认请按\"Yes\"")),
                                   QMessageBox::Yes | QMessageBox::No);

    if (button != QMessageBox::Yes) return;


    m_video->editRule(m_ipcid, m_ruleid);
    m_isEdit=true;

    buttonCreateRule->setEnabled(false);
    buttonCancelDraw->setEnabled(true);
    buttonRestLast->setEnabled(true);
    buttonRedo->setEnabled(true);
    buttonEndDraw->setEnabled(true);

    buttonConfirm->setEnabled(true);
    buttonDel->setEnabled(false);
    buttonEdit->setEnabled(false);
}

void ConfigWidget::slotDel()
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("是否确定删除当前规则"),
                                   QString(tr("放弃请按\"No\",确认请按\"Yes\"")),
                                   QMessageBox::Yes | QMessageBox::No);

    if (button != QMessageBox::Yes) return;


    ALARM_MANAGER->RemoveConfigRule(m_ipcid, m_ruleid);
    m_video->loadRule(m_ipcid);

    setRuleWidgetVisible(false);
    buttonCreateRule->setEnabled(true);
    buttonCancelDraw->setEnabled(false);
    buttonRestLast->setEnabled(false);
    buttonRedo->setEnabled(false);
    buttonEndDraw->setEnabled(false);
}

void ConfigWidget::slot_addtime()
{
    if(timeEdStart->time()>=timeEdStop->time())
    {
        QMessageBox::about(NULL, "温馨提示", "时间选择不正确，结束时间需大于开始时间。");
        return;
    }

    QString alarmNumDuringTime=lineEditAlarmNumDuringTime->text();
    QString time=QString("%1-%2-%3").arg(timeEdStart->time().toString("hh:mm:ss")).arg(timeEdStop->time().toString("hh:mm:ss")).arg(alarmNumDuringTime.toInt()+1);
    bool iscontains=false;
    for(int i=0;i<lvtAlarmTimes->count();i++)
    {
        QStringList timelist=lvtAlarmTimes->item(i)->text().split("-");
        QTime startTime=QTime::fromString(timelist[0],"hh:mm:ss");
        QTime stopTime=QTime::fromString(timelist[1],"hh:mm:ss");
        //时间段a->b ,c->d  ,(a-d)*(b-c)<0 则有重复
        double a=timeEdStart->time().secsTo(stopTime);
        double c=timeEdStop->time().secsTo(startTime);
        if (a*c<0)
        {

            iscontains=true;
        }
    }
    if(!iscontains)
        lvtAlarmTimes->addItem(time);
    else
        QMessageBox::about(NULL, "温馨提示", "您新增的时间段与之前时间段有重复，请更改！");
}

void ConfigWidget::slot_deltime()
{
    int row=lvtAlarmTimes->currentRow();
    lvtAlarmTimes->takeItem(row);
}

void ConfigWidget::setRuleWidgetVisible(bool visible)
{   
    buttonConfirm->setVisible(visible);
    buttonDel->setVisible(visible);
    buttonEdit->setVisible(visible);
    m_ruleWidget->setVisible(visible);
}

void ConfigWidget::setRuleButtonVisible(bool visible)
{
    buttonCreateRule->setVisible(visible);
    buttonCancelDraw->setVisible(visible);
    buttonRestLast->setVisible(visible);
    buttonRedo->setVisible(visible);
    buttonEndDraw->setVisible(visible);
}

