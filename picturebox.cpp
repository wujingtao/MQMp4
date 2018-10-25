#include "picturebox.h"
#include <QPainter>
#include <QDebug>
#include <qmessagebox.h>
#include <qjudgeposition.h>
#include "qmyglobal.h"
#include "AlarmManager.h"

static const int IMAGE_WIDTH = 160;
static const int IMAGE_HEIGHT = 120;
static const QSize IMAGE_SIZE = QSize(IMAGE_WIDTH, IMAGE_HEIGHT);

PictureBox::PictureBox(QWidget *parent) : QLabel(parent)
{
    m_pixmap = QPixmap(IMAGE_SIZE);
    m_pixmap.fill();
    m_scale = 1.0;
    m_mode = FIXED_SIZE;
    m_brush = QBrush(Qt::white);

    m_isNewRule=false;
    m_isDDraw=false;
    m_nextPoint.setX(0);
    m_nextPoint.setY(0);
}

void PictureBox::setBackground(QBrush brush)
{
    m_brush = brush;
    update();
}

void PictureBox::reset()
{
    m_isNewRule=false;
    m_isDDraw=false;
    m_vecDrawPoint.clear();

    update();
}

void PictureBox::clearRulePoint()
{
    m_rulePoints.clear();
    update();
}

void PictureBox::loadRule(const QString &ipcid)
{    
    //g_Offset_x，g_Offset_y初始化

    m_rulePoints.clear();
    ALARM_MANAGER->getConfigRulePoint(ipcid, m_rulePoints);

    for(map<QString, vector<QPoint>>::iterator it = m_rulePoints.begin(); it!=m_rulePoints.end(); ++it)
    {
        vector<QPoint> &vecPoint = it->second;
        for(size_t i=0; i<vecPoint.size(); i++)
        {
            vecPoint[i].setX(vecPoint[i].x()+g_Offset_x);
            vecPoint[i].setY(vecPoint[i].y()+g_Offset_y);
        }
    }

    update();
}

void PictureBox::editRule(const QString &ipcID, const QString &ruleID)
{
    setIsNewRule(true);

    QString key = IpcKey(ipcID, ruleID);
    map<QString, vector<QPoint>>::iterator it = m_rulePoints.find(key);
    if(it != m_rulePoints.end())
    {
        m_vecDrawPoint = it->second;

        m_rulePoints.erase(it);
    }

    update();
}

void PictureBox::setIsNewRule(bool isNew)
{
   m_isNewRule=isNew;
   m_isDDraw=isNew;
}

bool PictureBox::endDrawLine()
{
    //1.
    if(m_vecDrawPoint.size()<3)
    {
        QMessageBox::StandardButton button;
        button = QMessageBox::question(this, tr("是否结束画线"),
                                       QString(tr("您需要闭合线段，请继续描点画线")),
                                       QMessageBox::Yes | QMessageBox::No);

        return false;
    }

    //2.
    //未闭合的区域
    if(m_vecDrawPoint.size()>2)
    {
        double dis = QJudgePosition::disOfTwoPoints(m_vecDrawPoint.back(), m_vecDrawPoint.front());
        if(dis>20)
        {
            m_vecDrawPoint.push_back(m_vecDrawPoint.front());
        }
        else
        {
            m_vecDrawPoint.pop_back();
            m_vecDrawPoint.push_back(m_vecDrawPoint.front());
        }
    }

    m_isDDraw=false;
    update();
    return true;
}

bool PictureBox::IsDraw()
{
    return m_isDDraw;
}

void PictureBox::lastDrawPoint()
{
    if(m_vecDrawPoint.size()>0)
    {
        m_nextPoint = m_vecDrawPoint.back();
        m_vecDrawPoint.pop_back();

        update();
    }
}

void PictureBox::nextDrawPoint()
{
    if(m_nextPoint.x()!=0 && m_nextPoint.y()!=0)
    {
        m_vecDrawPoint.push_back(m_nextPoint);
        m_nextPoint.setX(0);
        m_nextPoint.setY(0);

        update();
    }
}

vector<QPoint> PictureBox::getDrawPoint()
{
    vector<QPoint> vecPoint;
    QPoint pt;
    for(size_t i=0; i<m_vecDrawPoint.size(); i++)
    {
        pt.setX(m_vecDrawPoint[i].x()-g_Offset_x);
        pt.setY(m_vecDrawPoint[i].y()-g_Offset_y);
        vecPoint.push_back(pt);
    }

    return vecPoint;
}

int PictureBox::getDrawPointSize()
{
    return m_vecDrawPoint.size()>0;
}

void PictureBox::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        bool success=false;
        m_pressKey="";

        QPoint pt = ev->pos();
        if(m_isDDraw)
        {
            m_vecDrawPoint.push_back(pt);
            success=true;
        }
        else if(!m_isNewRule)
        {
            for(map<QString, vector<QPoint>>::iterator it = m_rulePoints.begin(); it!=m_rulePoints.end(); ++it)
            {
                if( QJudgePosition::onPolygonLine(pt, it->second) )
                {
                    m_pressKey=it->first;
                    success=true;
                }
            }
        }

        if(success)
            update();
    }

}

void PictureBox::paintEvent(QPaintEvent * event)
{
    QLabel::paintEvent(event);


    QPainter painter(this);
    QPen pen = painter.pen();

    QColor color(255, 0, 0);
    pen.setColor(color);
    pen.setWidth(2);

    painter.setPen(pen);

    //1.line
    QPoint p1, p2;
    for(int i=0; i<m_vecDrawPoint.size(); i++)
    {
        if(i==0)
        {
            p1=m_vecDrawPoint[0];
            continue;
        }
        p2=m_vecDrawPoint[i];

        painter.drawLine(p1, p2);
        p1=p2;
    }

    //point
    pen.setColor(QColor(0, 0, 135));
    painter.setPen(pen);
    for(int i=0; i<m_vecDrawPoint.size(); i++)
    {
        painter.drawPoint(m_vecDrawPoint[i]);
    }

    //rulepoints
    //QColor lineColor(0,255,0);

    if(m_pressKey=="" && !m_isNewRule)
    {
        emit chooseRule("", "");
    }

    for(map<QString, vector<QPoint>>::iterator it = m_rulePoints.begin(); it!=m_rulePoints.end(); ++it)
    {
        vector<QPoint> vecPoint = it->second;

        if(it->first == m_pressKey)
        {
            pen.setColor(QColor(255, 0, 0));

            QString ipcid;
            QString ruleid;
            getIpcKey(it->first, ipcid, ruleid);

            emit chooseRule(ipcid, ruleid);
        }
        else
        {
            pen.setColor(QColor(0, 255, 0));
        }

        painter.setPen(pen);
        for(int i=0; i<vecPoint.size(); i++)
        {
            if(i==0)
            {
                p1=vecPoint[0];
                continue;
            }

            p2=vecPoint[i];

            painter.drawLine(p1, p2);
            p1=p2;
        }

    }
}

PictureBox::~PictureBox()
{

}
