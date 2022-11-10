//
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSurfaceTemperature.cpp
/// @brief Implementation for the parameter with the surface temperature at a particular age

#include "PrmSurfaceTemperature.h"

#include "cmbAPI.h"
#include "NumericFunctions.h"

namespace casa
{
using namespace std;

namespace
{
const std::string tableName = "SurfaceTempIoTbl";
const std::string ageColumnName = "Age";
const std::string temperatureColumnName = "Temperature";
const double epsilon = 1.e-6;
}

PrmSurfaceTemperature::PrmSurfaceTemperature(mbapi::Model& mdl, const std::vector<string>& parameters)
   : Parameter(),
     m_age{0},
     m_value{ Utilities::Numerical::IbsNoDataValue },
     m_tableRow{0}
{
   try
   {
      // The provide age should be one of the defined snapshots
      m_age = std::atof(parameters[0].c_str());

      if (!ageIsSnapshot(mdl, m_age))
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Age " << m_age << " is not a snapshot";
      }

      // The provide does not have to be in the temperature table.
      // If it is: read the value
      // If it is not: interpolate the value and add an entry to the table
      const int tableSize = mdl.tableSize( tableName );
      bool ageAtSnapShot = false;
      double rowAge = 0;
      int row = tableSize-1;
      double previousRowAge = 0;

      for (int j = 0; j<tableSize; ++j)
      {
         previousRowAge = rowAge;
         rowAge = mdl.tableValueAsDouble(tableName, j, ageColumnName);

         // Provided snapshot age is at snapshot defined in the table
         if ( std::fabs(rowAge - m_age) < epsilon )
         {
            ageAtSnapShot = true;
            row = j;
            break;
         }

         // Provide snapshot age is between the previous
         if ( m_age < rowAge)
         {
            ageAtSnapShot = false;
            row = j;
            break;
         }
      }
      if ( ageAtSnapShot )
      {
         m_value = mdl.tableValueAsDouble(tableName, row, temperatureColumnName);
         m_tableRow = row;
      }
      else
      {
         if (m_age > rowAge)
         {
            m_value = mdl.tableValueAsDouble(tableName, tableSize-1, temperatureColumnName);
            m_tableRow = tableSize;
         }
         else
         {
            const double value = mdl.tableValueAsDouble(tableName, row, temperatureColumnName);
            const double previous = mdl.tableValueAsDouble(tableName, row-1, temperatureColumnName);
            m_value = previous + (value - previous)*(m_age - previousRowAge)/(rowAge - previousRowAge);
            m_tableRow = row;
         }

         mdl.addRowToTable( tableName );
         mdl.setTableValue(tableName, tableSize, ageColumnName, m_age);
         mdl.setTableValue(tableName, tableSize, temperatureColumnName, m_value);
         mdl.tableSort(tableName, {ageColumnName});

         for (int k = 0; k<mdl.tableSize(tableName); ++k)
         {
            const double rowRage = mdl.tableValueAsDouble(tableName, k, ageColumnName);
            if (std::fabs(rowAge - m_age) < 1.e-6)
            {
               row = k;
               break;
            }
         }
      }
   }

   catch ( const ErrorHandler::Exception& e)
   {
      mdl.reportError( e.errorCode(), e.what() );
   }
}

PrmSurfaceTemperature::PrmSurfaceTemperature(const VarParameter* parent, const PrmSurfaceTemperature& param, const double val)
   : PrmSurfaceTemperature(param)
{
   setParent(parent);
   m_value = val;
}

ErrorHandler::ReturnCode PrmSurfaceTemperature::setInModel(mbapi::Model& caldModel, size_t /*caseID*/)
{
   return caldModel.setTableValue(tableName, m_tableRow, temperatureColumnName, m_value);
}

std::string PrmSurfaceTemperature::validate(mbapi::Model& caldModel)
{
   std::stringstream oss;

   if (!ageIsSnapshot(caldModel, m_age))
   {
      oss << "Age " << m_age << " is not a snapshot." << std::endl;
   }

   return oss.str();
}

std::vector<double> PrmSurfaceTemperature::asDoubleArray() const
{
   return {m_value};
}

bool PrmSurfaceTemperature::operator ==(const Parameter& prm) const
{
   const PrmSurfaceTemperature* pp = dynamic_cast<const PrmSurfaceTemperature*>(&prm);
   if ( !pp )
   {
      return false;
   }

   return NumericFunctions::isEqual(m_age, pp->m_age, epsilon) &&
         NumericFunctions::isEqual(m_value, pp->m_value, epsilon);
}

bool PrmSurfaceTemperature::save(CasaSerializer& sz) const
{
   bool ok = saveCommonPart(sz);
   ok = ok && sz.save( m_age, "SurfaceHistoryAge");
   ok = ok && sz.save( m_value, "SurfaceHistoryTemperature");
   ok = ok && sz.save( m_tableRow, "SurfaceHistoryRow");
   return ok;
}

PrmSurfaceTemperature::PrmSurfaceTemperature(CasaDeserializer& dz, unsigned int objVer) :
   Parameter( dz, objVer)
{
   bool ok = dz.load(m_age, "SurfaceHistoryAge");
   ok = ok && dz.load(m_value, "SurfaceHistoryTemperature");
   ok = ok && dz.load(m_tableRow, "SurfaceHistoryRow");

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "PrmSurfaceTemperature deserialization unknown error";
   }
}

void PrmSurfaceTemperature::loadVarPrm(CasaDeserializer& dz, const unsigned int objVer)
{
}

size_t PrmSurfaceTemperature::expectedParametersNumber()
{
   return 1;
}

size_t PrmSurfaceTemperature::optionalParametersNumber()
{
   return 0;
}

std::string PrmSurfaceTemperature::key()
{
   return "SurfaceTempIoTbl:Temperature";
}

std::string PrmSurfaceTemperature::varprmName()
{
   return "VarPrmSurfaceTemperature";
}

std::string PrmSurfaceTemperature::description()
{
   return "A variation of the surface temperature at a particular age";
}

std::string PrmSurfaceTemperature::fullDescription()
{
   std::ostringstream oss;

   oss << "    \"" << key() << "\" <age> <minVal> <maxVal> <prmPDF>\n";
   oss << "    Where:\n";
   oss << "       age           - age that is used as the parameter (has to be a major snapshot age)\n";
   oss << "       minVal     - the parameter minimal range scale factor value\n";
   oss << "       maxVal     - the parameter maximal range scale factor value\n";
   oss << "       prmPDF     - the parameter probability density function type, the value could be one of the following:\n";
   oss << "                \"Block\"    - uniform probability between min and max values,\n";
   oss << "                \"Triangle\" - triangle shape probability function. The top triangle value is taken from the base case\n";
   oss << "                \"Normal\"   - normal (or Gaussian) probability function. The position of highest value is taken from the base case\n";

   return oss.str();
}

std::string PrmSurfaceTemperature::helpExample(const char* cmdName)
{
   std::ostringstream oss;
   oss << "    #       VarPrmName             Age           min max  Parameter PDF\n";
   oss << "    " << cmdName << " \"" << key() << "\"  90 5   25   \"Block\"\n";
   return oss.str();
}

bool PrmSurfaceTemperature::ageIsSnapshot(mbapi::Model& mdl, const double age) const
{
   const mbapi::SnapshotManager & snapshotManager = mdl.snapshotManager();
   bool ageIsSnapshot = false;

   for (int i = 0; i < snapshotManager.snapshotsNumber() && !ageIsSnapshot; ++i)
   {
      const double ageSnapshot = snapshotManager.time(i);
      ageIsSnapshot = (std::fabs(age - ageSnapshot) < epsilon);
   }
   return ageIsSnapshot;
}

}
