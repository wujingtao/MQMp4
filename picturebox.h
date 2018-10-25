#ifndef PICTUREBOX_H
#define PICTUREBOX_H

#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <vector>
using namespace std;

class PictureBox : public QLabel
{
    Q_OBJECT
public:
    enum PB_MODE {FIXED_SIZE, FIX_SIZE_CENTRED, AUTO_ZOOM, AUTO_SIZE};
    explicit PictureBox(QWidget *parent = 0);
    ~PictureBox();

public:
    void reset();

   void loadRule(const QString &ipcid);
   void editRule(const QString &ipcID, const QString &ruleID);
   void setIsNewRule(bool isNew);
   bool endDrawLine();
   bool IsDraw();

   void lastDrawPoint();
   void nextDrawPoint();
   vector<QPoint> getDrawPoint();
   int getDrawPointSize();

   void clearRulePoint();

private:
    QPixmap m_pixmap;
    double m_scale;
    PB_MODE m_mode;
    QBrush m_brush;

protected:
    void mousePressEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent * event);

public slots:
    void setBackground(QBrush brush);

signals:
    void chooseRule(const QString &ipcID, const QString &ruleID);

private:
    bool m_isNewRule;
    bool m_isDDraw;

    QPoint m_nextPoint;
    vector<QPoint> m_vecDrawPoint;    
    QString m_pressKey;
    map<QString, vector<QPoint>> m_rulePoints;//key:ipcID_ruleID
};

#endif // PICTUREBOX_H
