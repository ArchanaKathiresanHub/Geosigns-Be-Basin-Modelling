//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmContinuous.h
/// @brief This file keeps API declaration for handling continuous parameters. 

#ifndef CASA_API_VAR_PRM_CONTINOUS_H
#define CASA_API_VAR_PRM_CONTINOUS_H

#include "Parameter.h"
#include "VarParameter.h"
#include "CasaDeserializer.h"


#include <memory>

/// @page CASA_VarPrmContinuousPage Continuous variable parameter
///
/// Continuous parameter - a parameter that can take any value between certain bounds
/// (for instance, a fault-sealing factor that varies in the [0, 1] range)
///
/// The following list of variable parameters is implemented in CASA API
/// - @subpage CASA_SourceRockTOCPage 
/// - @subpage CASA_TopCrustHeatProductionPage 
/// - @subpage CASA_OneCrustThinningEventPage 

namespace casa
{
   /// @brief Variable parameter with continuous value range.
   /// The parameter value can be represented by the one or several doubles values
   class VarPrmContinuous : public VarParameter
   {
   public:
      /// @brief Probability Density Function (PDF) shape for the parameter. It is used in casa::MonteCarloSolver
      enum PDF
      {
         Block,    ///< PDF is uniform in parameter range
         Triangle, ///< triangle shape of the PDF. Maximum position of PDF is defined by parameter value in base case
         Normal    ///< Gauss shape of the pdf
      };

      /// @brief Destructor
      virtual ~VarPrmContinuous() {;}

      /// @brief Define this variable parameter as a continuous
      /// @return VarParameter::Continuous
      virtual Type variationType() const { return Continuous; }

      /// @brief A parameter which corresponds the minimal range value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr minValue() const { return m_minValue; }

      /// @brief A parameter which corresponds the maximal range value of the variable parameter 
      /// @return the parameter object should be deleted by a caller
      virtual const SharedParameterPtr maxValue() const { return m_maxValue; }

      /// @brief A parameter which corresponds the base value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr baseValue() const { return m_baseValue; }

      /// @brief For continuous parameters weights are calculated using PDF
      /// @return empty array 
      virtual std::vector<double> weights() const { return std::vector<double>(); }

      /// @brief Get Probability Density Function type of the variable parameter
      /// @return parameter PDF type
      virtual PDF pdfType() const { return m_pdf; }

      /// @brief Calculate standard deviation values for parameter
      /// @return vector with standard deviation value for each sub-parameter
      std::vector<double> stdDevs() const;

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const = 0;

      /// @brief Wrapper function to use in C# through Swig due to absence of iterators in Swig
      /// @param vals vector with parameters values.
      /// @param[in,out] off position in array where this parameter values are located
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles(std::vector<double> & vals, int & off) const
      {
         std::vector<double>::const_iterator it = vals.begin() + off;
         std::vector<double>::const_iterator sit = it;
         SharedParameterPtr ret = newParameterFromDoubles( it );
         off += static_cast<unsigned int>(it - sit);
         return ret;
      }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new VarPrmContinuous instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objName expected object name
      /// @return new observable instance on susccess, or throw and exception in case of any error
      static VarPrmContinuous * load( CasaDeserializer & dz, const char * objName );

   protected:
      VarPrmContinuous() : m_pdf(Block) {;}

      // version of serialized object representation
      virtual unsigned int version() const { return 0; } // could be overloaded by child classes

      /// @brief  Constructor from input stream, implements common part of deserialization for continuous variable parameters
      /// @param dz input stream
      /// @param objVer The object version.
      VarPrmContinuous( CasaDeserializer & dz, unsigned int objVer );

      SharedParameterPtr m_baseValue;   ///< Base parameter value, used also as object factory for concrete parameter value
      SharedParameterPtr m_minValue;    ///< Base parameter value, used also as object factory for concrete parameter value
      SharedParameterPtr m_maxValue;    ///< Base parameter value, used also as object factory for concrete parameter value

      PDF                m_pdf;         ///< Probability density function for parameter. Block is default value
   
   private:
   };

}

#endif // CASA_API_VAR_PRM_CONTINOUS_H
