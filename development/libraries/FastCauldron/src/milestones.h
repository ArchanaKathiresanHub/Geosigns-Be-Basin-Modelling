#ifndef _MILESTONES_H_
#define _MILESTONES_H_

#include <vector>
#include <functional>
class IsoLineState
{
public:
   IsoLineState():
	previousBottomSurfaceAge(0.0),
	previousBottomSurfaceTemperature(0.0),
	previousBottomSurfaceDepth(0.0),
	areBottomSurfacePreviousPropertiesInitialized(false)
{
}
 double previousBottomSurfaceAge;
 double previousBottomSurfaceTemperature;
 double previousBottomSurfaceDepth;
 bool areBottomSurfacePreviousPropertiesInitialized;
};
class IsolinePoint
{
public:
	IsolinePoint(const double &time, const double &depth, const double &value);
	~IsolinePoint();
	double getTime()const;
	double getDepth()const;
	double getValue()const;
private:
	double m_time;
	double m_depth;
	double m_value;
};
inline double IsolinePoint::getTime()const
{
	return m_time;
}
inline double IsolinePoint::getDepth()const
{
	return m_depth;
}
inline double IsolinePoint::getValue()const
{
	return m_value;
}

class LessThanIsolinePointPtrByTime:public std::binary_function<const IsolinePoint *,const IsolinePoint *, bool>
{
public:
	bool operator()(const IsolinePoint *lhs, const IsolinePoint *rhs) const
	{
		return lhs->getTime() < rhs->getTime();
	}
};
class LessThanIsolinePointPtrByValue:public std::binary_function<const IsolinePoint *,const IsolinePoint *, bool>
{
public:
	bool operator()(const IsolinePoint *lhs, const IsolinePoint *rhs) const
	{
		return lhs->getValue() < rhs->getValue();
	}
};
typedef std::vector<IsolinePoint*> IsoLinePointContainer;
typedef IsoLinePointContainer::iterator IsoLinePointContainerIt;
typedef IsoLinePointContainer::const_iterator IsoLinePointContainerConstIt;

namespace database
{
	class Table;
}

class IsoLineTable
{
public:
	IsoLineTable();
	~IsoLineTable();
	void clear();
	void addIsoLinePoint(IsolinePoint *thePoint);
	void writeToDatabaseTable( database::Table* IsoLineDatabaseTable );
protected:
	void Sort();
private:
	IsoLinePointContainer m_theData;
};





#endif
