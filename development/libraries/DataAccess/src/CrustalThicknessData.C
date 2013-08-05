#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;
#define USESTANDARD

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"
#include "Interface/CrustalThicknessData.h"
#include "Interface/ObjectFactory.h"

using namespace DataAccess;
using namespace Interface;


const string CrustalThicknessData::s_MapAttributeNames[] =
{
   "T0Ini", "TRIni", "HCuIni", "HLMuIni", "HBu", "DeltaSL"
};

CrustalThicknessData::CrustalThicknessData (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
}


CrustalThicknessData::~CrustalThicknessData (void)
{
}

const double & CrustalThicknessData::getT0Ini(void) const
{
   return database::getT0Ini (m_record);
}

const double & CrustalThicknessData::getTRIni(void) const
{
   return database::getTRIni (m_record);
}

const double & CrustalThicknessData::getHCuIni(void) const
{
   return database::getHCuIni (m_record);
}

const double & CrustalThicknessData::getHLMuIni(void) const
{
   return database::getHLMuIni (m_record);
}

const double & CrustalThicknessData::getHBu(void) const
{
   return database::getHBu (m_record);
}

const int & CrustalThicknessData::getFilterHalfWidth(void) const
{
   return database::getFilterHalfWidth (m_record);
}

const double & CrustalThicknessData::getDeltaSL(void) const
{
   return database::getDeltaSL (m_record);
}

const string & CrustalThicknessData::getSurfaceName(void) const
{
   return database::getSurfaceName (m_record);
}

/// Return the (GridMap) value of one of this SourceRock's attributes
const Interface::GridMap * CrustalThicknessData::getMap (Interface::CTCMapAttributeId attributeId) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;
   GridMap * gridMap;

   if ((gridMap = (GridMap *) getChild (attributeIndex)) == 0)
   {
      gridMap = loadMap (attributeId);
   }
   return gridMap;
}
 
GridMap * CrustalThicknessData::loadMap (Interface::CTCMapAttributeId attributeId) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;

   string attributeGridName = s_MapAttributeNames[attributeIndex] + "Grid";
   const string & valueGridMapId = m_record->getValue (attributeGridName, (string *) 0);

   GridMap * gridMap = 0;
   if (valueGridMapId.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("CTCIoTbl", valueGridMapId);
   }
   else
   {
      double value;
      if ((value = m_record->getValue (s_MapAttributeNames[attributeIndex], (double *) 0)) != RecordValueUndefined)
      {
         //const Grid *grid = m_projectHandle->getInputGrid ();
	 const Grid * grid = m_projectHandle->getActivityOutputGrid();
	 if (!grid) grid = (const Grid *) m_projectHandle->getInputGrid ();
         gridMap = m_projectHandle->getFactory ()->produceGridMap (this, attributeIndex, grid, value);

         assert (gridMap == getChild (attributeIndex));
      }
   }
   return gridMap;
}

void CrustalThicknessData::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void CrustalThicknessData::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "Crustul Thickness Data:";
   buf << " t0 = " << getT0Ini();
   buf << ", tr = " << getTRIni ();
   buf << endl;
   
   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif

}
