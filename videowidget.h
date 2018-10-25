#ifndef WIDGET2_H
#define WIDGET2_H

#include <QWidget>
#include <QLabel>
#include <QTreeWidget>
#include "qmyglobal.h"
#include<qtimer.h>
#include "Mp4Server.h"

class VideoWidget :public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);

    void initWindow();
    void closeImg();
    void showCamera();

public slots:
    void readImg();
    void loadCameraTree();
    void on_videoTreewidget_doubleClicked(const QModelIndex &index);
    static QFileInfoList GetFileList(QString path);

    void slotOpenMp4();

private:
    QRect location;
    QPushButton * refreshTreeAndConfig; //刷新树列表和配置
    QTreeWidget * videoTreewidget;
    QLabel * video_1;

    QTimer *timer;
    QString url;

    Mp4Server *m_pMp4Server;
};

#endif // WIDGET2_H
