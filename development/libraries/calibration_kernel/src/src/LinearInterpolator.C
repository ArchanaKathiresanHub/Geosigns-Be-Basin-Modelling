#include "LinearInterpolator.h"

LinearInterpolator::LinearInterpolator(const double &startTime, const double &startValue, const double &endTime, const double &endValue)
{
	double DT = endTime - startTime;
	m_coefficientA = endTime * startValue / DT - startTime * endValue / DT;
	m_coefficientB = (endValue - startValue) / DT;
}
LinearInterpolator::~LinearInterpolator()
{

}
