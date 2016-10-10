//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LinearFunction.h"

#include <math.h>
#include <iostream>

using namespace std;

//------------------------------------------------------------//
LinearFunction::LinearFunction() {

   m_WLS_crit = 0.0;
   m_WLS_onset = 0.0;
   m_m1 = 0.0;
   m_m2 = 0.0;
   m_c2 = 0.0;
   m_maxBasalticCrustThickness = 0.0;
   m_magmaThicknessCoeff = 0.0;
} 
//------------------------------------------------------------//
double LinearFunction::getCrustTF( const double WLS ) {

   double TF = 1.0;
   
   if( m_maxBasalticCrustThickness == 0 ) {
      TF = m_m1 * WLS;
   } else {
      if( WLS < m_WLS_onset ) {
         TF = m_m1 * WLS;
      }else if( WLS > m_WLS_crit ) {
         TF = 1.0;
      } else {
         TF =  m_m2 * WLS + m_c2;
      }
   }
   return TF;
}
//------------------------------------------------------------//
double LinearFunction::getBasaltThickness( const double WLS ) {

   double thickness = 0.0;

   if( m_maxBasalticCrustThickness != 0 ) {
      if ( WLS < m_WLS_onset)  {
         thickness = 0.0;
      } else if( WLS >= m_WLS_crit ){
         thickness = m_maxBasalticCrustThickness - (WLS - m_WLS_crit) * m_magmaThicknessCoeff;
      } else {
         thickness = m_maxBasalticCrustThickness * ((WLS - m_WLS_onset) / ( m_WLS_crit - m_WLS_onset));
      }
      
      if( thickness < 0 ) {
         thickness = 0;
      }
   }
   return thickness;
}
//------------------------------------------------------------//
void LinearFunction::printCoeffs() {

   cout << "m1 = " << m_m1 << endl;
   cout << "m2 = " << m_m2 << endl;
   cout << "c2 = " << m_c2 << endl;
}

