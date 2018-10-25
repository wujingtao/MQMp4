#ifndef MP4SERVER_H
#define MP4SERVER_H

#include <qthread.h>
#include <qstring.h>
#include <vector>
using namespace std;

class Mp4Server : public QThread
{
public:
    Mp4Server(const vector<QString> vecRtsp);

public:
    void run();

    bool IsRun();
    int getLeaveCount();

private:
    vector<QString> m_vecRtsp;

    bool m_isRun;
    int m_leaveCount;
};

#endif // MP4SERVER_H
