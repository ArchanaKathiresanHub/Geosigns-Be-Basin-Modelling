#include "FracturePressureFunctionParameters.h"
#include "RunParameters.h"

#include "cauldronschemafuncs.h"
#include "LogHandler.h"

#include <assert.h>

using namespace database;

namespace DataAccess { namespace Interface {

FracturePressureFunctionParameters::FracturePressureFunctionParameters(
   ProjectHandle& projectHandle, database::Record* runOptionsIoTblRecord,
   database::Record* pressureIoTblRecord):
   DAObject(projectHandle, pressureIoTblRecord),
   m_type(FracturePressureFunctionType::None), m_name("")
{
   m_typeName = database::getFractureType(runOptionsIoTblRecord);
   if (m_typeName == "" || m_typeName.find("None") == 0)
       m_type = FracturePressureFunctionType::None;
   else if (m_typeName.find("FunctionOfDepthBelowSeaLevel") == 0)
      m_type = FracturePressureFunctionType::FunctionOfDepthWrtSeaLevelSurface;
   else if (m_typeName.find("FunctionOfDepthBelowSedimentSurface") == 0)
      m_type = FracturePressureFunctionType::FunctionOfDepthWrtSedimentSurface;
   else if (m_typeName.find("FunctionOfLithostaticPressure") == 0)
      m_type = FracturePressureFunctionType::FunctionOfLithostaticPressure;
   else assert(0);

   if (pressureIoTblRecord)
       m_name = database::getPresFuncName(pressureIoTblRecord);

   if ( projectHandle.getRunParameters ()->getFractureModel () <= 0 or
        projectHandle.getRunParameters ()->getFractureModel () > int ( CONSERVATIVE_2 )) {
      m_fractureModel = FracturePressureModel::NON_CONSERVATIVE_TOTAL;
   } else {
      m_fractureModel = FracturePressureModel ( projectHandle.getRunParameters ()->getFractureModel ());
   }

   if ( m_type != FunctionOfLithostaticPressure and m_type != None and database::getSelected(m_record) == 1 ) {
      m_coefficients.push_back ( database::getCoefA(m_record));
      m_coefficients.push_back ( database::getCoefB(m_record));
      m_coefficients.push_back ( database::getCoefC(m_record));
      m_coefficients.push_back ( database::getCoefD(m_record));
   }

}

FracturePressureFunctionParameters::FracturePressureFunctionParameters(
   ProjectHandle& projectHandle, database::Record* anyRecord):
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
