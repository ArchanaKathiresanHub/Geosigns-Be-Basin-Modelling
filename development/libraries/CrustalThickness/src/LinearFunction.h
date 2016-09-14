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

/// @class LinearFunction A linear function used by the CTC McKenzieCalculator
class LinearFunction {

   public:
   
      LinearFunction ();
      ~LinearFunction () {};

      /// @defgroup Mutators
      /// @{
      void setTTS_crit                 ( const double inTTS );
      void setTTS_onset                ( const double inTTS );
      void setM1                       ( const double inM1 );
      void setM2                       ( const double inM2 );
      void setC2                       ( const double inC2 );
      void setMaxBasalticCrustThickness( const double inMaxBasalticCrustThickness );
      void setMagmaThicknessCoeff      ( const double inMagmaThicknessCoeff );
      /// @}
   
      /// @defgroup Accessors
      /// @{
      double getTTS_crit                 () const;
      double getTTS_onset                () const;
      double getM1                       () const;
      double getM2                       () const;
      double getC2                       () const;
      double getMaxBasalticCrustThickness() const;
      double getMagmaThicknessCoeff      () const;
      /// @}

      /// @brief Compute the thinning factor according to the incrementalTTS value
      /// @param[in] incrementalTTS the incremental total tectonic subsidence
      /// @return the thinning factor (TF)
      double getCrustTF( const double incrementalTTS ) const;

      /// @brief Compute the basalt thickness according to the incrementalTTS value
      /// @param[in] incrementalTTS the incremental total tectonic subsidence
      /// @return the basalt thickness
      double getBasaltThickness( const double incrementalTTS ) const;
     
      /// @brief Print m1, m2 and c2 coefficients
      void printCoeffs() const;

   private:
   
      double m_TTS_crit;    ///< The critical total tectonic subsidence
      double m_TTS_onset;   ///< The total tectonic subsidence at melting point
      
      /// @defgroup Thinning factor coefficients
      /// @{
      double m_m1; ///< Pre-melt coefficient which defines the thinning factor linear function such as TF(x)=m_m1*x
      double m_m2; ///< Post-melt coefficient which defines the thinning factor linear function such as TF(x)=m_m2*x+m_c2
      double m_c2; ///< Post-melt intercept which defines the thinning factor linear function such as TF(x)=m_m2*x+m_c2
      /// @}

      double m_maxBasalticCrustThickness;   ///< The maximum oceanic (basaltic) crust thickness
      double m_magmaThicknessCoeff;         ///< The asthenospheric mantle (magma) thickness coefficient
};

//------------------------------------------------------------//

inline void LinearFunction::setTTS_crit( const double inTTS ) {
   m_TTS_crit = inTTS;
}

inline void LinearFunction::setTTS_onset( const double inTTS ) {
   m_TTS_onset = inTTS;
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

inline double LinearFunction::getTTS_crit() const {
   return m_TTS_crit;
}

inline double LinearFunction::getTTS_onset() const{
   return m_TTS_onset;
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

