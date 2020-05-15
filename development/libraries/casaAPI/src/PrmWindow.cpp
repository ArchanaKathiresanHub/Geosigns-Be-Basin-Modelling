//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmWindow.C
/// @brief This file keeps API implementation for for window handling.

// CASA API
#include "PrmWindow.h"
#include "VarParameter.h"

// ModelPseudo1d
#include "ModelPseudo1d.h"
#include "ModelPseudo1dInputData.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>
#include "Path.h" // for to_string

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>

namespace casa
{

static const char * s_projectIoTblName = "ProjectIoTbl";
static const char * s_ScaleX           = "ScaleX";
static const char * s_ScaleY           = "ScaleY";
static const char * s_WindowXMinCol    = "WindowXMin";
static const char * s_WindowXMaxCol    = "WindowXMax";
static const char * s_WindowYMinCol    = "WindowYMin";
static const char * s_WindowYMaxCol    = "WindowYMax";

// Constructor. Get parameter values from the model
PrmWindow::PrmWindow( mbapi::Model & mdl )
{
  m_xMin = mdl.tableValueAsInteger( s_projectIoTblName, 0, s_WindowXMinCol );
  m_xMax = mdl.tableValueAsInteger( s_projectIoTblName, 0, s_WindowXMaxCol );
  m_yMin = mdl.tableValueAsInteger( s_projectIoTblName, 0, s_WindowYMinCol );
  m_yMax = mdl.tableValueAsInteger( s_projectIoTblName, 0, s_WindowYMaxCol );
}

// Constructor. Set given parameter values
PrmWindow::PrmWindow(const int xMin, const int xMax, const int yMin, const int yMax, const double xCoordObservable, const double yCoordObservable)
  : m_xMin( xMin )
  , m_xMax( xMax )
  , m_yMin( yMin )
  , m_yMax( yMax )
  , m_xCoordObservable( xCoordObservable )
  , m_yCoordObservable( yCoordObservable )
{}

// Set this parameter value in Cauldron model
ErrorHandler::ReturnCode PrmWindow::setInModel( mbapi::Model & caldModel, size_t /*caseID*/ )//_TODO: change here.
{
  try
  {
    modelPseudo1d::ModelPseudo1d mdlPseudo1d (caldModel, m_xCoordObservable, m_yCoordObservable);

    mdlPseudo1d.initialize();
    mdlPseudo1d.extractScalarsFromInputMaps();
    mdlPseudo1d.setScalarsInModel();
    mdlPseudo1d.setSingleCellWindowXY();

  }
  catch( const ErrorHandler::Exception & ex ) { return caldModel.reportError( ex.errorCode(), ex.what() ); }

  return ErrorHandler::NoError;
}

// Validate window parameter values
std::string PrmWindow::validate( mbapi::Model & caldModel )
{
  std::ostringstream oss;

  const std::vector<double> & prms = asDoubleArray();
  std::vector<const char *>          colNames( 4 );

  colNames[0] = s_WindowXMinCol;
  colNames[1] = s_WindowXMaxCol;
  colNames[2] = s_WindowYMinCol;
  colNames[3] = s_WindowYMaxCol;

  for( size_t i = 0; i < colNames.size(); ++i )
  {
    if ( prms[i] < 0 ) { oss << colNames[i] << " can not be negative: " << prms[i] << "\n"; }

    if ( i % 2 > 0 && prms[i] - prms[i-1] < 0 ) { oss << colNames[i-1] << " can not be larger than " << colNames[i] << "\n"; }

    int mdlVal = caldModel.tableValueAsInteger( s_projectIoTblName, 0, colNames[i] );
    if ( ErrorHandler::NoError != caldModel.errorCode() ) { oss << caldModel.errorMessage() << std::endl; return oss.str(); }
    if ( mdlVal != prms[i] ) { oss << colNames[i] << " in the model (" << mdlVal << ") is differ from a parameter value (" << prms[i] << ")\n"; }
  }
  return oss.str();
}

// Get parameter value as an array of doubles
std::vector<double> PrmWindow::asDoubleArray() const
{
  std::vector<double> vals( 4 );

  vals[0] = m_xMin;
  vals[1] = m_xMax;
  vals[2] = m_yMin;
  vals[3] = m_yMax;

  return vals;
}

std::vector<double> PrmWindow::observableOrigin() const
{
  return { m_xCoordObservable, m_yCoordObservable };
}

// Are two parameters equal?
bool PrmWindow::operator == ( const Parameter & prm ) const
{
  const PrmWindow * pp = dynamic_cast<const PrmWindow *>( &prm );
  if ( !pp ) return false;

  if ( m_xMin != pp->m_xMin ) return false;
  if ( m_xMax != pp->m_xMax ) return false;
  if ( m_yMin != pp->m_yMin ) return false;
  if ( m_yMax != pp->m_yMax ) return false;

  return true;
}

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmWindow::save( CasaSerializer & sz ) const
{
  bool ok = true;

  ok = ok && sz.save( m_xMin, "xMin" );
  ok = ok && sz.save( m_xMax, "xMax" );
  ok = ok && sz.save( m_yMin, "yMin" );
  ok = ok && sz.save( m_yMax, "yMax" );
  ok = ok && sz.save( m_xCoordObservable, "xCoordObservable" );
  ok = ok && sz.save( m_yCoordObservable, "yCoordObservable" );

  return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmWindow::PrmWindow( CasaDeserializer & dz, unsigned int objVer )
{
  bool ok = true;
  if ( objVer < 2 )
  {
    std::string name;
    dz.load( name, "name" );
  }
  ok = ok ? dz.load( m_xMin, "xMin" ) : ok;
  ok = ok ? dz.load( m_xMax, "xMax" ) : ok;
  ok = ok ? dz.load( m_yMin, "yMin" ) : ok;
  ok = ok ? dz.load( m_yMax, "yMax" ) : ok;

  if (objVer > 0)
  {
    ok = ok ? dz.load( m_xCoordObservable, "xCoordObservable" ) : ok;
    ok = ok ? dz.load( m_yCoordObservable, "yCoordObservable" ) : ok;
  }

  if ( !ok )
  {
    throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "PrmWindow deserialization unknown error";
  }
}

}
