#include <codetimer.h>

CodeTimer::CodeTimer()
{
#ifdef _WIN32
	m_start.QuadPart = 0;
	m_stop.QuadPart = 0;
	QueryPerformanceFrequency( &m_freq );
#else

#endif
}

void CodeTimer::Start()
{
#ifdef _WIN32
	QueryPerformanceCounter(&m_start);
#else
	struct timezone tz;
	gettimeofday(&m_start, &tz);
#endif
}

void CodeTimer::Stop()
{
#ifdef _WIN32
	QueryPerformanceCounter(&m_stop);
#else
	struct timezone tz;
	gettimeofday(&m_stop, &tz);
#endif
}

double CodeTimer::GetElapseTimeMS()
{
#ifdef _WIN32
	CTINTEGER ticks;
	ticks.QuadPart = (m_stop.QuadPart - m_start.QuadPart) * 1000/m_freq.QuadPart;
	return (double)ticks.QuadPart;
#else
	double t1, t2;
	t1 =  (double)m_start.tv_sec + (double)m_start.tv_usec/(1000*1000);
	t2 =  (double)m_stop.tv_sec + (double)m_stop.tv_usec/(1000*1000);
	return (t2-t1)*1000;
#endif
}