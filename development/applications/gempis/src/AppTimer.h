#ifndef APPTIMER_H
#define APPTIMER_H _APPTIMER_H

#include <qobject.h>

class AppTimer :
        public QObject
{

    Q_OBJECT

    public: 
        AppTimer( QObject *, char * );
        ~AppTimer();

	void Set( long int );
	void Start( void );
	void Stop( void );

    public slots:

    private:
        QObject * m_receiver;
        char * m_member;
	long m_duration;    // in milliseconds

	QTimer *m_internalTimer;

    private slots:
    
    protected:
};

#endif
