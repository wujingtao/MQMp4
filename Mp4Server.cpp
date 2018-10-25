#include "Mp4Server.h"
#include "GlobalData.h"

#include "cameraserver.h"
#include "caffeserver.h"

Mp4Server::Mp4Server(const vector<QString> vecRtsp)
{
    m_vecRtsp = vecRtsp;

    m_isRun=false;
    m_leaveCount=0;
}


void Mp4Server::run()
{
    m_isRun=true;

    for(size_t i=0; i<m_vecRtsp.size(); i++)
    {
        m_leaveCount = m_vecRtsp.size()-i;

        bool hasCamera=false;
        while(!hasCamera)
        {
            Mp4ArgVecotr vecMp4Arg;
            GLOBAL_DATA->GetMp4Arg(vecMp4Arg);

            for(size_t k=0; k<vecMp4Arg.size(); k++)
            {
                CameraServer *pCameraServer = vecMp4Arg[k].pCameraServer;

                if(!pCameraServer->IsRun())
                {                    
                    QString path = "/home/mq/wjt/video/" + m_vecRtsp[i];
                    QDir dir(path);
                    if(!dir.exists()) dir.mkdir(path);

                    ffmpegDecode *pFfm = new ffmpegDecode(PIC_WIDTH, PIC_HEIGHT);

                    CaffeServer *pCaffeServer = vecMp4Arg[k].pCaffeServer;
                    pCaffeServer->SetArg(pFfm, m_vecRtsp[i], path);
                    if(!pCaffeServer->IsRun())
                    {
                        pCaffeServer->start();
                    }

                    pCameraServer->SetRtsp(pFfm, g_VideoOpenPath + "/"+ m_vecRtsp[i]+".mp4");
                    pCameraServer->start();

                    hasCamera=true;
                    break;
                }
            }

            msleep(1000);
        }
    }

    m_isRun=false;
    m_leaveCount=0;
}

int Mp4Server::getLeaveCount()
{
    return m_leaveCount;
}

bool Mp4Server::IsRun()
{
    return m_isRun;
}
