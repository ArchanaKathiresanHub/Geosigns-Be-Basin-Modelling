//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmCrustThinning.h
/// @brief This file keeps API declaration for handling variation of casa::PrmCrustThinning parameter

#ifndef CASA_API_VAR_PARAMETER_CRUST_THINNING_H
#define CASA_API_VAR_PARAMETER_CRUST_THINNING_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmCrustThinning parameter
   class VarPrmCrustThinning: public VarPrmContinuous
   {
   public:
      /// @brief Construct influential parameter for one crust thinning event in crust thickness history. 
      ///
      /// Crust thickness in Cauldron should be defined by a piecewise linear function @f$ thickness( time ) @f$ 
      /// User must provide a sorted by time a sequence of points @f$ [(time, thickness), ... ] @f$ .
      /// This influential parameter allows to define a crust thickness function as set of crust thinning events.
      /// To define this event user should provide these sub-parameters:
      /// -# @f$ d_0 @f$ - [m] initial crust thickness - defines thickness of the crust at the beginning of simulation
      ///
      /// -# @f$ t_0 @f$ - [Ma] defines time when crust thickness starts to change
      /// -# @f$ \delta t @f$ - [Ma] defines duration of crust thinning event
      /// -# @f$ \sigma @f$ - [dimensionless] crust thinning factor ( @f$ \sigma = \frac{d_1}{d_0} @f$ )
      /// Constructor takes 4 arrays (base,min,max,maps) of values as parameters. 
      /// Base/min/max arrays must contain:
      ///    @f$ d_0 @f$  as the first value, then
      ///    list of triplets for each event: @f$ (t_0^1,\delta t^1,\sigma^1),(t_0^2,\delta t^2,\sigma^2),...@f$ 
      /// Array of optional maps must be the size of number of thinning events, and for each event user could define thickness map
      VarPrmCrustThinning( const std::vector<double>      & basePrmValues ///< base case parameter values
                         , const std::vector<double>      & minPrmValues  ///< min range parameter values
                         , const std::vector<double>      & maxPrmValues  ///< max range parameter values
                         , const std::vector<std::string> & mapsList      ///< list of optional maps 
                         , PDF                              prmPDF        ///< probability density function (one for all sub-parameters)
                         , const char                     * name          ///< user specified parameter name
                         );

      virtual ~VarPrmCrustThinning();

      /// @brief Get name of influential parameter in short form     
      /// @return array of names for each subparameter
      virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of influential parameter
      virtual size_t dimension() const { return 1 + m_eventsNumber * 3; }

      using VarPrmContinuous::newParameterFromDoubles;
      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @brief Create parameter by reading the values stored in the project file
      /// @param[in, out] mdl the model where the parameters values should be read
      /// @param[in] vin an input vector with parameter cpecific values. Not used for this parameter
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const;

      /// @brief Average the values, interpolate for lithofractions and set the appropriate entries in the project3d file
      /// @return new parameter for given set of values
      virtual SharedParameterPtr makeThreeDFromOneD( mbapi::Model              & mdl ///< [in,out] mdl the model where to set the new averaged parameter
                                                   , const std::vector<double> & xin ///< the x coordinates of each 1D project 
                                                   , const std::vector<double> & yin ///< the y coordinates of each 1D project 
                                                   , const std::vector<SharedParameterPtr> & prmVec ///< the optimal parameter value of each 1D project
                                                   ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmCrustThinning"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmCrustThinning( CasaDeserializer & dz, unsigned int objVer );
      /// @}
      
   protected:
      size_t                   m_eventsNumber;
      std::vector<std::string> m_mapsName;
   };
}

#endif // CASA_API_VAR_PARAMETER_CRUST_THINNING_H

