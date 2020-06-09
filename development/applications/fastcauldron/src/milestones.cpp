#include "milestones.h"

#include <algorithm>
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"


IsolinePoint::IsolinePoint(const double &time, const double &depth, const double &value):
m_time(time),
m_depth(depth),
m_value(value)
{

}
IsolinePoint::~IsolinePoint()
{

}

IsoLineTable::IsoLineTable()
{
}
IsoLineTable::~IsoLineTable()
{
	clear();
}
void IsoLineTable::clear()
{
	IsoLinePointContainerIt it = m_theData.begin();
	IsoLinePointContainerIt itEnd = m_theData.end();
	while(it != itEnd)
	{
		delete (*(it++));
	}
	m_theData.clear();
}
void IsoLineTable::addIsoLinePoint(IsolinePoint *thePoint)
{
	m_theData.push_back(thePoint);
}
void IsoLineTable::writeToDatabaseTable( database::Table* IsoLineDatabaseTable )
{
	Sort();

	IsoLineDatabaseTable->clear();

	IsoLinePointContainerIt it = m_theData.begin();
	IsoLinePointContainerIt itEnd = m_theData.end();

	while(it != itEnd)
	{
		database::Record* newRecord = IsoLineDatabaseTable->createRecord ();
		database::setAge (newRecord, (*it)->getTime());
   	database::setContourValue (newRecord, (*it)->getValue());
   	database::setSum (newRecord, (*it)->getDepth());
   	database::setNP (newRecord, 1);
		++it;
	}
}
void IsoLineTable::Sort()
{
	if( false == m_theData.empty() )
	{
		LessThanIsolinePointPtrByValue valuePredicate;//primary sorting key
		
		//sort the entire container by contour value
		std::sort(m_theData.begin(), m_theData.end(), valuePredicate);
			
		LessThanIsolinePointPtrByTime timePredicate;//secondary sorting key

		IsoLinePointContainerIt itSortBegin = m_theData.begin();
		IsoLinePointContainerIt itSortEnd   = m_theData.begin();

		while( itSortEnd != m_theData.end() )
		{
			//find the end of the sub-range
			itSortEnd = std::find_if(m_theData.begin(), m_theData.end(), bind1st( valuePredicate, (*itSortBegin) ) );
			//sort the sub-range by time
			std::sort(itSortBegin, itSortEnd, timePredicate);
			//set the beginning of the next sub-range
			itSortBegin = itSortEnd;
		}
	}
}
