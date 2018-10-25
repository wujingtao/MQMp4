#include "AlarmManager.h"
#include <QDateTime>
#include "qdrawline.h"
#include <QPainter>
#include <QProcess>
#include <qjudgeposition.h>
#include <sstream>
#include <fstream>
#include <QDebug>
using namespace std;

AlarmManager* AlarmManager::s_alarmManager = NULL;

AlarmManager* AlarmManager::Instance()
{
    if(s_alarmManager==NULL)
    {
        s_alarmManager=new AlarmManager;
    }

    return s_alarmManager;
}

AlarmManager::AlarmManager()
{

}

void AlarmManager::Alarm(const QString &filePath, const QString &cameraID, const tagDetectAll &detectAll)
{
    //
    int nowtime=time(NULL);
    QTime currentTime=QTime::currentTime();

    RuleMap::iterator it = m_rules.find(cameraID);
    if(it==m_rules.end())
    {
        return;
    }

    bool firstSave =true;
    RuleList &listRule = it->second;
    for(RuleList::iterator itlist = listRule.begin(); itlist!=listRule.end(); ++itlist)
    {

        QStringList times=itlist->times;
        bool isDeal = false;

        for(int i=0; i<times.size(); i++)
        {
            QString time=times[i];
            QStringList timelist = time.split("-");
            QTime startTime=QTime::fromString(timelist[0],"hh:mm:ss");
            QTime stopTime=QTime::fromString(timelist[1],"hh:mm:ss");
            if(currentTime>startTime&&currentTime<stopTime)
            {
                //if(itlist->alarm_type!=6 || detectAll.heads.size()>=2)//特殊处理，双人值岗人头需大于2.
                    isDeal=true;

                break;
            }
        }        

        if(isDeal)
        {
            if(firstSave)
            {
                saveImageName(cameraID, filePath);
                firstSave=false;
            }

            alarmAnaly(filePath, detectAll, *itlist, nowtime, currentTime);
        }
        else
        {
            clearAlarmStamp(itlist->rule_id);
        }

    }

    if(firstSave)
    {
        clearImageName(cameraID);
    }
}

void AlarmManager::saveImageName(const QString &cameraID, const QString &imageName)
{
    m_mutexImage.lock();
    ImageNameMap::iterator it = m_imageNames.find(cameraID);
    if(it != m_imageNames.end())
    {
        ImageNameList &listImageName=it->second;
        if(listImageName.size()>60)
        {
            listImageName.pop_front();
        }

        listImageName.push_back(imageName);
    }
    else
    {
        ImageNameList listImageName;
        listImageName.push_back(imageName);

        m_imageNames[cameraID]=listImageName;
    }
    m_mutexImage.unlock();
}

void AlarmManager::clearImageName(const QString &cameraID)
{
    m_mutexImage.lock();
    ImageNameMap::iterator it = m_imageNames.find(cameraID);
    if(it!=m_imageNames.end())
        m_imageNames.erase(it);
    m_mutexImage.unlock();
}

//返回mp4路径,filePath:/dealed/1570/1234567.mp4
bool AlarmManager::CreateMp4(const QString &cameraID, const QString &filePath)
{
    vector<QString> vecMp4Image;

    ImageNameMap::iterator it = m_imageNames.find(cameraID);
    if(it != m_imageNames.end())
    {
        int limit=0;
        ImageNameList &listImageName = it->second;
        for(ImageNameList::reverse_iterator itlist=listImageName.rbegin(); itlist!=listImageName.rend() && limit<MP4_IMAGE_COUNT; ++itlist)
        {
            vecMp4Image.push_back(*itlist);
            ++limit;
        }
    }

    //create a folder for ffmpeg convert pictures to mp4, for it must be like 0.jpg 1.jpg 2.jpg 3.jpg
    QString mp4dir=filePath;
    mp4dir.replace(".mp4","");

    QDir dirmp4(mp4dir);
    if(!dirmp4.exists())
       dirmp4.mkdir(mp4dir);

    int imageNum=vecMp4Image.size();
    for(int i=0;i<imageNum;i++)
    {
        QString newFileName=mp4dir+QString("/%1.jpg").arg(imageNum-i-1);
        QFile::copy(vecMp4Image[i],newFileName);
    }

    QString picsPare=QString("%1/%d.jpg").arg(mp4dir);

    QProcess  process;
    QStringList paramstr;
    //paramstr<<"-delay"<<QString("%1").arg(numPerMinute)<<"-loop"<<"1"<<picsParam<<"-resize"<<size<<outPutPath;
    paramstr<<"-r"<<"1"<<"-f"<<"image2"<<"-i"<<picsPare<<"-vcodec"<<"h264"<<"-y"<<filePath;
    //process.execute(QString("/usr/bin/convert"),paramstr);
    process.execute(QString("/usr/bin/ffmpeg"),paramstr);//command是要执行的命令,args是参数，
    qDebug()<<paramstr;
    dirmp4.removeRecursively();
    dirmp4.rmdir(mp4dir);

    /*
    if (!process.waitForStarted())
    {
        QString str = QString::fromLocal8Bit(process.readAllStandardError());
        //qDebug()<<"error"<<QString::fromLocal8Bit(process.readAllStandardError());
        LOG_ERROR("createmp4 error:%s.\n", str.toStdString().c_str());
        return false;
    }
    else
    {
        LOG_INFO("create mp4 success.\n");
    }
    */

    return true;
}

void AlarmManager::PushAlarmRecord(const mAlarmRecord_stu &record)
{
    //TODO:大小限制
    m_mutexRecord.lock();
    m_listAlarmRecord.push_back(record);
    m_mutexRecord.unlock();
}

bool AlarmManager::PopAlarmRecord(mAlarmRecord_stu &record)
{
    bool ret=false;
    m_mutexRecord.lock();
    if(!m_listAlarmRecord.empty())
    {
        record = m_listAlarmRecord.front();
        m_listAlarmRecord.pop_front();

        ret=true;
    }
    m_mutexRecord.unlock();

    return ret;
}

void AlarmManager::ReadRuleFile(const vector<QString> &vecFile)
{
    m_rules.clear();

    QString name;
    char buff[1024];
    for(size_t i=0; i<vecFile.size(); i++)
    {
        name = g_VideoOpenPath+"/"+vecFile[i]+".cfg";

        ifstream ifs(name.toStdString().c_str());
        if(ifs.is_open())
        {
            RuleList listRule;
            while(!ifs.eof())
            {
                ifs.getline(buff, 1024);

                QString str(buff);
                QStringList strlist = str.split(';');

                mRule rule;
                if(strlist.size()>=11)
                {
                    rule.rule_name = strlist[0];
                    rule.rule_id = strlist[1];
                    rule.alarm_type = strlist[2].toInt();
                    rule.camera_id = strlist[3];
                    rule.line_type = (emLineType)strlist[4].toInt();
                    rule.alarm_person_min = strlist[5].toInt();
                    rule.alarm_person_max = strlist[6].toInt();
                    rule.alarm_delay=strlist[7].toInt();
                    rule.alarm_threshold = strlist[8].toInt();
                    rule.two_remind_time = strlist[9].toInt();
                    //
                    QString strtimes = strlist[10];
                    strtimes.replace(")","");
                    strtimes.replace("(","");
                    rule.times=strtimes.split(",");
                    //
                    QString strPoint = strlist[11];
                    QStringList strlistPoint=strPoint.split("),(");
                    for(int i=0;i<strlistPoint.size();i++)
                    {
                        QString pointstr=strlistPoint[i];
                        pointstr.replace(")","");
                        pointstr.replace("(","");
                        QStringList pointx_y=pointstr.split(",");
                        if(pointx_y.size()==2)
                        {
                            int x=pointx_y[0].toInt();
                            int y=pointx_y[1].toInt();

                            QPoint point(x,y);
                            rule.points.push_back(point);
                        }
                    }

                    listRule.push_back(rule);
                }

            }
            m_rules[vecFile[i]] = listRule;

        }
    }
}

void AlarmManager::SaveRule(const QString &cameraID, const mRule &rule)
{
    m_mutexRule.lock();
    RuleMap::iterator it = m_rules.find(cameraID);

    RuleList listRule;
    if(it != m_rules.end())
    {        
        it->second.push_back(rule);

        listRule=it->second;
    }
    else
    {        
        listRule.push_back(rule);

        m_rules[cameraID]=listRule;
    }
    m_mutexRule.unlock();
}

void AlarmManager::RemoveRule(const QString &cameraID, const QString &ruleID)
{
    m_mutexRule.lock();
    RuleMap::iterator it = m_rules.find(cameraID);
    if(it != m_rules.end())
    {
        RuleList &listRule = it->second;
        for(RuleList::iterator itlist = listRule.begin(); itlist!=listRule.end(); ++itlist)
        {
            if(itlist->rule_id.compare(ruleID)==0)
            {
                listRule.erase(itlist);
                break;
            }
        }
    }
    m_mutexRule.unlock();
}

bool AlarmManager::HasRule(const QString &cameraID, QString ruleID)
{
    RuleMap::iterator it = m_rules.find(cameraID);
    if(it != m_rules.end())
    {
        RuleList &listRule = it->second;
        for(RuleList::iterator itlist = listRule.begin(); itlist!=listRule.end(); ++itlist)
        {
            if(itlist->rule_id.compare(ruleID)==0)
            {
                return true;
            }
        }
    }

    return false;
}

bool AlarmManager::HasRule(const QString &cameraID, QString ruleID, mRule &rule)
{

    RuleMap::iterator it = m_rules.find(cameraID);
    if(it != m_rules.end())
    {
        RuleList &listRule = it->second;
        for(RuleList::iterator itlist = listRule.begin(); itlist!=listRule.end(); ++itlist)
        {
            if(itlist->rule_id.compare(ruleID)==0)
            {
                rule = *itlist;
                return true;
            }
        }
    }

    return false;
}

int AlarmManager::getRuleID(const QString &cameraID)
{
    RuleMap::iterator it = m_rules.find(cameraID);
    if(it != m_rules.end())
    {
        return it->second.size()+1;
    }

    return 1;
}

void AlarmManager::ReadConfigRuleFile(const vector<QString> &vecFile)
{
    m_configRules.clear();

    QString name;
    char buff[1024];
    for(size_t i=0; i<vecFile.size(); i++)
    {
        name = g_ConfigOpenPath+"/"+vecFile[i]+".cfg";

        ifstream ifs(name.toStdString().c_str());
        if(ifs.is_open())
        {
            RuleList listRule;
            while(!ifs.eof())
            {
                ifs.getline(buff, 1024);

                QString str(buff);
                QStringList strlist = str.split(';');

                mRule rule;
                if(strlist.size()>=11)
                {
                    rule.rule_name = strlist[0];
                    rule.rule_id = strlist[1];
                    rule.alarm_type = strlist[2].toInt();
                    rule.camera_id = strlist[3];
                    rule.line_type = (emLineType)strlist[4].toInt();
                    rule.alarm_person_min = strlist[5].toInt();
                    rule.alarm_person_max = strlist[6].toInt();
                    rule.alarm_delay=strlist[7].toInt();
                    rule.alarm_threshold = strlist[8].toInt();
                    rule.two_remind_time = strlist[9].toInt();
                    //
                    QString strtimes = strlist[10];
                    strtimes.replace(")","");
                    strtimes.replace("(","");
                    rule.times=strtimes.split(",");
                    //
                    QString strPoint = strlist[11];
                    QStringList strlistPoint=strPoint.split("),(");
                    for(int i=0;i<strlistPoint.size();i++)
                    {
                        QString pointstr=strlistPoint[i];
                        pointstr.replace(")","");
                        pointstr.replace("(","");
                        QStringList pointx_y=pointstr.split(",");
                        if(pointx_y.size()==2)
                        {
                            QPoint point;
                            point.setX(pointx_y[0].toInt());
                            point.setY(pointx_y[1].toInt());
                            rule.points.push_back(point);
                        }
                    }

                    listRule.push_back(rule);
                }

            }
            m_configRules[vecFile[i]] = listRule;

        }
    }
}

bool AlarmManager::SaveConfigRule(const mRule &rule)
{
    bool success = true;
    m_mutexConfigRule.lock();
    RuleMap::iterator it = m_configRules.find(rule.camera_id);

    RuleList listRule;
    if(it != m_configRules.end())
    {
        it->second.push_back(rule);

        listRule=it->second;
    }
    else
    {
        listRule.push_back(rule);

        m_configRules[rule.camera_id]=listRule;
    }

    if(!saveRuleFile(rule.camera_id, listRule))
    {
        success=false;
        listRule.pop_back();
    }

    m_mutexConfigRule.unlock();

    return success;
}

void AlarmManager::RemoveConfigRule(const QString &cameraID, const QString &ruleID)
{
    m_mutexConfigRule.lock();
    RuleMap::iterator it = m_configRules.find(cameraID);
    if(it != m_configRules.end())
    {
        bool success=false;
        RuleList &listRule = it->second;
        for(RuleList::iterator itlist = listRule.begin(); itlist!=listRule.end(); ++itlist)
        {
            if(itlist->rule_id.compare(ruleID)==0)
            {
                listRule.erase(itlist);
                success=true;
                break;
            }
        }

        if(success)
        {
            saveRuleFile(cameraID, listRule);
        }
    }

    m_mutexConfigRule.unlock();
}

bool AlarmManager::UpdateConfigRule(const mRule &rule)
{
    bool success=false;
    m_mutexConfigRule.lock();
    RuleMap::iterator it = m_configRules.find(rule.camera_id);
    if(it != m_configRules.end())
    {
        mRule curRule;
        RuleList &listRule = it->second;
        for(RuleList::iterator itlist = listRule.begin(); itlist!=listRule.end(); ++itlist)
        {
            if(itlist->rule_id.compare(rule.rule_id)==0)
            {
                curRule=*itlist;
                mRule &selfRule=*itlist;
                selfRule=rule;

                /*
                itlist->rule_name=rule.rule_name;
                itlist->alarm_type=rule.alarm_type;
                itlist->line_type=rule.line_type;
                itlist->alarm_person_min=rule.alarm_person_min;
                itlist->alarm_person_max=rule.alarm_person_max;
                itlist->alarm_delay=rule.alarm_delay;
                itlist->alarm_threshold=rule.alarm_threshold;
                itlist->two_remind_time=rule.alarm_threshold;
                itlist->times=rule.times;
                itlist->points=rule.points;
                */

                if(saveRuleFile(rule.camera_id, listRule))
                {
                    success=true;
                }
                else
                {
                    selfRule=curRule;
                }

                break;
            }
        }
    }

    m_mutexConfigRule.unlock();

    return success;
}

bool AlarmManager::HasConfigRule(const QString &cameraID, QString ruleID, mRule &rule)
{
    RuleMap::iterator it = m_configRules.find(cameraID);
    if(it != m_configRules.end())
    {
        RuleList &listRule = it->second;
        for(RuleList::iterator itlist = listRule.begin(); itlist!=listRule.end(); ++itlist)
        {
            if(itlist->rule_id.compare(ruleID)==0)
            {
                rule = *itlist;
                return true;
            }
        }
    }

    return false;
}

void AlarmManager::getConfigRulePoint(const QString &cameraID, map<QString, vector<QPoint>> &rulePoints)
{
    m_mutexConfigRule.lock();
    RuleMap::iterator it = m_configRules.find(cameraID);
    if(it!=m_configRules.end())
    {
        RuleList &ruleList = it->second;

        for(RuleList::iterator itlist = ruleList.begin(); itlist!=ruleList.end(); ++itlist)
        {
            rulePoints[IpcKey(cameraID, itlist->rule_id)] = itlist->points;
        }
    }    

    m_mutexConfigRule.unlock();
}

int AlarmManager::getConfigRuleID(const QString &cameraID)
{
    RuleMap::iterator it = m_configRules.find(cameraID);
    if(it != m_configRules.end())
    {
        return it->second.size()+1;
    }

    return 1;
}

bool AlarmManager::IsAlarmDiscorrect(const QString &ruleID, const QRect &head)
{
    AlarmDiscorrectMap::iterator it = m_alarmDiscorrects.find(ruleID);
    if(it!=m_alarmDiscorrects.end())
    {
        AlarmDiscorrectList &listAlarmDiscorrect = it->second;
        for(AlarmDiscorrectList::iterator itlist = listAlarmDiscorrect.begin(); itlist!=listAlarmDiscorrect.end(); ++itlist)
        {
            int ret=isFourPointInside(head, *itlist);
            if(ret==1 || ret==3)
            {
                return true;
            }
        }
    }

    return false;
}

void AlarmManager:: SaveAlarmDiscorrect(const QString &key, const vector<QPoint> &vecPoint)
{
    //TODO:最好限制10个
    m_mutexDiscorrect.lock();
    AlarmDiscorrectMap::iterator it = m_alarmDiscorrects.find(key);
    if(it!=m_alarmDiscorrects.end())
    {
        m_alarmDiscorrects[key].push_back(vecPoint);
    }
    else
    {
        AlarmDiscorrectList listAlarmDiscorrect;
        listAlarmDiscorrect.push_back(vecPoint);

        m_alarmDiscorrects[key]=listAlarmDiscorrect;
    }
    m_mutexDiscorrect.unlock();
}

void AlarmManager::ClearAlarmDiscorrect()
{
    m_mutexDiscorrect.lock();
    m_alarmDiscorrects.clear();
    m_mutexDiscorrect.unlock();
}

bool AlarmManager::isSingleRecord(const QString &ruleID, const QPoint &center, int nowtime)
{
    SingleRecordMap::iterator it = m_singleRecords.find(ruleID);
    if(it!=m_singleRecords.end())
    {
        SingleRecord &sr = it->second;

        if(center.x() >= sr.rect.x() && center.x() <= sr.rect.x()+sr.rect.width() &&
                center.y()>=sr.rect.y() && center.y()<=sr.rect.y()+sr.rect.height())
        {
            if(nowtime > sr.timestamp+ g_SingleRecordLoopSecond)
            {
                m_singleRecords.erase(it);
                return false;
            }

            if(sr.count>3)
            {
                return true;
            }
        }
    }

    return false;
}

void AlarmManager::alarmAnaly(const QString &path, const tagDetectAll &detectAll, const mRule &rule, int nowtime, const QTime &curTime)
{
    QString key = rule.rule_id;
    int alarmtime = getAlarmStamp(key);

    int nextAlarmTime = rule.two_remind_time+alarmtime;
    if(alarmtime==0)
    {
        nextAlarmTime=nowtime;
        saveAlarmStamp(key, nowtime-rule.two_remind_time);
    }

    //未到二次提醒时间
    /*
    if(nowtime < nextAlarmTime)
    {
        return;
    }
    */
    int analyAlarmTime = nowtime - rule.alarm_delay;

    //报警延时内图片人数
    QRect keyHead;//danrenduchu head point
    int personNum = getPersonNum(key, rule.alarm_type, rule.line_type, rule.points, detectAll, keyHead, nowtime);
    saveImageHead(key, personNum, nowtime);

    int imageNum=0;
    int headNum=0;
    getImageHead(key, analyAlarmTime, rule.alarm_person_min, rule.alarm_person_max, imageNum, headNum);


    int percent = 0;
    if(imageNum!=0)
    {
        percent = headNum*100/imageNum;
    }


    //haoshi 25ms
    drawBox(percent, path, rule.rule_name, rule.points);

    if(percent >= rule.alarm_threshold && nowtime >= (nextAlarmTime+rule.alarm_delay))
    {
        //dan ren wu bao
        if(rule.alarm_type==1)
        {
            bool hasRecord=false;
            SingleRecordMap::iterator it = m_singleRecords.find(rule.rule_id);
            if(it!=m_singleRecords.end())
            {
                QRect rect = it->second.rect;
                QPoint center = keyHead.center();

                if(center.x() >= rect.x() && center.x() <= rect.x()+rect.width() &&
                        center.y()>=rect.y() && center.y()<=rect.y()+rect.height())
                {

                    it->second.count += 1;
                    it->second.timestamp=nowtime;

                    hasRecord=true;
                }
            }

            if(!hasRecord)
            {
                SingleRecord sr;
                sr.rect=keyHead;
                sr.count=1;
                sr.timestamp=nowtime;
                m_singleRecords[rule.rule_id] = sr;
            }
        }


        //上传报警
        mAlarmRecord_stu record;
        record.cameraID=rule.camera_id;
        record.rule_id = rule.rule_id;
        record.alarm_id = "";
        record.alarmLevel=AL_REMIND;
        if(rule.alarm_delay > 0)
        {
            record.alarmLevel=AL_ALARM;
        }
        record.flag=1;

        record.filePath = path;
        record.alarmtime = curTime.toString("hh:mm:ss");
        record.head_points = QString("%1,%2,%3,%4").arg(keyHead.x()).arg(keyHead.y()).arg(keyHead.width()).arg(keyHead.height());

        PushAlarmRecord(record);
        saveAlarmStamp(key, nowtime);
        }

}

void AlarmManager::saveImageHead(const QString &ruleID, int headNum, int nowtime)
{
    tagImageHead imageHead;
    imageHead.headNum=headNum;
    imageHead.timestamp=nowtime;

    ImageHeadMap::iterator it = m_imageHeads.find(ruleID);
    if(it != m_imageHeads.end())
    {
        if(it->second.size() > IMAGE_HEAD_COUNT)
        {
            it->second.pop_front();
        }
        it->second.push_back(imageHead);
    }
    else
    {
        ImageHeadList listImageHead;
        listImageHead.push_back(imageHead);

        m_imageHeads[ruleID]  = listImageHead;
    }
}

void AlarmManager::getImageHead(const QString &ruleID, int afterTime, int minHeads, int maxHeads, int &imageNum, int &headNum)
{
    ImageHeadMap::iterator it = m_imageHeads.find(ruleID);
    if(it != m_imageHeads.end())
    {
        ImageHeadList &listImageHead = it->second;
        for(ImageHeadList::reverse_iterator itlist = listImageHead.rbegin(); itlist != listImageHead.rend(); ++itlist)
        {
            if(itlist->timestamp < afterTime)
            {
                break;
            }

            if(itlist->headNum >= minHeads && itlist->headNum <= maxHeads)
            {
                ++headNum;
            }

            ++imageNum;
        }
    }
}

//获取相机报警时间
int AlarmManager::getAlarmStamp(const QString &ruleID)
{
    int stamp=0;
    m_mutexAlarm.lock();
    AlarmStamp::iterator it = m_alarmStamps.find(ruleID);
    if(it != m_alarmStamps.end())
    {
        stamp = it->second;
    }
    m_mutexAlarm.unlock();

    return stamp;
}

void AlarmManager::saveAlarmStamp(const QString &ruleID, int timestamp)
{
    m_mutexAlarm.lock();
    m_alarmStamps[ruleID] = timestamp;
    m_mutexAlarm.unlock();
}

void AlarmManager::clearAlarmStamp(const QString &ruleID)
{
    m_mutexAlarm.lock();
    AlarmStamp::iterator it = m_alarmStamps.find(ruleID);
    if(it!=m_alarmStamps.end())
        m_alarmStamps.erase(it);
    m_mutexAlarm.unlock();
}

//4个点相对区域的位置  返回1：四个点都在区域内 2：四个点都在区域外 3：与外框相交
int AlarmManager::isFourPointInside(const QRect &head, const vector<QPoint> &vecRule)
{
    QPoint leftUpPoint(head.x(),head.y());
    QPoint leftDownPoint(head.x(), head.y()+head.height());
    QPoint rightUpPoint(head.x()+head.width(), head.y());
    QPoint rightDownPoint(head.x()+head.width(), head.y()+head.height());
    bool isleftUpPointIn=QJudgePosition::insidepolygon(leftUpPoint, vecRule);
    bool isleftDownPointIn=QJudgePosition::insidepolygon(leftDownPoint, vecRule);
    bool isrightUpPointIn=QJudgePosition::insidepolygon(rightUpPoint, vecRule);
    bool isrightDownPointIn=QJudgePosition::insidepolygon(rightDownPoint, vecRule);
    if(isleftUpPointIn&&isleftDownPointIn&&isrightUpPointIn&&isrightDownPointIn)
    {
        return 1;
    }
    else if(!isleftUpPointIn&&!isleftDownPointIn&&!isrightUpPointIn&&!isrightDownPointIn)
    {
        return 2;
    }
    else return 3;
}

int  AlarmManager::getPersonNum(const QString &ruleID, int alarm_type, int linetype, const vector<QPoint> &rects, const tagDetectAll &detectAll, QRect &keyHead, int nowtime)
{
    if(rects.size()==0) return 0;

    QVector<tagCaffeObject> vecCaffeObj=detectAll.heads;
    if(linetype==LT_BodyInSide)
    {
        vecCaffeObj=detectAll.bodys;
    }

    int num=0;    
    for(int i=0;i< vecCaffeObj.size();i++)
    {
        //是否單人獨處誤報
        if(alarm_type==1 && IsAlarmDiscorrect(ruleID, vecCaffeObj[i].rect)) continue;

        int ret=isFourPointInside(vecCaffeObj[i].rect, rects);

        if(linetype==LT_OutSide)
        {
            if(ret==2 || ret==3) ++num;
        }
        else
        {
            if(ret==1 || ret==3)
            {

                //单人独处
                if(alarm_type==1)
                {
                    //人头是否误报3次以上
                    if(num<2 && isSingleRecord(ruleID, vecCaffeObj[i].rect.center(), nowtime))
                    {
                        continue;
                    }

                    keyHead=vecCaffeObj[i].rect;
                }

                ++num;
            }
        }
    }

/*
    if(num==1 && alarm_type==1)
    {
        SingleRecordMap::iterator it = m_singleRecords.find(ruleID);
        if(it==m_singleRecords.end())
        {
            SingleRecord sr;
            sr.rect=keyHead;
            sr.count=1;
            sr.timestamp=nowtime;
            m_singleRecords[ruleID] = sr;
        }
    }
*/

    return num;
}

double AlarmManager::percenTage(QMap<int,pendImg> imgs,int minPerson,int maxPerson)
{
    double count= 0;
    int imgNum =1;
    double percen = 0;
    imgNum = imgs.size();
    QMap<int,pendImg>::iterator iter;
    for(iter=imgs.begin();iter!=imgs.end();iter++)
    {
         pendImg img =iter.value();
         int personCount=img.personNum;
         if(personCount >= minPerson &&personCount < maxPerson)
         {
           // if(img.hasBody)
            count=count+1;
//            else
//            count=count+0.5;
         }
    }
    percen = (double)count/(double)imgNum;
    return percen;
}

void AlarmManager::imgText(QString path,QString str,int w,int h,QColor color)
{
    QImage image(path);//=QPixmap(path).toImage();//绝对路径
       //以你原来的那张图片来
       //构造一个image
       //qDebug()<<image.width();

       QPainter painter(&image); //为这个QImage构造一个QPainter
       painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
       //设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
       // color = Qt::blue;
       //改变画笔和字体
       QPen pen = painter.pen();
       pen.setColor(color);
       QFont font = painter.font();
       font.setBold(false);//加粗
       font.setPixelSize(20);//改变字体大小

       painter.setPen(pen);
       painter.setFont(font);

       painter.drawText(w,h,str);

       image.save(path,0);
}

void AlarmManager::drawBox(int percen,const QString &imgPath, const QString &ruleName, const vector<QPoint> &rulePoints)
{
/*
    255 160 122
    255 165 0
    240 128 128
    255 20 147
    255 0 0
*/

    QString str = QString("%1:%2%").arg(percen).arg(ruleName);

    QColor color(0,255,0);
    if(10 < percen&& percen<= 20)
    {
         color.setRgb(255,210,210);
    }
    else if(20 < percen&& percen <= 40)
    {
         color.setRgb(255,180,180);
    }
    else if(40<percen&& percen<=60)
    {
        color.setRgb(255,120,120);
    }
    else if(60<percen&& percen<=80)
    {
        color.setRgb(255,60,60);
    }
    else if(80<percen&& percen<=100)
    {
        color.setRgb(255,0,0);
    }

    QImage image(imgPath);//=QPixmap(path).toImage();//绝对路径

    QPainter painter(&image); //为这个QImage构造一个QPainter
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    //设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    //1.
    //改变画笔和字体
    QPen pen = painter.pen();
    pen.setColor(color);
    pen.setWidth(2);
    QFont font = painter.font();
    font.setBold(false);//加粗
    font.setPixelSize(20);//改变字体大小

    painter.setPen(pen);
    painter.setFont(font);

    painter.drawText(rulePoints[0].x(), rulePoints[0].y(), str);

    //2.
    QPoint p1, p2;
    for(int i=0; i<rulePoints.size(); i++)
    {
        if(i==0)
        {
            p1=rulePoints[0];
            continue;
        }
        p2=rulePoints[i];

        painter.drawLine(p1, p2);
        p1=p2;
    }
    painter.drawLine(p1, rulePoints[0]);

    image.save(imgPath);
}

bool AlarmManager::saveRuleFile(const QString &ipcid, RuleList &listRule)
{
    bool success=true;
    QString filename= g_ConfigOpenPath+"/" + ipcid+".cfg";
    ofstream ofs(filename.toStdString().c_str(), ofstream::out);
    if(ofs.is_open())
    {
        for(RuleList::iterator it = listRule.begin(); it!=listRule.end(); ++it)
        {
            mRule rule = *it;
            //point:(1,2),(3,4)
            QString strpoint="";
            for(int i=0; i<rule.points.size(); i++)
            {
                if(i!=0)
                    strpoint+=",";

                strpoint+=QString("(%1,%2)").arg(rule.points[i].x()).arg(rule.points[i].y());
            }

            //time:00:00:01-12:00:01-1,
            QString strtime;
            for(int i=0; i<rule.times.size(); i++)
            {
                if(i!=0)
                    strtime+=",";

                strtime += rule.times[i];
            }

            ofs<<rule.rule_name.toStdString()<<";"<<rule.rule_id.toStdString()<<";"
              <<rule.alarm_type<<";"<<rule.camera_id.toStdString()<<";"
              <<rule.line_type<<";"<<rule.alarm_person_min<<";"<<rule.alarm_person_max<<";"
             <<rule.alarm_delay<<";"<<rule.alarm_threshold<<";"<<rule.two_remind_time<<";"
             <<strtime.toStdString()<<";"<<strpoint.toStdString()<<endl;
        }
    }
    else
    {
        success=false;
    }
    ofs.close();

    return success;
}
//
