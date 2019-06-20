#include "FracturePressureFunctionParameters.h"
#include "RunParameters.h"

#include "cauldronschemafuncs.h"

#include <assert.h>

using namespace database;

namespace DataAccess { namespace Interface {

FracturePressureFunctionParameters::FracturePressureFunctionParameters(
   ProjectHandle* projectHandle, database::Record* runOptionsIoTblRecord, 
   database::Record* pressureIoTblRecord):
   DAObject(projectHandle, pressureIoTblRecord),
   m_type(None)
{
   string function = database::getFractureType(runOptionsIoTblRecord);
   if (function == "" || function.find("None") == 0)
       m_type = None;
   else if (function.find("FunctionOfDepthBelowSeaLevel") == 0)
      m_type = FunctionOfDepthWrtSeaLevelSurface;
   else if (function.find("FunctionOfDepthBelowSedimentSurface") == 0)
      m_type = FunctionOfDepthWrtSedimentSurface;
   else if (function.find("FunctionOfLithostaticPressure") == 0)
      m_type = FunctionOfLithostaticPressure;
   else assert(0);

   m_typeName = function;
   m_name = database::getPresFuncName ( pressureIoTblRecord );

   if ( m_projectHandle->getRunParameters ()->getFractureModel () <= 0 or
        m_projectHandle->getRunParameters ()->getFractureModel () > int ( CONSERVATIVE_2 )) {
      m_fractureModel = NON_CONSERVATIVE_TOTAL;
   } else {
      m_fractureModel = FracturePressureModel ( m_projectHandle->getRunParameters ()->getFractureModel ());
   }

   if ( database::getSelected(m_record) == 1 and m_type != FunctionOfLithostaticPressure and m_type != None ) {
      m_coefficients.push_back ( database::getCoefA(m_record));
      m_coefficients.push_back ( database::getCoefB(m_record));
      m_coefficients.push_back ( database::getCoefC(m_record));
      m_coefficients.push_back ( database::getCoefD(m_record));
   }

}

FracturePressureFunctionParameters::FracturePressureFunctionParameters(
   ProjectHandle* projectHandle, database::Record* anyRecord):
   DAObject(projectHandle, anyRecord),
   m_type(None)
{}

FracturePressureFunctionType FracturePressureFunctionParameters::type() const
{
   return m_type;
}

FracturePressureModel FracturePressureFunctionParameters::getFractureModel () const {
   return m_fractureModel;
}


const std::vector<double>& FracturePressureFunctionParameters::coefficients() const
{
   assert(database::getSelected(m_record) == 1);
   assert(m_type != FunctionOfLithostaticPressure && m_type != None);

   return m_coefficients;
}

const std::string& FracturePressureFunctionParameters::getTypeName () const {
   return m_typeName;
}


const std::string& FracturePressureFunctionParameters::getName () const {
   return m_name;
}


} } // namespace DataAccess::Interface
