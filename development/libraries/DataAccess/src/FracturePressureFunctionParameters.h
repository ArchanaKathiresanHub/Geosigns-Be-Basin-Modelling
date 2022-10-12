#ifndef DISTRIBUTEDDATAACCESS_INTERFACE_FRACTUREPRESSUREFUNCTIONPARAMETERS_H_
#define DISTRIBUTEDDATAACCESS_INTERFACE_FRACTUREPRESSUREFUNCTIONPARAMETERS_H_

#include <vector>
#include <string>

#include "Interface.h"
#include "DAObject.h"
#include "ProjectHandle.h"

namespace DataAccess { namespace Interface {

   class FracturePressureFunctionParameters: public DAObject
   {

   public:
      FracturePressureFunctionParameters(ProjectHandle& projecthandle, database::Record* runOptionsIoTblRecord,
                                         database::Record* pressureIoTblRecord);

      FracturePressureFunctionParameters(ProjectHandle& projecthandle, database::Record* anyRecord);

      FracturePressureFunctionType type() const;

      FracturePressureModel getFractureModel () const;

      const std::vector<double>& coefficients() const;

      /// \brief Get the name of the fracture function type.
      const std::string& getTypeName () const;

      /// \brief Get the name of the function.
      const std::string& getName () const;

   private:

      FracturePressureFunctionType m_type;
      FracturePressureModel        m_fractureModel;
      std::vector<double>               m_coefficients;
      std::string                  m_typeName;
      std::string                  m_name;

   };

} }

#endif
