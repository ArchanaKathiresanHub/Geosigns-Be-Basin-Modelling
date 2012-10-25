#ifndef CODETIMER_H
#define CODETIMER_H

#ifdef _WIN32
#include <windows.h>
typedef LARGE_INTEGER CTINTEGER;
#else
#include <sys/time.h>
#include <time.h>
typedef struct timeval CTINTEGER;
#endif

class CodeTimer {
private:
	CTINTEGER m_freq;
	CTINTEGER m_start;
	CTINTEGER m_stop;
public:
	CodeTimer();
	void Start();
	void Stop();
	double GetElapseTimeMS();
	//double GetElapseTimeSeconds();
};


#endif 