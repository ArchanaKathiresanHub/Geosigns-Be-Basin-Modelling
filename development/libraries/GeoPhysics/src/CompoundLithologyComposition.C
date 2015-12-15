#include "CompoundLithologyComposition.h"
#include <stdio.h>

GeoPhysics::CompoundLithologyComposition::CompoundLithologyComposition () {
   percent1 = 0.0;
   percent2 = 0.0;
   percent3 = 0.0;

   thermModel = "";
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithologyComposition::CompoundLithologyComposition ( const std::string& lithoName1,
                                                                         const std::string& lithoName2,
                                                                         const std::string& lithoName3, 
                                                                         const double       p1,
                                                                         const double       p2,
                                                                         const double       p3, 
                                                                         const std::string& lithoMixModel ) {

   setComposition ( lithoName1, lithoName2, lithoName3, p1, p2, p3, lithoMixModel );

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyComposition::setComposition ( const std::string& lithoName1,
                                                                const std::string& lithoName2,
                                                                const std::string& lithoName3, 
                                                                const double       p1,
                                                                const double       p2,
                                                                const double       p3, 
                                                                const std::string& lithoMixModel ) {


  lythoType1 = lithoName1;
  lythoType2 = lithoName2;
  lythoType3 = lithoName3;
  percent1 = p1;
  percent2 = p2;
  percent3 = p3;
  mixModel = lithoMixModel;

  if ( lythoType1 > lythoType2 ){
    std::swap ( lythoType1, lythoType2 );
    std::swap ( percent1, percent2 );
  }

  if ( lythoType2 > lythoType3 ){
    std::swap ( lythoType2, lythoType3 );
    std::swap ( percent2, percent3 );
  }

  if ( lythoType1 > lythoType2 ){
    std::swap ( lythoType1, lythoType2 );
    std::swap ( percent1, percent2 );
  }

}

//------------------------------------------------------------//

const std::string& GeoPhysics::CompoundLithologyComposition::lithologyName ( const int whichSimpleLithology ) const {

  if ( whichSimpleLithology == 1 ) {
    return lythoType1;
  } else if ( whichSimpleLithology == 2 ) {
    return lythoType2;
  } else { // whichSimpleLithology == 3
    return lythoType3;
  } 

}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithologyComposition::lithologyFraction ( const int whichSimpleLithology ) const {

  if ( whichSimpleLithology == 1 ) {
    return percent1;
  } else if ( whichSimpleLithology == 2 ) {
    return percent2;
  } else { // whichSimpleLithology == 3
    return percent3;
  } 

}

//------------------------------------------------------------//

const std::string& GeoPhysics::CompoundLithologyComposition::mixingModel () const {
  return mixModel;
}
//------------------------------------------------------------//

const std::string& GeoPhysics::CompoundLithologyComposition::thermalModel () const {
  return thermModel;
}
//------------------------------------------------------------//

void  GeoPhysics::CompoundLithologyComposition::setThermalModel ( const std::string& aThermalModel ) {
  thermModel = aThermalModel;
}

//------------------------------------------------------------//

std::string GeoPhysics::CompoundLithologyComposition::returnKeyString () const{

  std::string Key_String = "";

  Key_String += lythoType1;
  Key_String += "_";
  Key_String += lythoType2;
  Key_String += "_";
  Key_String += lythoType3;
  Key_String += "_";
  char char_percent1[8]; sprintf(char_percent1,"%7.3f",percent1);
  char char_percent2[8]; sprintf(char_percent2,"%7.3f",percent2);
  char char_percent3[8]; sprintf(char_percent3,"%7.3f",percent3);
  Key_String += char_percent1;
  Key_String += "_";
  Key_String += char_percent2;
  Key_String += "_";
  Key_String += char_percent3;
  Key_String += "_";
  Key_String += mixModel;  

  return Key_String;

}

//------------------------------------------------------------//

bool GeoPhysics::operator< (const CompoundLithologyComposition& lhs, const CompoundLithologyComposition& rhs) {
  
  std::string rhs_string = rhs.returnKeyString();
  std::string lhs_string = lhs.returnKeyString();

  return ( (lhs_string < rhs_string) );
}

