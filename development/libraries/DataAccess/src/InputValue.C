#if !(defined (_WIN32) || defined (_WIN64))
#include <unistd.h>
#endif
#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi


#include <string>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/InputValue.h"
#include "Interface/ProjectHandle.h"
#include "Interface/GridMap.h"

#include "hdf5funcs.h"

#define HDFFILENAME	"InputMap%d.HDF"

static bool copyFile (const string & inputFileName, const string & outputFileName);

using namespace DataAccess;
using namespace Interface;

InputValue::InputValue (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record), m_index (0)
{
   m_eventAge = -1;
   m_propertyName = "";
   m_surfaceName = "";
   m_formationName = "";
   m_reservoirName = "";
}

InputValue::~InputValue (void)
{
}

/// Return the type of the InputValue
InputValueType InputValue::getType (void) const
{
   const string & mapType = getMapType ();

   if (mapType == "HDF5") return PropertyMap;
   else if (mapType == "DECBINARY") return PropertyMap;
   else if (mapType == "DECASCII") return PropertyMap;
   else if (mapType == "XYZ") return PropertyMap;
   else if (mapType == "ZYCOR") return PropertyMap;
   else if (mapType == "CPS3") return PropertyMap;
   else if (mapType == "EPIRUS") return PropertyMap;
   else if (mapType == "LANDMARK") return PropertyMap;
   else if (mapType == "CHARISMA") return PropertyMap;
   else return FaultMap;

}

/// Return the format identifier of the file
const string & InputValue::getMapType (void) const
{
   return database::getMapType (m_record);
}

/// Return the name of the file from which the InputValue was read
const string & InputValue::getFileName (void) const
{
   return getMapFileName (m_record);
}

const string & InputValue::getReferringTableName (void) const
{
   return getReferredBy (m_record);
}

const string & InputValue::getMapName (void) const
{
   return database::getMapName (m_record);
}

const GridMap * InputValue::getGridMap (void) const
{
   if (getChild (ValueMap) == 0)
   {
      loadGridMap ();
   }
   return (GridMap *) getChild (ValueMap);
}

/// Get the age at which the InputValue started to make an impact
double InputValue::getEventAge (void) const
{
   fillEventAttributes ();
   return m_eventAge;
}

/// Get the name of the property associated with this InputValue
const string & InputValue::getPropertyName (void) const
{
   fillEventAttributes ();
   return m_propertyName;
}

/// Get the name of the Surface associated with this InputValue
const string & InputValue::getSurfaceName (void) const
{
   fillEventAttributes ();
   return m_surfaceName;
}

/// Get the name of the Formation associated with this InputValue
const string & InputValue::getFormationName (void) const
{
   fillEventAttributes ();
   return m_formationName;
}

/// Get the name of the Reservoir associated with this InputValue
const string & InputValue::getReservoirName (void) const
{
   fillEventAttributes ();
   return m_reservoirName;
}

bool InputValue::fillEventAttributes (void) const
{
   if (m_eventAge >= 0) return true;
   m_eventAge = 0;

   const string & tblName = getReferredBy (m_record);

   Table * tbl = m_projectHandle->getTable (tblName);
   if (tbl)
   {
      if (tblName == "StratIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getDepthGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Depth";
	       m_surfaceName = database::getSurfaceName (record);
	       m_eventAge = getDepoAge (record);
	       return true;
	    }
	    else if (getThicknessGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Thickness";
	       m_formationName = database::getLayerName (record);
	       m_eventAge = getDepoAge (record);
	       return true;
	    }
	    else if (getPercent1Grid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "litho1 %";
	       m_formationName = database::getLayerName (record);
	       m_eventAge = getDepoAge (record);
	       return true;
	    }
	    else if (getPercent2Grid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "litho2 %";
	       m_formationName = database::getLayerName (record);
	       m_eventAge = getDepoAge (record);
	       return true;
	    }
	 }
      }
      else if (tblName == "MobLayThicknIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getThicknessGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "ML Thickness";
	       m_formationName = database::getLayerName (record);
	       m_eventAge = getAge (record);
	       return true;
	    }
	 }
      }
      else if (tblName == "CrustIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getThicknessGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Thickness";
	       m_formationName = "Crust";
	       m_eventAge = getAge (record);
	       return true;
	    }
	 }
      }
      else if (tblName == "BoundaryValuesIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getPropertyValueGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "BoundaryValue";
	       m_eventAge = getBeginTimeValues (record);
	       return true;
	    }
	 }
      }
      else if (tblName == "SurfaceTempIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getTemperatureGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Temperature";
	       m_surfaceName = "Surface";
	       m_eventAge = getAge (record);
	       return true;
	    }
	 }
      }
      else if (tblName == "SurfaceDepthIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getDepthGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Depth";
	       m_surfaceName = "Surface";
	       m_eventAge = getAge (record);
	       return true;
	    }
	 }
      }
      else if (tblName == "MntlHeatFlowIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getHeatFlowGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "HeatFlow";
	       m_formationName = "Mantle";
	       m_eventAge = getAge (record);
	       return true;
	    }
	 }
      }
      else if (tblName == "BasementIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getTopCrustHeatProdGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Heat Flux";
	       m_surfaceName = "Top of Crust";
	       m_eventAge = 0;
	       return true;
	    }
	 }
      }
      else if (tblName == "ReservoirIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getDepthOffsetGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Depth Offset";
	       m_reservoirName = database::getReservoirName (record);
	       m_eventAge = 0;
	       return true;
	    }
	    else if (getThicknessGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Thickness";
	       m_formationName = database::getFormationName (record);
	       m_reservoirName = database::getReservoirName (record);
	       m_eventAge = 0;
	       return true;
	    }
	    else if (getLayerFrequencyGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Layer Frequency";
	       m_reservoirName = database::getReservoirName (record);
	       m_eventAge = 0;
	       return true;
	    }
	    else if (getNetToGrossGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "NetToGross";
	       m_reservoirName = database::getReservoirName (record);
	       m_eventAge = 0;
	       return true;
	    }
	 }
      }
      else if (tblName == "PalinspasticIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getDepthGrid (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "High Res. Depth";
	       m_surfaceName = database::getSurfaceName (record);
	       m_eventAge = 0;
	       return true;
	    }
	    else if (getFaultcutsMap (record) == database::getMapName (m_record))
	    {
	       m_propertyName = "Fault Cuts";
	       m_surfaceName = database::getSurfaceName (record);
	       m_eventAge = 0;
	       return true;
	    }
	 }
      }
      else if (tblName == "SourceRockLithoIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 const string gridNames[] = {
	    "TocIni", "S1Ini", "S2Ini", "S3Ini", "HcIni", "OcIni", "NcIni", "CharLength", 
	    "UpperBiot", "LowerBiot", "PreAsphaltStartAct", "PreAsphaltEndAct", 
	    "NettThickIni", "NGenexTimeSteps", "NGenexSlices", ""
	 };

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    int i;
	    for (i = 0; gridNames[i] != ""; ++i)
	    {
	       string fullGridName = gridNames[i] + "Grid";
	       if (record->getValue<std::string>(fullGridName) == database::getMapName (m_record))
	       {
		  m_propertyName = gridNames[i];
		  m_formationName = database::getLayerName (record);
		  m_eventAge = 0;
		  return true;
	       }
	    }
	 }
      }
      
		else if (tblName == "TouchstoneMapIoTbl")
      {
	 Table::iterator tblIter;
	 Record * record = 0;

	 for (tblIter = tbl->begin (); tblIter != tbl->end (); ++tblIter)
	 {
	    record = * tblIter;
	    if (getFaciesMap ( record ) == database::getMapName (m_record))
		{
	       m_propertyName = "FaciesIndex";
	       m_formationName = database::getFormationName (record);   
	       m_surfaceName = database::getSurfaceName (record);   
	       return true;
		}    
	 }
      }      
   }
   return false;
}

unsigned int InputValue::computeIndex ()
{
   const string & hdf5FileName = getHDF5FileName (m_record);
   if (hdf5FileName.size () != 0)
   {
      unsigned int index;
      sscanf (hdf5FileName.c_str (), HDFFILENAME, &index);
      return index;
   }
   return 0;
}

unsigned int InputValue::applyIndex (unsigned int newIndex)
{
   const string & hdf5FileName = getHDF5FileName (m_record);
   if (hdf5FileName.size () != 0)
   {
      unsigned int index;
      sscanf (hdf5FileName.c_str (), HDFFILENAME, &index);
      setIndex (index);
      return index;
   }
   else
   {
      setIndex (newIndex);
      return newIndex;
   }
}

string InputValue::saveToDirectory (const string & directory)
{
   if (getType () == PropertyMap)
   {
      char indexStr[10];
      sprintf (indexStr, "%d", getIndex ());

      GridMap * gridMap = (GridMap *) getGridMap ();
      string fileName = string ("InputMap") + indexStr + ".HDF";
      string fullFileName = directory + '/' + fileName;
      gridMap->saveHDF5 (fullFileName);

      return fileName;
   }
   else
   {
      string command;
      string inputFileName = m_projectHandle->getProjectPath () + "/" + getFileName ();
      string outputFileName = directory + "/" + getFileName ();

      if (!copyFile (inputFileName, outputFileName))
      {
	 cerr << "Copying " << inputFileName << " to " << outputFileName << " Failed" << endl;
	 return string ("");
      }
      else
      {
	 return getFileName ();
      }
   }
}

int InputValue::getMapSequenceNumber () const {
   return getMapSeqNbr (m_record);
}


GridMap * InputValue::loadGridMap (void) const
{
   assert (m_record);
   const string & hdf5FileName = getHDF5FileName (m_record);

   if (getMapType () == "HDF5")
   {
      string mapFileName = m_projectHandle->getProjectPath () + "/" + getMapFileName (m_record);
      return m_projectHandle->loadGridMap (this, ValueMap, mapFileName, HDF5::findLayerName (mapFileName, getMapSeqNbr (m_record)));
   }
   else if (hdf5FileName != "")
   {
      string mapFileName = m_projectHandle->getFullOutputDir () + "/" + hdf5FileName;
      return m_projectHandle->loadGridMap (this, ValueMap, mapFileName, HDF5::findLayerName (mapFileName, 0));
   }
   else
   {
      return 0;
   }
}

void InputValue::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void InputValue::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "MappedInputValue:";
   buf << " ReferredBy = " << getReferredBy (m_record);
   buf << ", MapName = " << database::getMapName (m_record);
   buf << ", MapType = " << getMapType ();
   buf << ", MapFileName = " << getMapFileName (m_record);
   buf << ", FileId = " << getFileId (m_record);
   buf << ", MapSeqNbr = " << getMapSeqNbr (m_record);
   buf << ", HDF5FileName = " << getHDF5FileName (m_record);
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}

static bool copyFile (const string & inputFileName, const string & outputFileName)
{
   if (outputFileName.length () == 0)
      return false;

   ofstream outputStream;

   outputStream.open (outputFileName.c_str (), ios::out);

   if (outputStream.fail ())
   {
      cerr << "Error occurred during opening output file " << outputFileName << endl;
      return false;
   }

   if (inputFileName.length () == 0)
   {
      outputStream.close ();
      return false;
   }

   ifstream inputStream;

   inputStream.open (inputFileName.c_str (), ios::in);

   if (inputStream.fail ())
   {
      cerr << "Error occurred during opening input file " << inputFileName << endl;

      outputStream.close ();
      return false;
   }

   inputStream.seekg (0, ifstream::end);

   long size = inputStream.tellg ();

   inputStream.seekg (0);

   char * buffer = new char[size];

   inputStream.read (buffer, size);

   outputStream.write (buffer, size);

   delete[] buffer;

   outputStream.close ();
   inputStream.close ();

   return true;
}
