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

// CASA API
#include "Parameter.h"
#include "VarParameter.h"
#include "CasaDeserializer.h"

// STL/C
#include <memory>
#include <set>

/// @page CASA_VarPrmContinuousPage Continuous variable parameter
///
/// Continuous parameter - a parameter that can take any value between certain bounds
/// (for instance, a fault-sealing factor that varies in the [0, 1] range)
///
/// The following list of variable parameters is implemented in CASA API
/// - @subpage CASA_SourceRockTOCPage 
/// - @subpage CASA_SourceRockHIPage 
/// - @subpage CASA_SourceRockHCPage 
/// - @subpage CASA_SourceRockPreAsphaltStartActPage
/// - @subpage CASA_TopCrustHeatProductionPage 
/// - @subpage CASA_OneCrustThinningEventPage 
/// - @subpage CASA_CrustThinningPage
/// - @subpage CASA_PorosityModelPage 
/// - @subpage CASA_PermeabilityModelPage
/// - @subpage CASA_LithoSTPThermalCondPage

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

      /// @brief Average the values, interpolate for lithofractions and set the appropriate entries in the project3d file
      /// @return new parameter for given set of values
      virtual SharedParameterPtr makeThreeDFromOneD( mbapi::Model              & mdl ///< [in,out] the model where to set the new averaged parameter
                                                   , const std::vector<double> & xin ///< the x coordinates of each 1D project 
                                                   , const std::vector<double> & yin ///< the y coordinates of each 1D project 
                                                   , const std::vector<SharedParameterPtr> & prmVec /// the optimal parameter value of each 1D project
                                                   ) const = 0;

      /// @brief Convert Cauldron parameter values to SUMlib values for some variable parameters
      /// @param prm cauldron parameter with to this variable parameter corresponded type
      /// @return parameter values suitable for SUMlib
      virtual std::vector<double> asDoubleArray( const SharedParameterPtr prm ) const { return prm->asDoubleArray(); }
      
      /// @brief Returns mask array where for selected parameters true value set
      ///        Selected parameters means parameters where min/max values are different
      /// @return mask array with true value for selected parameters
      virtual std::vector<bool> selected() const;

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

      
      // Available slots
      // called from categorical parameter on which this parameter depends on
      virtual void onCategoryChosen( const Parameter * ) { ; }
      virtual void onSerialization( CasaSerializer::ObjRefID objSerID ) { m_dependsOn.insert( objSerID ); }

   protected:
      VarPrmContinuous() : m_pdf(Block) {;}

      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief  Implements common part of deserialization for continuous variable parameters
      /// @param dz input stream
      /// @param objVer The object version.
      bool deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer );

      SharedParameterPtr m_baseValue;   ///< Base parameter value, used also as object factory for concrete parameter value
      SharedParameterPtr m_minValue;    ///< Base parameter value, used also as object factory for concrete parameter value
      SharedParameterPtr m_maxValue;    ///< Base parameter value, used also as object factory for concrete parameter value
      PDF                m_pdf;         ///< Probability density function for parameter. Block is default value

      std::set< CasaSerializer::ObjRefID > m_dependsOn; // list of category parametrs ID on which this parameter is dependent
   private:
   };

}

#endif // CASA_API_VAR_PRM_CONTINOUS_H
