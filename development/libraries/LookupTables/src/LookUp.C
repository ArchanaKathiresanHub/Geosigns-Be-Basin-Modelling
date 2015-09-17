/*
 * Copyright (C) 2015 Shell International Exploration & Production.
 * All rights reserved.
 *
 * Developed under license for Shell by PDS B.V.
 *
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell
 */

#include "LookUp.h"
#include <algorithm>
#include <iostream>
#include <math.h>

using namespace std;
using namespace TSR_Tables;

LookUp::LookUp( std::vector<double> const &pressure /* rows */, std::vector<double> const &temperature /* columns */ )
    : m_pressure(pressure), m_temperature(temperature)
{
   std::sort(m_pressure.begin(), m_pressure.end());
   std::sort(m_temperature.begin(), m_temperature.end());
    
   m_data.resize(m_pressure.size() * m_temperature.size());
}

LookUp::LookUp(std::vector<double> const &pressure /* rows */, std::vector<double> const &temperature /* columns */, 
               std::vector<double> const &values ,
               std::vector<float> const &phases )
    : m_pressure(pressure), m_temperature(temperature) 
{
   std::sort(m_pressure.begin(), m_pressure.end());
   std::sort(m_temperature.begin(), m_temperature.end());
   
   for ( int i = 0; i < m_pressure.size() * m_temperature.size(); ++ i ) {
      m_data.push_back( make_pair( values[i], phases[i] ));
   }
}

bool LookUp::checkValidity() const 
{
  
   if( m_data.size() != m_pressure.size() * m_temperature.size() ){
      return false;
   }
   for ( unsigned int i = 0; i < m_pressure.size() * m_temperature.size(); ++ i ) {
      if( m_data[i].first < 0 ) {
         if( m_data[i].second > 0 ) {
            return false;
         }
      }
      if( m_data[i].second < 0 ) {
         if( m_data[i].first > 0 ) {
            return false;
         }
      }
   }  
   return true;
}

void LookUp::printValue( const double p1, const double t1 ) const
{
   dataValue val;
   //  lookup.printTable();
   get( p1, t1, val );
   
   std::cout << "Val(" << p1 << "," << t1 << ") = " << val.first << " " << val.second << std::endl;
}

void LookUp::printTable() const
{
   std::cout << "\t";
   for( unsigned int i = 0; i < m_temperature.size(); ++ i ) {
      std::cout << m_temperature [i] << "\t";
   }
   std::cout << std::endl;
   
   unsigned k = 0;
   for( unsigned int i = 0; i < m_pressure.size(); ++ i ) {
      std::cout << m_pressure [i] << "\t";
      for( unsigned int j = 0; j < m_temperature.size(); ++ j ) {
         
         std::cout <<  m_data[k].first << " " << m_data[k].second << "\t";
         ++ k;
      }
      std::cout << std::endl;
   }
   std::cout << std::endl;
}

dataValue LookUp::getValue( int i, int j ) const
{
#ifdef DEBUG    
   std::cout << "data(" << i << "," << j << ") = " << m_data [ i * 4 + j ] << std::endl;
#endif
   
   return m_data [ i * m_temperature.size() + j ];
}

bool LookUp::get(double p, double t, dataValue &value) const
{
   std::vector<double>::const_iterator p_pos = std::lower_bound(m_pressure.begin(), m_pressure.end(), p);
   if(p_pos == m_pressure.end())
      return false;
   
   std::vector<double>::const_iterator t_pos = std::lower_bound(m_temperature.begin(), m_temperature.end(), t);
   if(t_pos == m_temperature.end())
      return false;
   
#ifdef DEBUG
   std::cout << "*p_pos == " << *p_pos << ", *t_pos == " << *t_pos << std::endl;
   std::cout << " p_ind == " << p_pos - m_pressure.begin() << ", t_ind == " << t_pos - m_temperature.begin() << std::endl;
#endif
   
   const double MinOffset = 1e-4;
   
   int p_ind1 = p_pos - m_pressure.begin() ;
   int t_ind1 = t_pos - m_temperature.begin();
   
   int p_ind = p_ind1, t_ind = t_ind1;
   
   const double x2 = m_pressure[p_ind1];
   const double y2 = m_temperature[t_ind1];
   
   if( p_ind1 > 0 and fabs( x2 - p ) > MinOffset ) {
      p_ind = p_ind1 - 1; 
   }
   
   if( t_ind1 > 0 and fabs( y2 - t ) > MinOffset ) {
      t_ind = t_ind1 - 1; 
   }
   
#ifdef DEBUG    
   std::cout << " p_ind1 == " << p_ind1 << ", t_ind1 == " << t_ind1 << std::endl;
   std::cout << " t_ind == " << t_ind << ", t_ind1 == " << t_ind1 << std::endl;
#endif
   
   const double x1 = m_pressure[p_ind];
   const double y1 = m_temperature[t_ind];
   
   double fractionP, fractionT;
   
   const float undefined = -1;
   
   if(( x2 - x1 ) <= MinOffset ) {
      fractionP = 0;
   } else {
      fractionP = ( p - x1 ) / ( x2 - x1 );
      if ( fractionP <= MinOffset ) {
         fractionP = 0;
      } else if ( fractionP >= ( 1 - MinOffset )) {
         fractionP = 1;
      }
   }
   
   if(( y2 - y1 ) <= MinOffset ) {
      fractionT = 0;
   }  else {
      fractionT = ( t - y1 ) / ( y2 - y1 );
      if ( fractionT <= MinOffset ) {
         fractionT = 0;       
      } else if ( fractionT >= ( 1 - MinOffset )) {
         fractionT = 1;
      }
   } 
   
   const dataValue value00 = getValue( p_ind, t_ind );
   
   if ( fractionP == 0 and fractionT == 0 ) {
      value.first = value00.first;
      value.second = value00.second;
      
      return value.second != undefined;
   }
   
   const dataValue value01 = getValue( p_ind, t_ind1 );
   const dataValue value10 = getValue( p_ind1, t_ind );
   const dataValue value11 = getValue( p_ind1, t_ind1 );
   
   if( value00.second == undefined or value01.second == undefined or
       value10.second == undefined or value11.second == undefined ) {
      
      value.first = -1;
      value.second = -1;
      return false;
   }
   
   value.first = value00.first * ( 1 - fractionT ) *( 1 - fractionP ) + 
      value01.first * ( 1 - fractionP ) * fractionT +  
      value10.first * fractionP * ( 1 - fractionT ) +
      value11.first * fractionP * fractionT;
   
   value.second = value00.second * ( 1 - fractionT ) * ( 1 - fractionP ) + 
      value01.second * ( 1 - fractionP ) * fractionT +  
      value10.second * fractionP * ( 1 - fractionT ) +
      value11.second * fractionP * fractionT;
     
   return true;
}


// Local Variables:
// mode: c++
// c-basic-offset: 3
// tab-width: 3
// indent-tabs-mode: nil
// End:

