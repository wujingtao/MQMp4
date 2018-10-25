#ifndef FFMPEGDECODE_H
#define FFMPEGDECODE_H
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QMutex>

#include "qmyglobal.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}

class ffmpegDecode
{
public:
    ffmpegDecode(int width,int height);
    ~ffmpegDecode();

public:
    static void Register();
    bool IsInit();
    bool init(const QString &rtsp);
    void close();

    bool readOneFrame(bool keyframe=false);
    bool getOneFrame(cv::Mat &mat);
    int getFrameRate();

private:
    bool openDecode();
    bool prepare();
    void get(AVCodecContext *pCodecCtx, SwsContext *img_convert_ctx,AVFrame    *pFrame);

private:
    AVFrame    *pAvFrame;
    AVFormatContext    *m_pFormatCtx;
    AVCodecContext    *pCodecCtx;

    int videoindex;

    SwsContext *img_convert_ctx;
    AVPacket *packet;

    cv::Mat *m_pCvMat;
    int m_width,m_height;

    bool m_isInit;
    QMutex m_mutex;
    bool m_success;

    bool m_keyframe;
    int m_frameRate;
};
#endif // FFMPEGDECODE_H
