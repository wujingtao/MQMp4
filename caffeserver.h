#ifndef CAFFESERVER_H
#define CAFFESERVER_H
#include <qmyglobal.h>
#include <qthread.h>
#include "objectdetector.h"
#include "GlobalData.h"

class CaffeServer : public QThread
{
public:
    CaffeServer(const ChannelVector &vecChannel);
    CaffeServer();

    void SetArg(ffmpegDecode *pFfm, const QString &cameraID, const QString &savePath);

    void run();

    bool IsRun();

private:
    void getRectsFromImage(const cv::Mat &inputimage, ObjectDetector *pDetector, tagDetectAll &detectAll);

private:
    ChannelVector m_vecChannel;
    ffmpegDecode  *m_pFfm;
    imageAnalyse *m_pAnalyse;

    bool m_isRun;
};

#endif // CAFFESERVER_H
