#ifndef QJUDGEPOSITION_H
#define QJUDGEPOSITION_H
#include <QPoint>

#include <vector>
using namespace std;
class QJudgePosition
{
public:
    QJudgePosition();

   static bool online(QPoint point1,QPoint point2,QPoint point3);
   static bool onPolygonLine(QPoint p,vector<QPoint> points);
   static bool insidepolygon(QPoint point,vector<QPoint> points);
   static double disOfTwoPoints(QPoint a,QPoint b);
};

#endif // QJUDGEPOSITION_H
