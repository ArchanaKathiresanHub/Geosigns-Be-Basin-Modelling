//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_LINEARFUNCTION_H_
#define _CRUSTALTHICKNESS_LINEARFUNCTION_H_

/// @class LinearFunction Stores the linear functions used by the CTC
class LinearFunction {

   public:
   
      LinearFunction ();
      ~LinearFunction () {};

      /// @defgroup Mutators
      /// @{
      void setWLS_crit                 ( const double inWLS );
      void setWLS_onset                ( const double inWLS );
      void setM1                       ( const double inM1 );
      void setM2                       ( const double inM2 );
      void setC2                       ( const double inC2 );
      void setMaxBasalticCrustThickness( const double inMaxBasalticCrustThickness );
      void setMagmaThicknessCoeff      ( const double inMagmaThicknessCoeff );
      /// @}
   
      /// @defgroup Accessors
      /// @{
      double getWLS_crit                 () const;
      double getWLS_onset                () const;
      double getM1                       () const;
      double getM2                       () const;
      double getC2                       () const;
      double getMaxBasalticCrustThickness() const;
      double getMagmaThicknessCoeff      () const;
      /// @}

      /// @brief Compute the thinning factor according to the WLS value
      /// @param WLS the water loaded subsidence
      /// @return the thinning factor (TF)
      double getCrustTF        ( const double WLS );

      /// @brief Compute the basalt factor according to the WLS value
      /// @param WLS the water loaded subsidence
      /// @return the basalt thickness
      double getBasaltThickness( const double WLS );
     
      /// @brief Print m1, m2 and c2 coefficients
      void printCoeffs();

   private:
   
      double m_WLS_crit;    ///< The maximum water loaded subsidence
      double m_WLS_onset;   ///< The initial water loaded subsidence at the beginning of the rifting
      
      /// @defgroup Thinning factor coefficients
      /// @{
      double m_m1;
      double m_m2;          
      double m_c2;
      /// @}

      double m_maxBasalticCrustThickness;   ///< The maximum oceanic (basaltic) crust thickness
      double m_magmaThicknessCoeff;         ///< The asthenospheric mantle (magma) thickness coefficient
};

//------------------------------------------------------------//

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

