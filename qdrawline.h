#ifndef QDRAWLINE_H
#define QDRAWLINE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QList>
#include <QPoint>
#include <QImage>
class qdrawline
{
public:
    qdrawline();

    static void drawLine(QVector<QPoint> pointlist,QImage &img,QColor pointColor,QColor lineColor);                //画线函数
};

#endif // QDRAWLINE_H
