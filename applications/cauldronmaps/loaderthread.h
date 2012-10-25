#ifndef _LOADERTHREAD_H
#define _LOADERTHREAD_H 1

#ifdef QT_THREAD_SUPPORT

#include <qthread.h>

class InfoBase;

class LoaderThread: public QThread
{
public:
    LoaderThread (InfoBase * infoBase, unsigned int whatToLoad = 0);

    virtual void run ();

private:
    InfoBase * m_infoBase;
    unsigned int m_whatToLoad;
};

#endif
#endif
