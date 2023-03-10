//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmWindow.h
/// @brief This file keeps API declaration for the window handling.

#ifndef CASA_API_PARAMETER_WINDOW_H
#define CASA_API_PARAMETER_WINDOW_H

#include "Parameter.h"

// CMB API
#include "UndefinedValues.h"

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_WindowedProject Project areal window
///
/// Window in the Project3d file specified by WindowXMin, WindowXMax, WindowYMin, WindowYMax  columns in ProjectIoTbl table
namespace casa
{
   class VarParameter;

   /// @brief Reduce the project size by imposing window over project XY area
   class PrmWindow : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get the values of the window
      PrmWindow( mbapi::Model & mdl );

      /// @brief Constructor. Create parameter from min/max window values
      /// @param xMin minimum node in x direction
      /// @param xMax maximum node in x direction
      /// @param yMin minimum node in y direction
      /// @param yMax maximum node in y direction
      PrmWindow( const int xMin, const int xMax, const int yMin, const int yMax, const double xCoordObservable = 0.0, const double yCoordObservable = 0.0 );

      ///@brief Destructor
      virtual ~PrmWindow() {;}

      /// @brief Get influential parameter which was used to create this parameter
      /// @return Pointer to the influential parameter
      virtual const VarParameter * parent() const { return 0; }

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate windowing parameter values
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

      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 2; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "PrmWindow"; }

      std::vector<double> observableOrigin() const;

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmWindow( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   private:
      int         m_xMin;
      int         m_xMax;
      int         m_yMin;
      int         m_yMax;
      double      m_xCoordObservable;
      double      m_yCoordObservable;
   };

} // namespace casa

#endif // CASA_API_PARAMETER_WINDOW_H
