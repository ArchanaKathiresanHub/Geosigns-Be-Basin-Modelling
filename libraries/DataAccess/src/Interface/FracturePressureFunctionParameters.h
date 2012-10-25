#ifndef DISTRIBUTEDDATAACCESS_INTERFACE_FRACTUREPRESSUREFUNCTIONPARAMETERS_H_
#define DISTRIBUTEDDATAACCESS_INTERFACE_FRACTUREPRESSUREFUNCTIONPARAMETERS_H_

#include <vector>

#include "../Interface/Interface.h"
#include "../Interface/DAObject.h"
#include "../Interface/ProjectHandle.h"

namespace DataAccess { namespace Interface {

   class FracturePressureFunctionParameters: public DAObject
   {
   private:

      FracturePressureFunctionType m_type;
      FracturePressureModel        m_fractureModel;
      vector<double>               m_coefficients;

   public:
      FracturePressureFunctionParameters(ProjectHandle* projecthandle, database::Record* runOptionsIoTblRecord, 
                                         database::Record* pressureIoTblRecord);

      FracturePressureFunctionParameters(ProjectHandle* projecthandle, database::Record* anyRecord);

      FracturePressureFunctionType type() const;

      FracturePressureModel getFractureModel () const;

      const std::vector<double>& coefficients() const;

   };

} }

#endif
