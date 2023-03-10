//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmOneCrustThinningEvent.h
/// @brief This file keeps API declaration for one event of crust thinning parameter handling.

#ifndef CASA_API_PARAMETER_ONE_CRUST_THINNING_EVENT_H
#define CASA_API_PARAMETER_ONE_CRUST_THINNING_EVENT_H

#include "Parameter.h"

// CMB API
#include "UndefinedValues.h"

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_OneCrustThinningEventPage One event crust thinning parameter
///
/// Crust thickness in Cauldron should be defined by a piecewise linear function @f$ D( t ) @f$
/// User must provide a sorted by time a sequence of points @f$ [p_0(t_0, d_0), p_1(t_1, d_1), ... ] @f$ .
/// This influential parameter allows to define a crust thickness function with one crust thinning event.
/// To define such event, user should provide these sub-parameters:
/// -# initial crust thickness @f$ d_0 @f$ [m]. The valid range is [0:100000]
/// -# start time for the thinning event @f$ t_0 @f$ [Ma]. The valid range is [0:1000];
/// -# duration of the thinning @f$ \delta t = t_1 - t_0 @\f$ [Ma]. The value must be in range  @f$ 0 < \delta t < t_0 @f$
/// -# final crust thickness which is defined as a @f$ d_1 = \sigma \cdot d_0 @f$ where is @f$ \sigma @f$ - a thinning factor with valid range [0:1].
///
/// @image html CrustThinningOneEvent.png "One event of crust thinning"
namespace casa
{
   class VarPrmOneCrustThinningEvent;

   /// @brief Single event crust thinning parameter
   class PrmOneCrustThinningEvent : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get value for single event crust thinning parameter
      PrmOneCrustThinningEvent( mbapi::Model & mdl );

      /// @brief Constructor. Create parameter from variation of influential parameter
      /// @param parent pointer to a influential parameter which created this one
      /// @param thickIni initial crust thickness [m]
      /// @param t0 start time for crust thinning event [Ma]
      /// @param dt duration of crust thinning event [Ma]
      /// @param coeff crust thinning factor [unitless]
      PrmOneCrustThinningEvent( const VarPrmOneCrustThinningEvent * parent, double thickIni, double t0, double dt, double coeff );

      ///@brief Destructor
      virtual ~PrmOneCrustThinningEvent() = default;

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate crust thinning parameter values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects

      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const;

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert( false ); return Utilities::Numerical::NoDataIntValue; }

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 1; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "PrmOneCrustThinningEvent"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmOneCrustThinningEvent( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   private:
      double               m_initialThickness; ///< initial crust thickness
      double               m_t0;               ///< start time for thinning event
      double               m_dt;               ///< duration of thinning event
      double               m_coeff;            ///< factor for the crust thinning
   };
}
#endif // CASA_API_PARAMETER_ONE_CRUST_THINNING_EVENT_H
