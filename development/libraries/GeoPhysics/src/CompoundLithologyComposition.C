//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "CompoundLithologyComposition.h"
#include <stdio.h>

GeoPhysics::CompoundLithologyComposition::CompoundLithologyComposition ():
   m_percent1 (0.0),
   m_percent2 (0.0),
   m_percent3 (0.0),
   m_thermModel ("")
{

}

//------------------------------------------------------------//

GeoPhysics::CompoundLithologyComposition::CompoundLithologyComposition ( const std::string& lithoName1,
                                                                         const std::string& lithoName2,
                                                                         const std::string& lithoName3, 
                                                                         const double       p1,
                                                                         const double       p2,
                                                                         const double       p3, 
                                                                         const std::string& lithoMixModel,
                                                                         const float        layeringIndex) {

   setComposition ( lithoName1, lithoName2, lithoName3, p1, p2, p3, lithoMixModel, layeringIndex );

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyComposition::setComposition ( const std::string& lithoName1,
                                                                const std::string& lithoName2,
                                                                const std::string& lithoName3, 
                                                                const double       p1,
                                                                const double       p2,
                                                                const double       p3, 
                                                                const std::string& lithoMixModel,
                                                                const float        layeringIndex) {


  m_lythoType1 = lithoName1;
  m_lythoType2 = lithoName2;
  m_lythoType3 = lithoName3;
  m_percent1 = p1;
  m_percent2 = p2;
  m_percent3 = p3;
  m_mixModel = lithoMixModel;
  m_mixLayeringIndex = layeringIndex;

  if ( m_lythoType1 > m_lythoType2 ){
    std::swap ( m_lythoType1, m_lythoType2 );
    std::swap ( m_percent1, m_percent2 );
  }

  if ( m_lythoType2 > m_lythoType3 ){
    std::swap ( m_lythoType2, m_lythoType3 );
    std::swap ( m_percent2, m_percent3 );
  }

  if ( m_lythoType1 > m_lythoType2 ){
    std::swap ( m_lythoType1, m_lythoType2 );
    std::swap ( m_percent1, m_percent2 );
  }

}

//------------------------------------------------------------//

const std::string& GeoPhysics::CompoundLithologyComposition::lithologyName ( const int whichSimpleLithology ) const {

  if ( whichSimpleLithology == 1 ) {
    return m_lythoType1;
  } else if ( whichSimpleLithology == 2 ) {
    return m_lythoType2;
  } else { // whichSimpleLithology == 3
    return m_lythoType3;
  } 

}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithologyComposition::lithologyFraction ( const int whichSimpleLithology ) const {

  if ( whichSimpleLithology == 1 ) {
    return m_percent1;
  } else if ( whichSimpleLithology == 2 ) {
    return m_percent2;
  } else { // whichSimpleLithology == 3
    return m_percent3;
  } 

}

//------------------------------------------------------------//

const std::string& GeoPhysics::CompoundLithologyComposition::mixingModel () const {
  return m_mixModel;
}
//------------------------------------------------------------//

float GeoPhysics::CompoundLithologyComposition::layeringIndex() const {
	return m_mixLayeringIndex ;
}
//------------------------------------------------------------//

const std::string& GeoPhysics::CompoundLithologyComposition::thermalModel () const {
  return m_thermModel;
}
//------------------------------------------------------------//

void  GeoPhysics::CompoundLithologyComposition::setThermalModel ( const std::string& aThermalModel ) {
  m_thermModel = aThermalModel;
}

//------------------------------------------------------------//

std::string GeoPhysics::CompoundLithologyComposition::returnKeyString () const{

  std::string Key_String = "";

  Key_String += m_lythoType1;
  Key_String += "_";
  Key_String += m_lythoType2;
  Key_String += "_";
  Key_String += m_lythoType3;
  Key_String += "_";
  char char_percent1[8]; sprintf(char_percent1,"%7.3f",m_percent1);
  char char_percent2[8]; sprintf(char_percent2,"%7.3f",m_percent2);
  char char_percent3[8]; sprintf(char_percent3,"%7.3f",m_percent3);
  Key_String += char_percent1;
  Key_String += "_";
  Key_String += char_percent2;
  Key_String += "_";
  Key_String += char_percent3;
  Key_String += "_";
  Key_String += m_mixModel;  

  return Key_String;

}

//------------------------------------------------------------//

bool GeoPhysics::operator< (const CompoundLithologyComposition& lhs, const CompoundLithologyComposition& rhs) {
  
  std::string rhs_string = rhs.returnKeyString();
  std::string lhs_string = lhs.returnKeyString();

  return ( (lhs_string < rhs_string) );
}

