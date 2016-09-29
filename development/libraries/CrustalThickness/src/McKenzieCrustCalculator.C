//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "McKenzieCrustCalculator.h"

// Geophysics library
#include "GeoPhysicalConstants.h"

// utilities library
#include "NumericFunctions.h"

McKenzieCrustCalculator::McKenzieCrustCalculator(
   const InterfaceInput&    inputData,
   AbstractInterfaceOutput& outputData,
   const AbstractValidator& validator,
   const double age,
   const Interface::GridMap* previousThinningFactor,
   const Interface::GridMap* previousContinentalCrustThickness,
   const Interface::GridMap* previousOceanicCrustThickness ) :
      m_constants     ( inputData.getConstants()     ),
      m_depthBasement ( inputData.getDepthBasement() ),
      m_HCuMap        ( inputData.getHCuMap()    ),
      m_HBuMap        ( inputData.getHBuMap(age) ),
      m_HLMuMap       ( inputData.getHLMuMap()   ),
      m_riftStartAge  ( inputData.getRiftingStartAge(age) ),
      m_riftEndAge    ( inputData.getRiftingEndAge  (age) ),
      m_firstI        ( inputData.firstI() ),
      m_firstJ        ( inputData.firstJ() ),
      m_lastI         ( inputData.lastI()  ),
      m_lastJ         ( inputData.lastJ()  ),
      m_continentalCrustRatio( inputData.getContinentalCrustRatio() ),
      m_oceanicCrustRatio    ( inputData.getOceanicCrustRatio()     ),
      m_previousThinningFactor           ( previousThinningFactor            ),
      m_previousContinentalCrustThickness( previousContinentalCrustThickness ),
      m_previousOceanicCrustThickness    ( previousOceanicCrustThickness     ),
      m_outputData ( outputData  ),
      m_validator  ( validator   )
{
   // Check the inputs
   // In normal behavior these exceptions will be handled before the creation of the McKenzieCalculator by the inputData object itself
   if (m_depthBasement == nullptr){
      throw std::invalid_argument( "Basement depth provided by the interface input is a null pointer" );
   }
   else if (m_constants.getModelTotalLithoThickness() == 0.0){
      throw std::invalid_argument( "Total lithosphere thickness provided by the interface input is equal to 0 and will lead to divisions by 0" );
   }
   else if (m_constants.getBackstrippingMantleDensity() == m_constants.getWaterDensity()){
      throw std::invalid_argument( "Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0" );
   }
   else if (m_constants.getDecayConstant() == 0.0){
      throw std::invalid_argument( "Decay constant provided by the interface input is 0 and will lead to divisions by 0" );
   }
   else if (m_constants.getTau() == 0.0){
      throw std::invalid_argument( "Tau provided by the interface input is 0 and will lead to divisions by 0" );
   }
   else if (not NumericFunctions::inRange(m_continentalCrustRatio, 0.0, 1.0)) {
      throw std::invalid_argument( "The continental crust ratio (which defines the lower and upper continental crust) provided by the interface input is outisde the range [0,1]" );
   }
   else if (not NumericFunctions::inRange( m_oceanicCrustRatio, 0.0, 1.0 )) {
      throw std::invalid_argument( "The oceanic crust ratio (which defines the lower and upper oceanic crust) provided by the interface input is outisde the range [0,1]" );
   }
   //if one previous grid data is not nullptr then they all shouldn't be nullptr
   else if ( m_previousThinningFactor         != nullptr
      or m_previousContinentalCrustThickness  != nullptr
      or m_previousOceanicCrustThickness      != nullptr ){
      if (m_previousThinningFactor == nullptr){
         throw std::invalid_argument( "The previous thinning factor provided to the McKenzie calculator is a null pointer but some other previous data are not null" );
      }
      else if (m_previousContinentalCrustThickness == nullptr){
         throw std::invalid_argument( "The previous continental crust thickness provided to the McKenzie calculator is a null pointer but some other previous data are not null" );
      }
      else if (m_previousOceanicCrustThickness == nullptr){
         throw std::invalid_argument( "The previous oceanic crust thickness provided to the McKenzie calculator is a null pointer but some other previous data are not null" );
      }
   }
   else if (m_riftStartAge<=0){
      throw std::invalid_argument( "The beginning of the rifting event is inferior or equal to 0 for age " + std::to_string( age ) + "Ma");
   }
   else if (m_riftStartAge == DataAccess::Interface::DefaultUndefinedScalarValue){
      throw std::invalid_argument( "The beginning of the rifting event is undefined for age " + std::to_string( age ) + "Ma" );
   }
   else if (m_riftEndAge<0){
      throw std::invalid_argument( "The end of the rifting event is inferior to 0 for age " + std::to_string( age ) + "Ma");
   }
   else if (m_riftEndAge == DataAccess::Interface::DefaultUndefinedScalarValue){
      throw std::invalid_argument( "The end of the rifting event is undefined for age " + std::to_string( age ) + "Ma" );
   }
   else if (m_riftEndAge >= m_riftStartAge){
      throw std::invalid_argument( "The end of the rifting event is supperior or equal to its end for age " + std::to_string( age ) + "Ma");
   }
};

//------------------------------------------------------------//
void McKenzieCrustCalculator::checkInputValues (const unsigned int i, const unsigned int j ) const{
   if (m_HCuMap.getValue( i, j ) <= 0
      and m_HCuMap.getValue( i, j ) != m_HCuMap.getUndefinedValue()) {
      throw std::invalid_argument( "Initial continental crust thickness is inferior or equal to 0 for node ("
         + std::to_string( i ) + "," + std::to_string( j ) + ")" );
   }
   if (m_HLMuMap.getValue( i, j ) <= 0
      and m_HLMuMap.getValue( i, j ) != m_HLMuMap.getUndefinedValue()) {
      throw std::invalid_argument( "Initial lithospheric mantle thickness is inferior or equal to 0 for node ("
         + std::to_string( i ) + "," + std::to_string( j ) + ")" );
   }
   if (m_HCuMap.getValue( i, j ) + m_HLMuMap.getValue( i, j ) <= 0
      and m_HCuMap.getValue( i, j )  != m_HCuMap.getUndefinedValue() 
      and m_HLMuMap.getValue( i, j ) != m_HLMuMap.getUndefinedValue()){
      throw std::invalid_argument( "Initial lithospheric thickness (crust + mantle) is inferior or equal to 0 for node ("
         + std::to_string( i ) + "," + std::to_string( j ) + ")" );
   }
   if (m_HBuMap.getValue( i, j ) < 0
      and m_HBuMap.getValue( i, j ) != m_HBuMap.getUndefinedValue()) {
      throw std::invalid_argument( "Maximum oceanic (basaltic) crust thickness is inferior to 0 for node ("
         + std::to_string( i ) + "," + std::to_string( j ) + ")" );
   }
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateContinentalCrustDensity( const double initialContinentalCrustThickness ) const{
   // average uppermost mantle density 
   const double mantleDensityAV = m_constants.getLithoMantleDensity() * (1 - (m_constants.getCoeffThermExpansion() * m_constants.getBaseLithosphericTemperature() / 2)
      * ((m_constants.getReferenceCrustThickness() + initialContinentalCrustThickness) / m_constants.getModelTotalLithoThickness()));
   // estimated continental crust density
   assert(initialContinentalCrustThickness != 0);
   const double modelCrustDensity = (m_constants.getReferenceCrustDensity() * m_constants.getReferenceCrustThickness()
      + mantleDensityAV * (initialContinentalCrustThickness - m_constants.getReferenceCrustThickness())) / initialContinentalCrustThickness;
   return modelCrustDensity;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateAstenospherePotentialTemperature( const double maxOceanicCrustThickness ) const{
   // asthenosphere potential temperature
   double potentialTempAstheno = m_constants.getB() + m_constants.getA() * std::sqrt( maxOceanicCrustThickness );
   return potentialTempAstheno;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateThinningFactorOnset( const double potentialTempAstheno, const double initialLithosphericThickness ) const{
   const double heightSolidus = m_constants.getC() * potentialTempAstheno + m_constants.getD();
   // crustal thinning factor at melt onset
   assert( initialLithosphericThickness != 0 );
   const double thinningFactorOnset = 1 - heightSolidus / initialLithosphericThickness;
   return thinningFactorOnset;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateThinningFactorOnsetLinearized( const double thinningFactorOnset) const {
   // liner approximation of thinning factor at melt onset
   const double thinningFactorOnsetLinearized = (1 + 2 * thinningFactorOnset) / 3;
   return thinningFactorOnsetLinearized;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateThinningFactorOnsetAtMaxBasalt( const double maxOceanicCrustThickness, const double thinningFactorOnset ) const{
   // crustal thinning factor at threshold
   double thinningFactorOnsetAtMaxBasalt;
   if (maxOceanicCrustThickness == 0) {
      thinningFactorOnsetAtMaxBasalt = thinningFactorOnset;
   }
   else {
      thinningFactorOnsetAtMaxBasalt = thinningFactorOnset + ((1 - thinningFactorOnset) * sqrt( 2000 / maxOceanicCrustThickness ));
   }
   return thinningFactorOnsetAtMaxBasalt;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateMagmaDensity( const double maxOceanicCrustThickness ) const{
   double magmaticDensity = m_constants.getE() + (m_constants.getF() - m_constants.getE())
      * (1 - exp( -1 * maxOceanicCrustThickness / m_constants.getDecayConstant() ));
   if ( maxOceanicCrustThickness != 0 and ( m_constants.getBackstrippingMantleDensity() - magmaticDensity ) == 0.0 ) {
      throw McKenzieException() << "Your mantle density (from configuration file) is equal to your magmatic density (from computation)";
   }
   return magmaticDensity;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateTTSexhume( const double averageRiftTime ) const{
   const double expValue = 1 - exp( -15 * averageRiftTime / m_constants.getTau() );
   const double TTSexhume = m_constants.getInitialSubsidenceMax() + m_constants.getE0()
      * ((1 - exp( -averageRiftTime / m_constants.getTau() )) + (GeoPhysics::Pi2by8 - 1) * expValue);
   return TTSexhume;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateTTScritical( const double TTSexhume, const double maxOceanicCrustThickness, const double magmaticDensity ) const{
   const double TTScritical = TTSexhume - maxOceanicCrustThickness * (m_constants.getBackstrippingMantleDensity() - magmaticDensity)
      / (m_constants.getBackstrippingMantleDensity() - m_constants.getWaterDensity());
   return TTScritical;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateTTSOnsetLinearized( const double averageRiftTime,
                                                             const double thinningFactorOnsetLinearized ) const{
   const double r = (thinningFactorOnsetLinearized == 1.0 ?
      1.0 : sin( M_PI * (1 - thinningFactorOnsetLinearized) ) / (M_PI * (1 - thinningFactorOnsetLinearized)));
   const double expValue = 1 - exp( -15 * averageRiftTime / m_constants.getTau() );
   const double TTSOnsetLinearized = thinningFactorOnsetLinearized * m_constants.getInitialSubsidenceMax() + m_constants.getE0()
      * (r * (1 - exp( -averageRiftTime / m_constants.getTau() )) + (GeoPhysics::Pi2by8 * thinningFactorOnsetLinearized - r) * expValue);
   return TTSOnsetLinearized;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateTTSexhumeSerpentinized( const double TTSexhume ) const {
   /// @todo Move to the configuration file if time allowed
   double InitialSubsidenceCorrection = 681.6394;
   return TTSexhume - InitialSubsidenceCorrection;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateITScorrected( const double ITS, const double previousTF ) const{
   return ITS * (1 - previousTF);
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateContinentalCrustThicknessOnset( const double initialContinentalCrustThickness, const double TFOnsetLinearized ) const {
   return initialContinentalCrustThickness * (1 - TFOnsetLinearized);
}

//------------------------------------------------------------//
void McKenzieCrustCalculator::defineLinearFunction(
   LinearFunction & theFunction,
   const double maxBasalticCrustThickness,
   const double magmaticDensity,
   const double thinningFactorOnsetLinearized,
   const double TTSOnsetLinearized,
   const double TTScritical
   ) const
{
   assert( magmaticDensity != m_constants.getBackstrippingMantleDensity() );
   const double magmaThicknessCoeff = (m_constants.getBackstrippingMantleDensity() - m_constants.getWaterDensity())
      / (m_constants.getBackstrippingMantleDensity() - magmaticDensity);
   // form Y = m1 * X
   assert( TTSOnsetLinearized != 0 );
   const double m1 = thinningFactorOnsetLinearized / TTSOnsetLinearized;
   // form Y = m2 * X + c2
   const double m2 = (TTScritical == TTSOnsetLinearized ? 0 : (1 - thinningFactorOnsetLinearized) / (TTScritical - TTSOnsetLinearized));
   const double c2 = thinningFactorOnsetLinearized - TTSOnsetLinearized * m2;

   theFunction.setTTS_crit ( TTScritical        );
   theFunction.setTTS_onset( TTSOnsetLinearized );
   theFunction.setM1( m1 );
   theFunction.setM2( m2 );
   theFunction.setC2( c2 );
   theFunction.setMagmaThicknessCoeff      ( magmaThicknessCoeff       );
   theFunction.setMaxBasalticCrustThickness( maxBasalticCrustThickness );
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateResidualDepthAnomaly( const double TTScritial, const double TTSadjusted ) const {
   return TTScritial - TTSadjusted;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateContinentalCrustalThickness( const double thinningFactor, const double previousContinentalCrustThickness ) const {
   double continentalCrusltalThickness = 0;
   if ( thinningFactor <= 1.0 ){
      continentalCrusltalThickness = (1 - thinningFactor) * previousContinentalCrustThickness;
   }
   return continentalCrusltalThickness;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateOceanicCrustalThickness( const double ITSadjusted,
                                                                  const double TTSexhume,
                                                                  const LinearFunction& linearFunction,
                                                                  const double previousOceanicCrustThickness ) const {
   double oceanicCrustalThickness = previousOceanicCrustThickness;
   if (ITSadjusted < TTSexhume){
      oceanicCrustalThickness += linearFunction.getBasaltThickness( ITSadjusted );
   }
   // else, the mantle is at the surface so no basalt layer above it
   return oceanicCrustalThickness;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateTopOceanicCrust( const double continentalCrusltalThickness, const double depthBasement ) const {
   return continentalCrusltalThickness + depthBasement;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateMoho( const double topOceanicCrust, const double oceanicCrustalThickness ) const {
   return topOceanicCrust + oceanicCrustalThickness;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::calculateEffectiveCrustalThickness( const double continentalCrusltalThickness,
                                                                    const double oceanicCrustalThickness,
                                                                    const double initialContinentalCrustThickness,
                                                                    const double initialLithosphericMantleThickness ) const {
   assert( initialLithosphericMantleThickness != 0 );
   double effectiveCrustalThickness = continentalCrusltalThickness + oceanicCrustalThickness
      * (initialContinentalCrustThickness / initialLithosphericMantleThickness);
   return effectiveCrustalThickness;
}

//------------------------------------------------------------//
void McKenzieCrustCalculator::divideCrust(
   CrustType type,
   const double totalCrustThickness,
   double& upperCrustThickness,
   double& lowerCrustThickness ) const {
   double ratio;
   if (type == McKenzieCrustCalculator::CONTINENTAL){
      ratio = m_continentalCrustRatio;
   }
   else if (type == McKenzieCrustCalculator::OCEANIC) {
      ratio = m_oceanicCrustRatio;
   }
   else{
      throw McKenzieException() << "Undefined crust type provided when dividing the crust in its lower and upper part";
   }
   const double factor = totalCrustThickness / (1 + ratio);
   upperCrustThickness = ratio * factor;
   lowerCrustThickness = factor;
}

//------------------------------------------------------------//
void McKenzieCrustCalculator::compute(){

   // initialize all results to 0.0
   double continentalCrustDensity          = 0.0, potentialTempAstheno             = 0.0, magmaticDensity                = 0.0;
   double thinningFactorOnset              = 0.0, thinningFactorOnsetLinearized    = 0.0, thinningFactorOnsetAtMaxBasalt = 0.0;
   double TTSexhume                        = 0.0, TTScritical                      = 0.0, TTSOnsetLinearized             = 0.0, TTSexhumeSerpentinized       = 0.0;
   double thicknessContinetalCrustOnset    = 0.0, residualDepthAnomaly             = 0.0, thinningFactor                 = 0.0;
   double continentalCrustalThickness      = 0.0, oceanicCrustalThickness          = 0.0, topOceanicCrust                = 0.0, effectiveCrustalThickness    = 0.0, moho = 0.0;
   double upperContinentalCrustalThickness = 0.0, lowerContinentalCrustalThickness = 0.0, upperOceanicCrustalThickness   = 0.0, lowerOceanicCrustalThickness = 0.0;
   LinearFunction linearFunction = LinearFunction();

   retrieveData();
   for (unsigned int i = m_firstI; i <= m_lastI; ++i) {
      for (unsigned int j = m_firstJ; j <= m_lastJ; ++j) {
         checkInputValues( i, j );
         const double initialContinentalCrustThickness   = m_HCuMap.getValue ( i, j );
         const double maxBasalticCrustThickness          = m_HBuMap.getValue ( i, j );
         const double initialLithosphericMantleThickness = m_HLMuMap.getValue( i, j );
         if (m_validator.isValid( i, j ) and
             initialContinentalCrustThickness   != m_HCuMap.getUndefinedValue()  and
             maxBasalticCrustThickness          != m_HBuMap.getUndefinedValue()  and
             initialLithosphericMantleThickness != m_HLMuMap.getUndefinedValue())
         {
            //crust densities and temperature
            continentalCrustDensity = calculateContinentalCrustDensity         ( initialContinentalCrustThickness );
            magmaticDensity         = calculateMagmaDensity                    ( maxBasalticCrustThickness        );
            potentialTempAstheno    = calculateAstenospherePotentialTemperature( maxBasalticCrustThickness        );

            //thinning factors
            const double initialLithosphericThickness = initialContinentalCrustThickness + initialLithosphericMantleThickness;
            thinningFactorOnset            = calculateThinningFactorOnset           ( potentialTempAstheno, initialLithosphericThickness );
            thinningFactorOnsetAtMaxBasalt = calculateThinningFactorOnsetAtMaxBasalt( maxBasalticCrustThickness, thinningFactorOnset     );
            thinningFactorOnsetLinearized  = calculateThinningFactorOnsetLinearized ( thinningFactorOnset );

            //McKenzie's Total Tectonic Subsidence (back calculated from linearized thinning factor)
            const double averageRiftTime = (m_riftStartAge + m_riftEndAge) / 2.0;
            TTSexhume              = calculateTTSexhume             ( averageRiftTime );
            TTScritical            = calculateTTScritical           ( TTSexhume, maxBasalticCrustThickness, magmaticDensity );
            TTSOnsetLinearized     = calculateTTSOnsetLinearized    ( averageRiftTime, thinningFactorOnsetLinearized );
            TTSexhumeSerpentinized = calculateTTSexhumeSerpentinized( TTSexhume );

            //McKenzie's linear function
            defineLinearFunction( linearFunction, maxBasalticCrustThickness, magmaticDensity, thinningFactorOnsetLinearized, TTSOnsetLinearized, TTScritical );

            //Crust thicknesses at melt onset (oceanic crustal thickness is implicitly 0)
            thicknessContinetalCrustOnset = calculateContinentalCrustThicknessOnset( initialContinentalCrustThickness, thinningFactorOnsetLinearized );

            // incTTS
            const double TTS = m_outputData.getMapValue( CrustalThicknessInterface::outputMaps::WLSadjustedMap               , i, j );
            const double ITS = m_outputData.getMapValue( CrustalThicknessInterface::outputMaps::incTectonicSubsidenceAdjusted, i, j );
            const double previousTF                        = getPreviousTF( i, j );
            const double previousContinentalCrustThickness = getPreviousContinentalCrustThickness( i, j );
            const double previousOceanicCrustThickness     = getPreviousOceanicCrustThickness( i, j );
            const double depthBasement = m_depthBasement->get( i, j );
            if ( TTS                               != Interface::DefaultUndefinedMapValue  and
                 ITS                               != Interface::DefaultUndefinedMapValue  and
                 previousTF                        != Interface::DefaultUndefinedMapValue  and
                 previousContinentalCrustThickness != Interface::DefaultUndefinedMapValue  and
                 previousOceanicCrustThickness     != Interface::DefaultUndefinedMapValue  and
                 depthBasement                     != m_depthBasement->getUndefinedValue() and
                 TTScritical >= TTSOnsetLinearized ) {

               residualDepthAnomaly         = calculateResidualDepthAnomaly        ( TTScritical, TTS );
               thinningFactor               = linearFunction.getCrustTF            ( ITS );
               continentalCrustalThickness  = calculateContinentalCrustalThickness ( thinningFactor, previousContinentalCrustThickness );
               oceanicCrustalThickness      = calculateOceanicCrustalThickness     ( ITS, TTSexhume, linearFunction, previousOceanicCrustThickness );
               divideCrust( CONTINENTAL, continentalCrustalThickness, upperContinentalCrustalThickness, lowerContinentalCrustalThickness );
               divideCrust( OCEANIC    , oceanicCrustalThickness    , upperOceanicCrustalThickness    , lowerOceanicCrustalThickness     );
               topOceanicCrust              = calculateTopOceanicCrust             ( continentalCrustalThickness, depthBasement );
               moho                         = calculateMoho                        ( topOceanicCrust, oceanicCrustalThickness );
               effectiveCrustalThickness    = calculateEffectiveCrustalThickness   ( continentalCrustalThickness,
                  oceanicCrustalThickness, initialContinentalCrustThickness, initialLithosphericMantleThickness );

            }
            else{
               residualDepthAnomaly             = Interface::DefaultUndefinedMapValue;
               thinningFactor                   = Interface::DefaultUndefinedMapValue;
               continentalCrustalThickness      = Interface::DefaultUndefinedMapValue;
               oceanicCrustalThickness          = Interface::DefaultUndefinedMapValue;
               upperContinentalCrustalThickness = Interface::DefaultUndefinedMapValue;
               lowerContinentalCrustalThickness = Interface::DefaultUndefinedMapValue;
               upperOceanicCrustalThickness     = Interface::DefaultUndefinedMapValue;
               lowerOceanicCrustalThickness     = Interface::DefaultUndefinedMapValue;
               topOceanicCrust                  = Interface::DefaultUndefinedMapValue;
               moho                             = Interface::DefaultUndefinedMapValue;
               effectiveCrustalThickness        = Interface::DefaultUndefinedMapValue;
            }

         }
         else
         {
            continentalCrustDensity        = Interface::DefaultUndefinedMapValue;
            magmaticDensity                = Interface::DefaultUndefinedMapValue;
            potentialTempAstheno           = Interface::DefaultUndefinedMapValue;
            thinningFactorOnset            = Interface::DefaultUndefinedMapValue;
            thinningFactorOnsetAtMaxBasalt = Interface::DefaultUndefinedMapValue;
            thinningFactorOnsetLinearized  = Interface::DefaultUndefinedMapValue;
            TTSexhume                      = Interface::DefaultUndefinedMapValue;
            TTScritical                    = Interface::DefaultUndefinedMapValue;
            TTSOnsetLinearized             = Interface::DefaultUndefinedMapValue;
            TTSexhumeSerpentinized         = Interface::DefaultUndefinedMapValue;
            thicknessContinetalCrustOnset  = Interface::DefaultUndefinedMapValue;
            linearFunction.setM1( Interface::DefaultUndefinedMapValue );
            linearFunction.setM2( Interface::DefaultUndefinedMapValue );
            linearFunction.setC2( Interface::DefaultUndefinedMapValue );
            residualDepthAnomaly             = Interface::DefaultUndefinedMapValue;
            thinningFactor                   = Interface::DefaultUndefinedMapValue;
            continentalCrustalThickness      = Interface::DefaultUndefinedMapValue;
            oceanicCrustalThickness          = Interface::DefaultUndefinedMapValue;
            upperContinentalCrustalThickness = Interface::DefaultUndefinedMapValue;
            lowerContinentalCrustalThickness = Interface::DefaultUndefinedMapValue;
            upperOceanicCrustalThickness     = Interface::DefaultUndefinedMapValue;
            lowerOceanicCrustalThickness     = Interface::DefaultUndefinedMapValue;
            topOceanicCrust                  = Interface::DefaultUndefinedMapValue;
            moho                             = Interface::DefaultUndefinedMapValue;
            effectiveCrustalThickness        = Interface::DefaultUndefinedMapValue;
         }

         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::estimatedCrustDensityMap,       i, j, continentalCrustDensity          );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::basaltDensityMap,               i, j, magmaticDensity                  );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::PTaMap,                         i, j, potentialTempAstheno             );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::TFOnsetMap,                     i, j, thinningFactorOnset              );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::TFOnsetMigMap,                  i, j, thinningFactorOnsetAtMaxBasalt   );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::TFOnsetLinMap,                  i, j, thinningFactorOnsetLinearized    );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSExhumeMap,                   i, j, TTSexhume                        );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSCritMap,                     i, j, TTScritical                      );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSOnsetMap,                    i, j, TTSOnsetLinearized               );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSExhumeSerpMap,               i, j, TTSexhumeSerpentinized           );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::thicknessCrustMeltOnset,        i, j, thicknessContinetalCrustOnset    );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::slopePreMelt,                   i, j, linearFunction.getM1()           );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::slopePostMelt,                  i, j, linearFunction.getM2()           );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::interceptPostMelt,              i, j, linearFunction.getC2()           );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::RDAadjustedMap,                 i, j, residualDepthAnomaly             );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::TFMap,                          i, j, thinningFactor                   );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::thicknessCrustMap,              i, j, continentalCrustalThickness      );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::thicknessBasaltMap,             i, j, oceanicCrustalThickness          );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::UpperContinentalCrustThickness, i, j, upperContinentalCrustalThickness );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::LowerContinentalCrustThickness, i, j, lowerContinentalCrustalThickness );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::UpperOceanicCrustThickness,     i, j, upperOceanicCrustalThickness     );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::LowerOceanicCrustThickness,     i, j, lowerOceanicCrustalThickness     );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::topBasaltMap,                   i, j, topOceanicCrust                  );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::mohoMap,                        i, j, moho                             );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::ECTMap,                         i, j, effectiveCrustalThickness        );
      }
   }
   restoreData();
}


//------------------------------------------------------------//
void McKenzieCrustCalculator::retrieveData() {
   m_HCuMap.retrieveData ();
   m_HLMuMap.retrieveData();
   m_HBuMap.retrieveData ();
   m_depthBasement->retrieveData();
   if (m_previousThinningFactor != nullptr){
      m_previousThinningFactor->retrieveData();
   }
   if (m_previousContinentalCrustThickness != nullptr){
      m_previousContinentalCrustThickness->retrieveData();
   }
   if (m_previousOceanicCrustThickness != nullptr){
      m_previousOceanicCrustThickness->retrieveData();
   }
}

//------------------------------------------------------------//
void McKenzieCrustCalculator::restoreData() {
   m_HCuMap.restoreData ();
   m_HLMuMap.restoreData();
   m_HBuMap.restoreData ();
   m_depthBasement->restoreData();
   if (m_previousThinningFactor != nullptr){
      m_previousThinningFactor->restoreData();
   }
   if (m_previousContinentalCrustThickness != nullptr){
      m_previousContinentalCrustThickness->restoreData();
   }
   if (m_previousOceanicCrustThickness != nullptr){
      m_previousOceanicCrustThickness->restoreData();
   }
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::getPreviousTF( const unsigned int i, const unsigned int j ) const{
   // if no previous map then the previous TF is equal to 0
   double previousTF = 0.0;
   if (m_previousThinningFactor != nullptr){
      previousTF = m_previousThinningFactor->getValue( i, j );
   }
   return previousTF;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::getPreviousContinentalCrustThickness( const unsigned int i, const unsigned int j ) const{
   // if no previous map then the previous continental crust thickness is equal to the inital crust thickness
   double previousContinentalCrustThickness = m_HCuMap.getValue( i, j );
   if (m_previousContinentalCrustThickness != nullptr){
      previousContinentalCrustThickness = m_previousContinentalCrustThickness->getValue( i, j );
   }
   return previousContinentalCrustThickness;
}

//------------------------------------------------------------//
double McKenzieCrustCalculator::getPreviousOceanicCrustThickness( const unsigned int i, const unsigned int j ) const{
   // if no previous map then the previous oceanic crust thickness is equal 0
   double previousOceanicCrustThickness = 0.0;
   if (m_previousOceanicCrustThickness != nullptr){
      previousOceanicCrustThickness = m_previousOceanicCrustThickness->getValue( i, j );
   }
   return previousOceanicCrustThickness;
}