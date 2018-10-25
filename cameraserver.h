#ifndef CAMERASERVER_H
#define CAMERASERVER_H

/***************************************************************************
 * //rtsp截图片线程
 * //根据摄像头数据确定开启的rtsp流的数目
 * ************************************************************************/
#include <QTime>

#include "qthread.h"
#include "ffmpegdecode.h"
#include "GlobalData.h"

class CameraServer:public QThread
{
public:
    CameraServer(int channelID, const vector<mIPC> &vecIPC);    
    CameraServer();
    void SetRtsp(ffmpegDecode *pFfm, const QString &rtsp);

    void run();

    bool IsRun();

private:
    void ffm_init();
    bool init();
    void loop(const QTime &curtime);
    void switchCamera(int index, const QTime &curtime);    

private:
    ffmpegDecode *m_pFfm;
    QString m_rtsp;    

    int m_channelID;
    QTime m_startTime;

    vector<mIPC> m_vecIPC;
    int m_index;

    bool m_isRun;
};

#endif // CAMERASERVER_H
