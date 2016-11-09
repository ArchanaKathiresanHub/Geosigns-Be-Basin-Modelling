#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;


#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"
#include "Interface/SourceRock.h"
#include "Interface/ObjectFactory.h"

using namespace DataAccess;
using namespace Interface;

SourceRock::SourceRock (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_layerName = "";
}

SourceRock::~SourceRock (void)
{
}

const string SourceRock::s_MapAttributeNames[] =
{
   "TocIni", "S1Ini", "S2Ini", "S3Ini", "HcIni", "OcIni", "NcIni", "CharLength", "UpperBiot", "LowerBiot",
   "KerogenStartAct", "KerogenEndAct", "PreAsphaltStartAct", "PreAsphaltEndAct", "NettThickIni",
   "NGenexTimeSteps", "NGenexSlices", "VREini", "AsphalteneDiffusionEnergy", "ResinDiffusionEnergy", "C15AroDiffusionEnergy", "C15SatDiffusionEnergy"
};

const string & SourceRock::getType (void) const
{
   return database::getSourceRockType (m_record);

}
const string & SourceRock::getLayerName (void) const
{
   if ( database::getLayerName (m_record) != "" ) {
      return database::getLayerName (m_record);
   } else {
      assert ( m_layerName != "" );
      return m_layerName;
   }

}

void SourceRock::setLayerName ( const std::string& name ) {
   m_layerName = name;
}

//mademlis
const double & SourceRock::getHcVRe05(void) const
{
   return database::getHcVRe05 (m_record);
}
const double & SourceRock::getScVRe05(void) const
{
   return database::getScVRe05 (m_record);
}
const double & SourceRock::getPreAsphaltStartAct(void) const
{
   return database::getPreAsphaltStartAct (m_record);
}
const double & SourceRock::getVREini(void) const
{
   return database::getVREini (m_record);
}
const double & SourceRock::getAsphalteneDiffusionEnergy(void) const
{
   return database::getAsphalteneDiffusionEnergy (m_record);
}
const double & SourceRock::getResinDiffusionEnergy(void) const
{
   return database::getResinDiffusionEnergy (m_record);
}
const double & SourceRock::getC15AroDiffusionEnergy(void) const
{
   return database::getC15AroDiffusionEnergy (m_record);
}
const double & SourceRock::getC15SatDiffusionEnergy(void) const
{
   return database::getC15SatDiffusionEnergy (m_record);
}
const string & SourceRock::getBaseSourceRockType (void) const
{
   return database::getBaseSourceRockType (m_record);
}

///Return true if the VREoptimization flag has been set to Yes
bool SourceRock::isVREoptimEnabled(void) const
{
   bool ret = true;
   const string & dataBaseValue = database::getVREoptimization (m_record);
   if(dataBaseValue != "Yes")
   {
       ret = false;
   }
   return ret;
}        
///Return the value of the VRE threshold
const double & SourceRock::getVREthreshold(void) const
{
   return database::getVREthreshold(m_record);
}

bool SourceRock::isVESMaxEnabled(void) const
{
   bool ret = true;
   const string & dataBaseValue = database::getVESLimitIndicator(m_record);
   if(dataBaseValue != "Yes")
   {
       ret = false;
   }
   return ret;
}        
const double & SourceRock::getVESMax(void) const
{
   return database::getVESLimit(m_record);
}

// shale gas related functions
/// Whether to perform adsorption
bool SourceRock::doApplyAdsorption (void) const
{
   return (database::getApplyAdsorption(m_record) == 1);
}

/// Whether to use a TOC-dependent adsorption capacity function
bool SourceRock::adsorptionIsTOCDependent (void) const
{
   return (database::getAdsorptionTOCDependent (m_record) == 1);
}

/// Whether to (also) perform OTGC during adsorption
bool SourceRock::doComputeOTGC (void) const
{
   return (database::getComputeOTGC (m_record) == 1);
}

/// get the name of the adsorption function
const string & SourceRock::getAdsorptionCapacityFunctionName (void) const
{
   return database::getAdsorptionCapacityFunctionName (m_record);
}

/// get the adsorption simulator
const string & SourceRock::getAdsorptionSimulatorName (void) const
{
   return database::getWhichAdsorptionSimulator (m_record);
}


/// Return the (GridMap) value of one of this SourceRock's attributes
const GridMap * SourceRock::getMap (SourceRockMapAttributeId attributeId) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;
   GridMap * gridMap;

   if ((gridMap = (GridMap *) getChild (attributeIndex)) == 0)
   {
      gridMap = loadMap (attributeId);
   }
   return gridMap;
}
 
GridMap * SourceRock::loadMap (SourceRockMapAttributeId attributeId) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;

   string attributeGridName = s_MapAttributeNames[attributeIndex] + "Grid";
   const string & valueGridMapId = m_record->getValue<std::string>(attributeGridName);

   GridMap * gridMap = 0;
   if (valueGridMapId.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("SourceRockLithoIoTbl", valueGridMapId);
   }
   else
   {
      double value = m_record->getValue<double>(s_MapAttributeNames[attributeIndex]);
      if ( value != RecordValueUndefined)
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

void SourceRock::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void SourceRock::asString (string & str) const
{
   ostringstream buf;

   buf << "SourceRock:";
   buf << " Type = " << getType ();
   buf << " , BaseType = " << getBaseSourceRockType ();
   buf << endl;
   
   str = buf.str ();

}
