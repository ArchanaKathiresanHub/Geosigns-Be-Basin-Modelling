//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmOneCrustThinningEvent.h
/// @brief This file keeps API declaration for handling variation of casa::PrmOneCrustThinningEvent parameter

#ifndef CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H
#define CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmOneCrustThinningEvent parameter
   class VarPrmOneCrustThinningEvent: public VarPrmContinuous
   {
   public:
      /// @brief Construct variable parameter for one crust thinning event in crust thickness history. 
      ///
      /// Crust thickness in Cauldron should be defined by a piecewise linear function @f$ thickness( time ) @f$ 
      /// User must provide a sorted by time a sequence of points @f$ [(time, thickness), ... ] @f$ .
      /// This variable parameter allows to define a crust thickness function with one crust thinning event.
      /// To define this event user should provide these sub-parameters:
      /// -# @f$ d_0 @f$ - [m] initial crust thickness - defines thickness of the crust at the beginning of simulation
      /// -# @f$ t_0 @f$ - [Ma] defines time when crust thickness starts to change
      /// -# @f$ \delta t @f$ - [Ma] defines duration of crust thinning event
      /// -# @f$ \sigma @f$ - [dimensionless] crust thinning factor ( @f$ \sigma = \frac{d_1}{d_0} @f$ )
      VarPrmOneCrustThinningEvent( double baseThickIni      ///<  base value for the initial thickness
                                 , double minThickIni       ///<  minimal value for the initial thickness
                                 , double maxThickIni       ///<  maximal value for the initial thickness
                                 , double baseT0            ///<  base value for the event start time
                                 , double minT0             ///<  minimal value for the event start time
                                 , double maxT0             ///<  maximal value for the event start time
                                 , double baseDeltaT        ///<  base value for the event duration
                                 , double minDeltaT         ///<  minimal value for the event duration
                                 , double maxDeltaT         ///<  maximal value for the event duration
                                 , double baseThinningFct   ///<  base value for the thinning factor
                                 , double minThinningFct    ///<  minimal value for the thinning factor
                                 , double maxThinningFct    ///<  maximal value for the thinning factor
                                 , PDF prmPDF               ///<  probability density function for all 4 sub-parameters
                                 );

      virtual ~VarPrmOneCrustThinningEvent();

      /// @brief Get name of variable parameter in short form     
      /// @return array of names for each subparameter
      virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const { return 4; }

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmOneCrustThinningEvent"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const { return VarPrmContinuous::save( sz, version ); }

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmOneCrustThinningEvent( CasaDeserializer & dz, unsigned int objVer ) : VarPrmContinuous( dz, objVer ) { ; }
      /// {@
   protected:
   };
}

#endif // CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H

