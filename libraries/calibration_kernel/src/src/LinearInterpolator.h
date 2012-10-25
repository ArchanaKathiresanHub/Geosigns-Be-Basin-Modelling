#ifndef LINEARINTERPOLATOR_H
#define LINEARINTERPOLATOR_H

class LinearInterpolator
{
public:
	LinearInterpolator(const double &startTime, const double &startValue, const double &endTime, const double &endValue);
	~LinearInterpolator();
	double evaluate(const double &time) const;
	
private:
	double m_coefficientA;
	double m_coefficientB;
};
inline double LinearInterpolator::evaluate(const double &time) const
{
		return (m_coefficientA + m_coefficientB * time);
}
#endif
