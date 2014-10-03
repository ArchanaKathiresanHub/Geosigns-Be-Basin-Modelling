//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmOneCrustThinningEvent.C
/// @brief This file keeps API implementation for single event crust thinning parameter handling


#include "PrmOneCrustThinningEvent.h"
#include "VarPrmOneCrustThinningEvent.h"

// CMB API
#include "cmbAPI.h"

#include <cassert>
#include <cmath>
#include <sstream>

namespace casa
{

static const char * s_stratIoTblName         = "StratIoTbl";
static const char * s_stratIoTblAgeCol       = "DepoAge";
static const char * s_crustIoTblName         = "CrustIoTbl";
static const char * s_crustIoTblAgeCol       = "Age";
static const char * s_crustIoTblThicknessCol = "Thickness";
static const double s_eps = 1.e-8;

// Constructor. Get parameter values from the model
PrmOneCrustThinningEvent::PrmOneCrustThinningEvent( mbapi::Model & mdl ) : m_parent( 0 )
{
   int crustIoTblSize = mdl.tableSize( s_crustIoTblName );

   m_initialThicknes = UndefinedDoubleValue;
   m_t0              = UndefinedDoubleValue;
   m_dt              = UndefinedDoubleValue;
   m_coeff           = UndefinedDoubleValue;

   if ( ErrorHandler::NoError == mdl.errorCode() )
   {
      std::vector<double> t( crustIoTblSize );
      std::vector<double> d( crustIoTblSize );

      for ( size_t i = 0; i < crustIoTblSize; ++i ) // extract crust thickness profile with time, supposed to be sorted
      {
         t[i] = mdl.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblAgeCol );
         d[i] = mdl.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblThicknessCol );
      }

      if ( 4 == crustIoTblSize )
      {
         m_initialThicknes = d.back();
         m_coeff = d.front() / d.back();
         m_t0 = t[2];
         m_dt = t[2] - t[1];
      }
   }
}

// Constructor. Set given parameter values
PrmOneCrustThinningEvent::PrmOneCrustThinningEvent( const VarPrmOneCrustThinningEvent * parent, double thickIni, double t0, double dt, double coeff ) : 
     m_parent( parent )
   , m_initialThicknes( thickIni ) 
   , m_t0( t0 )
   , m_dt( dt )
   , m_coeff( coeff )
{
}

// Set this parameter value in Cauldron model
ErrorHandler::ReturnCode PrmOneCrustThinningEvent::setInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.clearTable( s_crustIoTblName ) ) return caldModel.errorCode();
  
   // get position in table the eldest layer
   int firstLayerRow = caldModel.tableSize( s_stratIoTblName ) - 1;
   if ( ErrorHandler::NoError != caldModel.errorCode() ) return caldModel.errorCode();
   
   // get age of the eldest layer
   double eldestAge = caldModel.tableValueAsDouble( s_stratIoTblName, firstLayerRow, s_stratIoTblAgeCol );
   if ( ErrorHandler::NoError != caldModel.errorCode() ) return caldModel.errorCode();

   // add 4 rows to the table
   for ( size_t i = 0; i < 4; ++i )
   {
      if ( ErrorHandler::NoError != caldModel.addRowToTable( s_crustIoTblName ) ) return caldModel.errorCode();
   }

   // 0 time
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 0, s_crustIoTblAgeCol,       0.0                         ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 0, s_crustIoTblThicknessCol, m_initialThicknes * m_coeff ) ) return caldModel.errorCode();

   // end event time
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 1, s_crustIoTblAgeCol,       m_t0 - m_dt                 ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 1, s_crustIoTblThicknessCol, m_initialThicknes * m_coeff ) ) return caldModel.errorCode();

   // start event time
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 2, s_crustIoTblAgeCol,       m_t0              ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 2, s_crustIoTblThicknessCol, m_initialThicknes ) ) return caldModel.errorCode();

   // before time begin
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 3, s_crustIoTblAgeCol,       eldestAge         ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( s_crustIoTblName, 3, s_crustIoTblThicknessCol, m_initialThicknes ) ) return caldModel.errorCode();

   return ErrorHandler::NoError;
}

// Validate crust thinning parameter values 
std::string PrmOneCrustThinningEvent::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_initialThicknes < 0.0    ) { oss << "Initial crust thickness value can not be negative: "                             << m_initialThicknes << "\n"; }
   else if ( m_initialThicknes > 100000 ) { oss << "Too big value for the initial crust thickness, it can not be more then 100 km: " << m_initialThicknes << "\n"; }

   if      ( m_t0 <= 0.0   ) { oss << "Starting time for the crust thinning event must be greater than 0: " << m_t0 << "\n"; }
   else if ( m_t0 > 1000.0 ) { oss << "Too big value for the starting time of crust thinning event: "       << m_t0 << "\n"; }

   if (      m_dt < 0    ) { oss << "Duration of the crust thinning event can not be negative: "                   << m_dt << "\n"; }
   else if ( m_dt > m_t0 ) { oss << "Duration of the crust thinning event can not be greater than starting time: " << m_dt << "\n"; }

   if (      m_coeff < 0 ) { oss << "Crust thinning factor can not be negative: "      << m_coeff << "\n"; }
   else if ( m_coeff > 1 ) { oss << "Crust thinning factor can not greater than 1.0: " << m_coeff << "\n"; }
 

   int crustIoTblSize = caldModel.tableSize( s_crustIoTblName );
   
   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }
   
   if ( crustIoTblSize != 4 ) { oss << "Project has a mismatched profile to single crust thinning event for the crust thinning history\n"; }

   double t[4];
   double d[4];
   for ( size_t i = 0; i < 4; ++i ) // extract crust thickness profile with time, supposed to be sorted
   {
      t[i] = caldModel.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblAgeCol );
      d[i] = caldModel.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblThicknessCol );
   }
   
   if ( std::abs( d[3] - m_initialThicknes ) > s_eps ) 
   { 
      oss << "Initial crust thickness value in the model (" << d[3] << ") is differ from parameter value (" << m_initialThicknes << ")\n";
   }

   if ( std::abs( t[2] - m_t0 ) > s_eps ) 
   {
      oss << "Start time for crust thinning value in the model (" << t[2] << ") is differ from parameter value (" << m_t0 << ")\n";
   }

   if ( std::abs( (t[2] - t[1]) - m_dt ) > s_eps )
   {
      oss << "Crust thinning duration value in the model (" << t[2]-t[1] << ") is differ from parameter value (" << m_dt << ")\n";
   }

   if ( std::abs( (d[3]/d[0] ) - m_coeff ) > s_eps )
   {
      oss << "Crust thinning factor value in the model (" << d[3]/d[0] << ") is differ from parameter value (" << m_coeff << ")\n";
   }

   if ( std::abs( d[0] - d[1] ) > s_eps || std::abs( d[3] - d[2] ) > s_eps )
   {
      oss << "Crust thinning profile in model does not match to single crust thinning event pattern. Crust thickness must decrease only between 2nd and 3d point\n";
   }

   return oss.str(); // another model, no reason to check further
}
  
// Get parameter value as an array of doubles
std::vector<double> PrmOneCrustThinningEvent::asDoubleArray() const
{
   std::vector<double> vals(4);

   vals[0] = m_initialThicknes;
   vals[1] = m_t0;
   vals[2] = m_dt;
   vals[3] = m_coeff;

   return vals;
}

}


