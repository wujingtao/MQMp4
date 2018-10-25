#include "cameraserver.h"
#include "qmyglobal.h"

CameraServer::CameraServer(int channelID, const vector<mIPC> &vecIPC)
{
    m_channelID=channelID;
    m_vecIPC=vecIPC;
    m_pFfm=NULL;

    m_isRun=false;
}

CameraServer::CameraServer()
{    
    m_pFfm=NULL;
    m_isRun=false;
}

void CameraServer::SetRtsp(ffmpegDecode *pFfm, const QString &rtsp)
{
    if(m_pFfm!=NULL)
    {
        delete m_pFfm;
    }
    m_pFfm=pFfm;
    m_rtsp=rtsp;
}

void CameraServer::run()
{    
   if(m_pFfm==NULL) return;

    m_isRun=true;

    ffm_init();

    int frameRate = m_pFfm->getFrameRate();
    int sleepFrame = 30;
    if(frameRate > 0  && frameRate<1000)
    {
        sleepFrame = 1000/m_pFfm->getFrameRate();
    }


    int readcount=0;
    while(m_pFfm!=NULL && m_pFfm->readOneFrame())
    {
        msleep(sleepFrame);
        ++readcount;
    }

    LOG_INFO("cameraserver run end.readcount:%d, %s.\n", readcount, m_rtsp.toStdString().c_str());
    m_isRun=false;
}

bool CameraServer::IsRun()
{
    return m_isRun;
}

bool CameraServer::init()
{
    QTime curtime = QTime::currentTime();
    for(size_t i=0; i<m_vecIPC.size(); i++)
    {
        if(m_vecIPC[i].minutes == 0)
        {
            QTime start_time = m_vecIPC[i].start_time;
            QTime end_time = m_vecIPC[i].end_time;

            if(curtime>start_time && curtime<end_time)
            {
                switchCamera(i, curtime);
                return true;
            }
        }
        else
        {
            switchCamera(i, curtime);
            return true;
        }
    }

    return false;
}

void CameraServer::loop(const QTime &curtime)
{   
    int index=0;
    for(size_t i=0; i<m_vecIPC.size(); i++)
    {
        index = m_index+i;
        if(index==m_vecIPC.size())
        {
            index=0;
        }

        int minutes = m_vecIPC[index].minutes;

        if(minutes>0)
        {
            //TODO:test
            /*
            if(m_channelID==2)
            {
                LOG_INFO(" minutes to sec:%d.\n", minutes*60);
            }
            */

            if(i==0)
            {
                if(m_startTime.addSecs(minutes*60) > curtime) break;
            }
            else
            {
                switchCamera(index, curtime);
                break;
            }
        }
        else
        {
            QTime start_time = m_vecIPC[index].start_time;
            QTime end_time = m_vecIPC[index].end_time;

            //TODO:test
            /*
            if(m_channelID==2)
            {
                LOG_INFO("ct:%s, st:%s, et:%s.\n", curtime.toString("hh:mm:ss").toStdString().c_str(),
                         start_time.toString("hh:mm:ss").toStdString().c_str(),
                         end_time.toString("hh:mm:ss").toStdString().c_str());
            }
            */

            if(curtime>start_time && curtime<end_time)
            {
                if(i!=0)
                {
                    switchCamera(index, curtime);
                }
                break;
            }

        }
    }

}

void CameraServer::switchCamera(int index, const QTime &curtime)
{
    GLOBAL_DATA->SetCameraArgIndex(m_channelID, index);

    tagCameraArg cameraArg;
    if(GLOBAL_DATA->GetCameraArg(m_channelID, cameraArg))
    {
        m_pFfm = cameraArg.pFfm;
        m_rtsp = cameraArg.rtsp;

        m_index=index;

        //ffm_init();

        m_startTime=curtime;

        //TODO:test
        //if(m_channelID==2)
        //{
            //LOG_INFO("index:%d, cameraID:%s.\n", index, cameraArg.ipcid.toStdString().c_str());
        //}
    }
    else
    {
       GLOBAL_DATA->SetCameraArgIndex(m_channelID, m_index);
    }
}

void CameraServer::ffm_init()
{
    int conncount = 0;
    int base = 2000;    
    while(m_pFfm!=NULL && !m_pFfm->init(m_rtsp))
    {
        if(base>2000*1800)
            base==2000*1800;
        else
            base = base*2;

        LOG_WARN("rtsp:%s, ffm_init() error count:%d.\n", m_rtsp.toStdString().c_str(), ++conncount);
        msleep(base);
    }
}
