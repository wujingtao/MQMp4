#include "videowidget.h"
#include <QPalette>
#include <QHeaderView>
#include<QThread>
#include<qtimer.h>
#include<qdebug.h>
#include<QMessageBox>
#include "GlobalData.h"
#include <qfiledialog.h>
#include "AlarmManager.h"
//暂时不用，后期可拓展
VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent)
{
    initWindow();

    m_pMp4Server=NULL;
}

QFileInfoList VideoWidget::GetFileList(QString path)
{
    QDir dir(path);
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
//    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

//    for(int i = 0; i != folder_list.size(); i++)
//    {
//        QString name = folder_list.at(i).absoluteFilePath();
//        QFileInfoList child_file_list = GetFileList(name);
//        file_list.append(child_file_list);
//    }

    return file_list;
}
void VideoWidget::initWindow()
{
    setMinimumSize(screenX,screenY*0.91);
    setMaximumSize(screenX,screenY*0.91);
    location=this->geometry();
    int treewidth=location.width()*0.2;
    int videowidth=location.width()*0.8/4;

    int height=location.height();
    int videoheight=height/4-2;

    refreshTreeAndConfig=new QPushButton(this);
    refreshTreeAndConfig->setFixedSize(screenX*0.08,screenY*0.04);
    refreshTreeAndConfig->setText("打开MP4");
    refreshTreeAndConfig->setObjectName("refreshTreeAndConfig");
    refreshTreeAndConfig->setStyleSheet("#refreshTreeAndConfig{border-image: url(:/image/button2.png);}"
                                        "#refreshTreeAndConfig:hover{border-image: url(:/image/button.png);}"
                                        "#refreshTreeAndConfig:pressed{border-image: url(:/image/button3.png);}");
    refreshTreeAndConfig->setGeometry(5,5,screenX*0.08,screenY*0.04);
    //摄像头树
    videoTreewidget=new QTreeWidget(this);
    videoTreewidget->setFixedSize(screenX*0.157,screenY*0.91);
    videoTreewidget->setWindowTitle("视频列表");
    videoTreewidget->header()->setVisible(false);
    videoTreewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView *head=videoTreewidget->header();
    videoTreewidget->setStyleSheet("background:transparent; font-size:17px; color: white;");
    videoTreewidget->setWindowFlags(Qt::FramelessWindowHint);
    head->setSectionResizeMode(QHeaderView::ResizeToContents );
    videoTreewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    videoTreewidget->setGeometry(0,screenY*0.04,screenX*0.157,screenY*0.91);
    //connect(refreshTreeAndConfig,SIGNAL(clicked()),this,SLOT(loadCameraTree()));
    connect(refreshTreeAndConfig,SIGNAL(clicked()),this,SLOT(slotOpenMp4()));
    connect(videoTreewidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(on_videoTreewidget_doubleClicked(QModelIndex)));


    video_1=new QLabel(this);
    video_1->setFixedSize(location.width()*0.8,location.height());
    video_1->setFrameShape(QFrame::Box);
    video_1->setFrameShadow(QFrame::Sunken);
    video_1->setGeometry(treewidth,5,location.width()*0.8,location.height());
}

void VideoWidget::readImg()
{
    QFileInfoList file_list,file_listTemp;
    file_list.clear();
    file_listTemp = GetFileList(url);
    for(int i=0;i<file_listTemp.size();i++)
    {
        QFileInfo fileInfo=file_listTemp[i];
        if(fileInfo.fileName().toLower().contains("jpg")||fileInfo.fileName().toLower().contains("png"))
        {
            file_list.append(fileInfo);//指定后缀，加入列表
        }
    }
    if(file_list.size()<3) return;
    QFileInfo fileInfo = file_list.at(file_list.size() - 2);
    QString path = url + fileInfo.fileName();
    if(!path.contains(".mp4",Qt::CaseSensitive) && !path.contains(".gif",Qt::CaseSensitive))
    {
        if(path.toLower().contains("jpg")||path.toLower().contains("png"))
        {
            QImage image;
            image.load(path);
            video_1->clear();
            video_1->setPixmap(QPixmap::fromImage(image).scaled(location.width()*0.8,location.height(), Qt::IgnoreAspectRatio));
            //video_1->setPixmap(QPixmap::fromImage(image));

            video_1->update();
        }
    }
}

void VideoWidget::closeImg()
{
    timer->stop();
}

void VideoWidget::slotOpenMp4()
{
    if(m_pMp4Server!=NULL && m_pMp4Server->IsRun())
    {
        QMessageBox::about(NULL, "MP4播放中", QString("请等待MP4运行完成,剩余数量:%1").arg(m_pMp4Server->getLeaveCount()));
        return;
    }

     g_VideoOpenPath = QFileDialog::getExistingDirectory(NULL, tr("Open Directory mp4"), g_VideoOpenPath);
     if(g_VideoOpenPath=="")
     {
         videoTreewidget->clear();
         return;
     }

     //1.
    vector<QString> vecMp4;
    QDir dir(g_VideoOpenPath);
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

    //2.
    ALARM_MANAGER->ReadRuleFile(vecMp4);

    //3.
    videoTreewidget->clear();

    for(size_t i=0; i<vecMp4.size(); i++)
    {
        QTreeWidgetItem *itemIPC = new QTreeWidgetItem(videoTreewidget, QStringList(QString(vecMp4[i])) );
        itemIPC->setIcon(0, QIcon(":/image/ipc_normal.png"));
        itemIPC->setToolTip(0,QString(vecMp4[i]) );
    }

    m_pMp4Server = new Mp4Server(vecMp4);
    m_pMp4Server->start();

}

//加载摄像头列表[AreaID],[AreaName],[AreaIP],[IPCID],[IPCName],[IPCRtspAddrMain]
void VideoWidget::loadCameraTree()
{
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
}

void VideoWidget::on_videoTreewidget_doubleClicked(const QModelIndex &index)
{
    showCamera();
}

//加载单个摄像头的报警画面
void VideoWidget::showCamera()
{    
    QString temp = videoTreewidget->currentIndex().data().toString();
    /*
    QStringList list=temp.split("---");
    if(list.size()>=2)
    {
        QString ipcid=list[1];
        url= QString("/home/mq/video/%1/dealed/").arg(ipcid);
    }
    */

    url=QString("/home/mq/wjt/video/%1/dealed/").arg(temp);
}

