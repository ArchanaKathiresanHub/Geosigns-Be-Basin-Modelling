//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_LINEARFUNCTION_H
#define CRUSTALTHICKNESS_LINEARFUNCTION_H

namespace CrustalThickness
{
   /// @class LinearFunction A linear function used by the CTC McKenzieCalculator
   class LinearFunction {

   public:

      LinearFunction();
      ~LinearFunction() {
         // Empty constructor (comment removes SonarQube issue)
      };

      /// @defgroup Mutators
      /// @{
      void setTTS_crit( const double inTTS );
      void setTTS_onset( const double inTTS );
      void setM1( const double inM1 );
      void setM2( const double inM2 );
      void setC2( const double inC2 );
      void setMaxBasalticCrustThickness( const double inMaxBasalticCrustThickness );
      void setMagmaThicknessCoeff( const double inMagmaThicknessCoeff );
      /// @}

      /// @defgroup Accessors
      /// @{
      double getTTS_crit() const;
      double getTTS_onset() const;
      double getM1() const;
      double getM2() const;
      double getC2() const;
      double getMaxBasalticCrustThickness() const;
      double getMagmaThicknessCoeff() const;
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

      double m_TTS_crit;  ///< The critical total tectonic subsidence         [m]
      double m_TTS_onset; ///< The total tectonic subsidence at melting point [m]

      /// @defgroup Thinning factor coefficients
      /// @{
      double m_m1; ///< Pre-melt coefficient which defines the thinning factor linear function such as TF(x)=m_m1*x
      double m_m2; ///< Post-melt coefficient which defines the thinning factor linear function such as TF(x)=m_m2*x+m_c2
      double m_c2; ///< Post-melt intercept which defines the thinning factor linear function such as TF(x)=m_m2*x+m_c2
      /// @}

      double m_maxBasalticCrustThickness;   ///< The maximum oceanic (basaltic) crust thickness
      double m_magmaThicknessCoeff;         ///< The asthenospheric mantle (magma) thickness coefficient
   };

} // End namespace CrustalThickness

//------------------------------------------------------------//

inline void CrustalThickness::LinearFunction::setTTS_crit( const double inTTS ) {
   m_TTS_crit = inTTS;
}

inline void CrustalThickness::LinearFunction::setTTS_onset( const double inTTS ) {
   m_TTS_onset = inTTS;
}

inline void CrustalThickness::LinearFunction::setM1( const double inM1 ) {
   m_m1 = inM1;
}

inline void CrustalThickness::LinearFunction::setM2( const double inM2 ) {
   m_m2 = inM2;
}

inline void CrustalThickness::LinearFunction::setC2( const double inC2 ) {
   m_c2 = inC2;
}

inline void CrustalThickness::LinearFunction::setMaxBasalticCrustThickness( const double inMaxBasalticCrustThickness ) {
   m_maxBasalticCrustThickness = inMaxBasalticCrustThickness;
}

inline void CrustalThickness::LinearFunction::setMagmaThicknessCoeff( const double inMagmaThicknessCoeff ) {
   m_magmaThicknessCoeff = inMagmaThicknessCoeff;
}

inline double CrustalThickness::LinearFunction::getTTS_crit() const {
   return m_TTS_crit;
}

inline double CrustalThickness::LinearFunction::getTTS_onset() const{
   return m_TTS_onset;
}

inline double CrustalThickness::LinearFunction::getM1() const{
   return m_m1;
}

inline double CrustalThickness::LinearFunction::getM2() const{
   return m_m2;
}

inline double CrustalThickness::LinearFunction::getC2() const{
   return m_c2;
}

inline double CrustalThickness::LinearFunction::getMaxBasalticCrustThickness() const{
   return m_maxBasalticCrustThickness;
}

inline double CrustalThickness::LinearFunction::getMagmaThicknessCoeff() const{
   return m_magmaThicknessCoeff;
}

#endif

