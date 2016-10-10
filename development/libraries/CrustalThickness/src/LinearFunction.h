//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _LINEAR_FUNCTION_H_
#define _LINEAR_FUNCTION_H_

class LinearFunction {

public:

   LinearFunction ();

   ~LinearFunction () {};

   
private:

   double m_WLS_crit;
   double m_WLS_onset;
   double m_m1;
   double m_m2;
   double m_c2;
   double m_maxBasalticCrustThickness;
   double m_magmaThicknessCoeff;
  
public:
   void setWLS_crit( const double inWLS );
   void setWLS_onset( const double inWLS );
   void setM1( const double inM1 );
   void setM2( const double inM2 );
   void setC2( const double inC2 );
   void setMaxBasalticCrustThickness( const double inMaxBasalticCrustThickness );
   void setMagmaThicknessCoeff( const double inMagmaThicknessCoeff );

   double getWLS_crit() const;
   double getWLS_onset() const;
   double getM1() const;
   double getM2() const;
   double getC2() const;
   double getMaxBasalticCrustThickness() const;
   double getMagmaThicknessCoeff() const;

   double getCrustTF( const double WLS );
   double getBasaltThickness( const double WLS );
  
   void printCoeffs();
};

inline void LinearFunction::setWLS_crit( const double inWLS ) {
   m_WLS_crit = inWLS;
}

inline void LinearFunction::setWLS_onset( const double inWLS ) {
   m_WLS_onset = inWLS;
}

inline void LinearFunction::setM1( const double inM1 ) {
   m_m1 = inM1;
}

inline void LinearFunction::setM2( const double inM2 ) {
   m_m2 = inM2;
}

inline void LinearFunction::setC2( const double inC2 ) {
   m_c2 = inC2;
}

inline void LinearFunction::setMaxBasalticCrustThickness( const double inMaxBasalticCrustThickness ) {
   m_maxBasalticCrustThickness = inMaxBasalticCrustThickness;
}

inline void LinearFunction::setMagmaThicknessCoeff( const double inMagmaThicknessCoeff ) {
   m_magmaThicknessCoeff = inMagmaThicknessCoeff;
}
inline double LinearFunction::getWLS_crit() const {
   return m_WLS_crit;
}
inline double LinearFunction::getWLS_onset() const{
   return m_WLS_onset;
}
inline double LinearFunction::getM1() const{
   return m_m1;
}
inline double LinearFunction::getM2() const{
   return m_m2;
}
inline double LinearFunction::getC2() const{
   return m_c2;
}
inline double LinearFunction::getMaxBasalticCrustThickness() const{
   return m_maxBasalticCrustThickness;
}
inline double LinearFunction::getMagmaThicknessCoeff() const{
   return m_magmaThicknessCoeff;
}

#endif

