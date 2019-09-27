//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmCrustThinning.h
/// @brief This file keeps API declaration for crust thinning parameter handling.

#ifndef CASA_API_PARAMETER_CRUST_THINNING_H
#define CASA_API_PARAMETER_CRUST_THINNING_H

#include "Parameter.h"

// CMB API
#include "UndefinedValues.h"

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_CrustThinningPage Multi event crust thinning parameter
///
/// Crust thickness in Cauldron should be defined by a piecewise linear function @f$ D( t ) @f$
/// User must define a sorted by time a sequence of points @f$ [p_0(t_0, d_0), p_1(t_1, d_1), ... ] @f$ .
/// This influential parameter allows to define the crust thickness function as a series of crust thinning events.
/// At first user should define the initial crust thickness: @f$ d_0 @f$ [m]. The valid range is [0:100000]
/// Then must be defined a series of crust thinning events.
/// For each event user should define:
/// -# start time for the thinning event @f$ t_s @f$ [Ma]. The valid range is [0:1000];
/// -# duration of the thinning event @f$ \delta t = t_e - t_s @f$ [MY]. The value must be in range  @f$ 0 < \delta t < t_s @f$
/// -# crust thinning factor @f$ \sigma @f$  The valid range is [0:1]
/// -# optional thickness map name
/// The crust thickness after each event will be equal @f$ d_{n} = d_{n-1} \cdot \sigma @f$ if no map name was specified or
/// @f$ d_{n} = map_n \cdot \sigma @f$ if map was given for this event.
///
///              t0       t1 t2      t3 t4    t5 t6
///            --+--------+--+-------+--+-----+--+--->
///           S0 + *--------*  .       .  .     .  .     t0: S0 - ThickIni                      |     | T0 |  DeltaT | ThinningFct | MapName |
///      Ev1 [   |           \ .       .  .     .  .     t1: S0                                 | ----|----|---------|:-----------:|---------|
///           S1 +.  .  .  .Map1-------*  .     .  .     t2: S1 = Map1 * f2                     | Ev1 | t1 | (t2-t1) |     f1      |"Map1"   |
///          [   |                      \ .     .  .     t3: S1                                 | Ev2 | t3 | (t4-t1) |     f2      | ""      |
///      Ev2 [   |                       \.     .  .     t4: S2 = S1 * f3 = (Map1 * f2)  * f3   | Ev3 | t5 | (t6-t5) |     f3      |"Map2"   |
///           S2 +.  .  .  .  .  .  .  .  *-----*  .     t5  S2
///      Ev3 [   |                               \ .     t6  S4 = Map2 * f4
///           S3 +.  .  .  .  .  .  .  .  .  .  . Map2
///              V
///
/// Example of crust thinning events sequence
namespace casa
{
   class VarPrmCrustThinning;

   /// @brief Single event crust thinning parameter
   class PrmCrustThinning : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get value for crust thinning parameter
      PrmCrustThinning( mbapi::Model & mdl );

      /// @brief Constructor. Create parameter from variation of influential parameter
      /// @param parent pointer to a influential parameter which created this one
      /// @param prmValues array of values:
      ///          - initial crust thickness [m]
      ///          - sequence of triplets (t0,dT,fct) for the events series
      /// @param mapsList list of optional maps
      PrmCrustThinning( const VarPrmCrustThinning * parent, const std::vector<double> & prmValues, const std::vector<std::string> & mapsList );

      ///@brief Destructor
      virtual ~PrmCrustThinning() = default;

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }

      /// @brief Get list of maps for the event sequense
      /// @return maps list
      std::vector<std::string> getMapsList() const { return m_maps; }

      /// @brief Get number of events
      /// @return events number
      size_t numberOfEvents() const { return m_eventsNumber; }

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
      virtual const char * typeName() const { return "PrmCrustThinning"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmCrustThinning(CasaDeserializer & dz, unsigned int objVer );
      /// @}

   private:
      double                   m_initialThickness; ///< initial crust thickness
      size_t                   m_eventsNumber;     ///< number of thinning events
      std::vector<double>      m_t0;               ///< events start time
      std::vector<double>      m_dt;               ///< events duration
      std::vector<double>      m_coeff;            ///< events crust thinning factor
      std::vector<std::string> m_maps;             ///< optional events map
   };
}
#endif // CASA_API_PARAMETER_CRUST_THINNING_H
