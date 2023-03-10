//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CompoundLithology.h"

// std library
#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <vector>

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;
#include "ConstantsMathematics.h"
using Utilities::Maths::MilliDarcyToM2;
using Utilities::Maths::MicroWattsToWatts;

#include "FormattingException.h"
#include "Interface.h"
#include "RunParameters.h"

#include "capillarySealStrength.h"
#include "GeoPhysicsProjectHandle.h"
#include "LambdaMixer.h"
#include "NumericFunctions.h"
#include "Quadrature.h"

using namespace DataAccess;
using namespace CBMGenerics;
using namespace capillarySealStrength;

using namespace std;

typedef formattingexception::GeneralException fastCauldronException;


GeoPhysics::CompoundLithology::CompoundLithology(ProjectHandle& projectHandle) : m_projectHandle(projectHandle)
, m_porosity()
{

   m_mixmodeltype              = Interface::HOMOGENEOUS;
   m_lithologyFractureGradient = 1.0;
   m_isFaultLithology          = false;
   m_isBasementLithology       = false;
}
//------------------------------------------------------------//

GeoPhysics::CompoundLithology::~CompoundLithology() {
   m_lithoComponents.clear();
}

std::string GeoPhysics::CompoundLithology::image() const {

   std::stringstream buffer;

   compContainer::const_iterator    componentIter;
   percentContainer::const_iterator percentIter;

   for (componentIter = m_lithoComponents.begin(), percentIter = m_componentPercentage.begin();
      componentIter != m_lithoComponents.end();
      ++componentIter, ++percentIter) {
      buffer << " Simple litho: " << *percentIter << std::endl << (*componentIter)->image() << std::endl << std::endl;
   }

   return buffer.str();
}
//------------------------------------------------------------//


GeoPhysics::CompoundLithologyComposition GeoPhysics::CompoundLithology::getComposition() const {

   double fraction1;
   double fraction2;
   double fraction3;

   std::string simpleName1;
   std::string simpleName2;
   std::string simpleName3;
   std::string mixingModel;
   double     layeringIndex;

   if (m_mixmodeltype == Interface::HOMOGENEOUS) {
      mixingModel = "Homogeneous";
      layeringIndex = -9999;
   }
   else if (m_mixmodeltype == Interface::LAYERED) {
      mixingModel = "Layered";
      layeringIndex = m_layeringIndex;
   }
   else {
      mixingModel = "Undefined";
   }

   switch (m_lithoComponents.size()){
   case 1:
      fraction1   = m_componentPercentage[0];
      fraction2   = 0.0;
      fraction3   = 0.0;
      simpleName1 = m_lithoComponents[0]->getName();
      simpleName2 = "";
      simpleName3 = "";
      break;

   case 2:
      fraction1   = m_componentPercentage[0];
      fraction2   = m_componentPercentage[1];
      fraction3   = 0.0;
      simpleName1 = m_lithoComponents[0]->getName();
      simpleName2 = m_lithoComponents[1]->getName();
      simpleName3 = "";
      break;

   case 3:
      fraction1   = m_componentPercentage[0];
      fraction2   = m_componentPercentage[1];
      fraction3   = m_componentPercentage[2];
      simpleName1 = m_lithoComponents[0]->getName();
      simpleName2 = m_lithoComponents[1]->getName();
      simpleName3 = m_lithoComponents[2]->getName();
      break;

   default:
      fraction1   = 0.0;
      fraction2   = 0.0;
      fraction3   = 0.0;
      simpleName1 = "";
      simpleName2 = "";
      simpleName3 = "";
      break;
   }
   return CompoundLithologyComposition(simpleName1, simpleName2, simpleName3, fraction1, fraction2, fraction3, mixingModel, layeringIndex);
}
//------------------------------------------------------------//


bool GeoPhysics::CompoundLithology::isBasement() const {
   return m_isBasementLithology;
}
//------------------------------------------------------------//


GeoPhysics::SimpleLithology* GeoPhysics::CompoundLithology::getSimpleLithology() const {
   return m_lithoComponents[0];
}
//------------------------------------------------------------//


const string GeoPhysics::CompoundLithology::getThermalModel() const {

   string thermalModel = "";
   if (isBasement()) {
      thermalModel = m_lithoComponents[0]->getThermalCondModelName();
   }
   return thermalModel;
}
//------------------------------------------------------------//


void GeoPhysics::CompoundLithology::addLithology(SimpleLithology* a_lithology, const double a_percentage) {
   m_lithoComponents.push_back(a_lithology);
   m_componentPercentage.push_back(a_percentage);
}
//------------------------------------------------------------//


void GeoPhysics::CompoundLithology::setMinimumPorosity(DataAccess::Interface::PorosityModel porosityModel, double  surfaceVoidRatio, double soilMechanicsCompactionCoefficient) {

   if (porosityModel == DataAccess::Interface::SOIL_MECHANICS_POROSITY) {
      setMinimumSoilMechanicsPorosity(surfaceVoidRatio, soilMechanicsCompactionCoefficient);
   }
   else {
      setMinimumExponentialPorosity();

   }

}
//------------------------------------------------------------//


void GeoPhysics::CompoundLithology::setMinimumExponentialPorosity() {

   compContainer::iterator componentIter;
   int count;

   for (componentIter = m_lithoComponents.begin(), count = 0; componentIter != m_lithoComponents.end(); ++componentIter, ++count) {
      minimumCompoundPorosity(count) = 0.0;
   }

}
//------------------------------------------------------------//


void GeoPhysics::CompoundLithology::setMinimumSoilMechanicsPorosity(double  surfaceVoidRatio, double soilMechanicsCompactionCoefficient) {

   double maximumMixedVES;
   double minimumVoidRatio = MinimumSoilMechanicsPorosity / (1.0 - MinimumSoilMechanicsPorosity);
   compContainer::iterator componentIter;
   int count;

   if (soilMechanicsCompactionCoefficient > 0.0) {
      maximumMixedVES = referenceEffectiveStress() * exp((surfaceVoidRatio - minimumVoidRatio) / soilMechanicsCompactionCoefficient);

      for (componentIter = m_lithoComponents.begin(), count = 0; componentIter != m_lithoComponents.end(); ++componentIter, ++count) {
         minimumCompoundPorosity(count) = (*componentIter)->porosity(maximumMixedVES, maximumMixedVES, false, 0.0);
      }

   }
   else {

      for (componentIter = m_lithoComponents.begin(), count = 0; componentIter != m_lithoComponents.end(); ++componentIter, ++count) {
         minimumCompoundPorosity(count) = (*componentIter)->getDepoPoro();
      }

   }

}
//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::createThCondTbl() {

   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   int Table_Size;
   int Array_Size = 99999;

   double anisotropy;
   anisotropy = (*componentIter)->getThCondAn();

   // Only initialized to prevent compiler warning, the warning is erroneous.
   double ThermalCondN = 0.0;

   // Only initialized to prevent compiler warning, the warning is erroneous.
   double ThermalCondP = 0.0;

   vector<ibs::XF> mixThermCondNTbl, mixThermCondPTbl;
   vector<ibs::XF>& thermCondTbl1 = (*componentIter)->getThermCondTbl();
   vector<ibs::XF>::iterator XYIter = thermCondTbl1.begin();

   Table_Size = thermCondTbl1.size();
   Array_Size = min(Array_Size, Table_Size);

   while (thermCondTbl1.end() != XYIter) {
      //Legacy behavior
      if (m_isLegacy)
      {
         if (m_mixmodeltype == Interface::HOMOGENEOUS)
         {
            ThermalCondN = 1.0;
            ThermalCondP = 1.0;
            ThermalCondN *= pow( (*XYIter).getF(), (*percentIter)*0.01 );
            ThermalCondP *= pow( (*XYIter).getF()*anisotropy, (*percentIter)*0.01 );
         }
         else if (m_mixmodeltype == Interface::LAYERED)
         {
            ThermalCondN = 0.0;
            ThermalCondP = 0.0;
            ThermalCondN += (*percentIter)*0.01 / (*XYIter).getF();
            ThermalCondP += (*percentIter)*0.01 * (*XYIter).getF() * anisotropy;
         }
      }
      //New rock property library behavior
      else
      {
         //Note:  mixing type is not considered (homogeneous, layered)
         //geometric mean
         ThermalCondN = 1.0;
         ThermalCondP = 1.0;
         ThermalCondN *= pow( (*XYIter).getF(), (*percentIter)*0.01 );
         ThermalCondP *= pow( (*XYIter).getF()*anisotropy, (*percentIter)*0.01 );
      }

      ibs::XF xyPoint((*XYIter).getX(), ThermalCondN);
      mixThermCondNTbl.push_back(xyPoint);
      xyPoint.set((*XYIter).getX(), ThermalCondP);
      mixThermCondPTbl.push_back(xyPoint);
      ++XYIter;
   }

   ++componentIter;
   ++percentIter;

   while (m_lithoComponents.end() != componentIter) {
      anisotropy = (*componentIter)->getThCondAn();

      vector<ibs::XF>& thermCondTbl2 = (*componentIter)->getThermCondTbl();
      Table_Size = thermCondTbl2.size();
      Array_Size = min(Array_Size, Table_Size);
      XYIter = thermCondTbl2.begin();
      vector<ibs::XF>::iterator mixXYnIter = mixThermCondNTbl.begin();
      vector<ibs::XF>::iterator mixXYpIter = mixThermCondPTbl.begin();

      while ((thermCondTbl2.end() != XYIter) && (mixThermCondNTbl.end() != mixXYnIter)) {
         ThermalCondN = (*mixXYnIter).getF();
         ThermalCondP = (*mixXYpIter).getF();

         //Legacy behavior
         if (m_isLegacy)
         {
            if (m_mixmodeltype == Interface::HOMOGENEOUS)
            {
               ThermalCondN *= pow( (*XYIter).getF(), (*percentIter)*0.01 );
               ThermalCondP *= pow( (*XYIter).getF()*anisotropy, (*percentIter)*0.01 );
            }
            else if (m_mixmodeltype == Interface::LAYERED)
            {
               ThermalCondN += (*percentIter)*0.01 / (*XYIter).getF();
               ThermalCondP += (*percentIter)*0.01 * (*XYIter).getF() * anisotropy;
            }
         }
         //New rock property library behavior
         else
         {
            ThermalCondN *= pow( (*XYIter).getF(), (*percentIter)*0.01 );
            ThermalCondP *= pow( (*XYIter).getF()*anisotropy, (*percentIter)*0.01 );
         }
         (*mixXYnIter).setF(ThermalCondN);
         (*mixXYpIter).setF(ThermalCondP);
         ++XYIter;
         ++mixXYnIter;
         ++mixXYpIter;
      }
      ++componentIter;
      ++percentIter;
   }

   // write into the interpolator
   int Counter = 0;
   vector<ibs::XF>::iterator mixXYnIter = mixThermCondNTbl.begin();
   for (mixXYnIter = mixThermCondNTbl.begin(); mixXYnIter != mixThermCondNTbl.end();
      mixXYnIter++) {
      if (++Counter > Array_Size) continue;
      //Legacy behavior
      if (m_isLegacy)
      {
         if (m_mixmodeltype == Interface::LAYERED && (*mixXYnIter).getF() != 0.0)
         {
            addThermCondPointN( (*mixXYnIter).getX(), 1 / (*mixXYnIter).getF() );
         }
         else
         {
            addThermCondPointN( (*mixXYnIter).getX(), (*mixXYnIter).getF() );
         }
      }
      //New rock property library behavior
      else
      {
         addThermCondPointN( (*mixXYnIter).getX(), (*mixXYnIter).getF() );
      }
   }

   Counter = 0;
   vector<ibs::XF>::iterator mixXYpIter = mixThermCondPTbl.begin();
   for (mixXYpIter = mixThermCondPTbl.begin(); mixXYpIter != mixThermCondPTbl.end();
      mixXYpIter++) {
      if (++Counter > Array_Size) continue;
      addThermCondPointP((*mixXYpIter).getX(), (*mixXYpIter).getF());
   }

   while (mixThermCondNTbl.size() != 0) {
      vector<ibs::XF>::iterator mixXYIter = mixThermCondNTbl.begin();
      mixThermCondNTbl.erase(mixXYIter);
   }
   while (mixThermCondPTbl.size() != 0) {
      vector<ibs::XF>::iterator mixXYIter = mixThermCondPTbl.begin();
      mixThermCondPTbl.erase(mixXYIter);
   }

}
//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::exponentialDecompactionFunction(const double ves) const {

   if (m_porosity.getCompactionCoefficient() <= 0.0) return 0.0;
   double  r1 = 1.0 - m_porosity.getSurfacePorosity() * exp(-ves * m_porosity.getCompactionCoefficient());
   double  r2 = log(r1);
   double  r3 = ves + r2 / m_porosity.getCompactionCoefficient();

   return r3;

}
//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::newExponentialDecompactionFunctionTop(const double topVes) const
{
    if (m_porosity.getCompactionCoefficient() <= 0.0) return 0.0;

    return topVes/(1.0 - m_porosity.getMinimumMechanicalPorosity()) +
            1.0 / ( 
                    m_porosity.getCompactionCoefficient() * (1.0 - m_porosity.getMinimumMechanicalPorosity())
                  ) *
            std::log( 
                          (1.0 - m_porosity.getMinimumMechanicalPorosity()) 
                      -   (m_porosity.getTopPorosity(topVes) - m_porosity.getMinimumMechanicalPorosity())
                    );
}
//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::newExponentialDecompactionFunctionBottom(const double BottomVes,const double topVes) const
{
    if (m_porosity.getCompactionCoefficient() <= 0.0) return 0.0;

	return topVes / (1.0 - m_porosity.getMinimumMechanicalPorosity()) +
            1.0 / (
                    m_porosity.getCompactionCoefficient() * (1 - m_porosity.getMinimumMechanicalPorosity())
                  ) *
		        std::log(
                            (1.0 - m_porosity.getMinimumMechanicalPorosity()) * std::exp(m_porosity.getCompactionCoefficient()*(BottomVes - topVes))
                        -   (m_porosity.getTopPorosity(topVes) - m_porosity.getMinimumMechanicalPorosity())
                        );
}
//------------------------------------------------------------//


void GeoPhysics::CompoundLithology::makeFault(const bool newFaultValue) {
   m_isFaultLithology = newFaultValue;
}
//------------------------------------------------------------//


double GeoPhysics::CompoundLithology::heatcapacity(const double temperature) const {

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   percentContainer::const_iterator percentIter = m_componentPercentage.begin();

   double lithoHeatCapacity = 0;

   while (m_lithoComponents.end() != componentIter) {
      lithoHeatCapacity += (*componentIter)->heatcapacity(temperature) * (*percentIter) / 100;
      ++componentIter;
      ++percentIter;
   }

   return lithoHeatCapacity;
}
//------------------------------------------------------------//


double GeoPhysics::CompoundLithology::densityXheatcapacity(const double temperature, const double  pressure) const {

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   percentContainer::const_iterator percentIter = m_componentPercentage.begin();

   double lithoHeatCapacity = 0.0;

   while (m_lithoComponents.end() != componentIter) {

#ifdef NOPRESSURE
      lithoHeatCapacity += (*componentIter)->heatcapacity(temperature) * (*componentIter)->getDensity(temperature, 0.0)  * (*percentIter) / 100;
#else
      // temporary remove pressure term from the density calculation
      lithoHeatCapacity += (*componentIter)->heatcapacity(temperature) * (*componentIter)->getDensity(temperature, pressure)  * (*percentIter) / 100;
#endif
      ++componentIter;
      ++percentIter;
   }

   return lithoHeatCapacity;
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::densityXheatcapacity ( const unsigned int       size,
                                                           ArrayDefs::ConstReal_ptr temperature,
                                                           ArrayDefs::ConstReal_ptr pressure,
                                                           ArrayDefs::Real_ptr      densityXHeatCap ) const {

   const SimpleLithology* currentLitho = m_lithoComponents [ 0 ];
   double fraction = m_componentPercentage [ 0 ] / 100.0;

   // There is always at least one lithology, so do this one first.
   for ( unsigned int j = 0; j < size; ++j ) {
      densityXHeatCap [ j ] = currentLitho->heatcapacity(temperature [ j ]) * currentLitho->getDensity(temperature [ j ], pressure [ j ])  * fraction;
   }

   // Now mix the density-times-heat-capacity of the remaining lithology.
   for ( size_t l = 1; l < m_lithoComponents.size (); ++l ) {
      fraction = m_componentPercentage [ l ] / 100.0;
      currentLitho = m_lithoComponents [ l ];

      for ( unsigned int j = 0; j < size; ++j ) {
         densityXHeatCap [ j ] += currentLitho->heatcapacity(temperature [ j ]) * currentLitho->getDensity(temperature [ j ], pressure [ j ])  * fraction;
      }

   }

}

//------------------------------------------------------------//


double GeoPhysics::CompoundLithology::hydrostatFullCompThickness(const double maxVes,
   const double thickness,
   const double densitydiff,
   const bool   overpressuredCompaction) const {

   /*This module is based on Lithology::CalcFullCompactedThickness method of Cauldron */

   /* if the compaction coefficient is nil then the full compacted thickness = thickness */

   ///
   /// If running an overpressure calculation then make an estimate how much
   /// overpressure there might be. More overpressure implies less ves. The scaling
   /// factor is based on the amount of ves there would be if the basin was hydrostatic
   /// pressured.
   ///
   const double vesScaleFactor = 0.5;

#if 0
   const double vesScaleFactor = m_projectHandle.getCommandLineParameters()->getVesScaling();
#endif
   double MaxVesValue;

   if (overpressuredCompaction) {
      MaxVesValue = maxVes * vesScaleFactor;
   }
   else {
      MaxVesValue = maxVes;
   }

   return  m_porosity.getFullCompThickness(MaxVesValue, thickness, densitydiff, vesScaleFactor, overpressuredCompaction);

}
//------------------------------------------------------------//


bool GeoPhysics::CompoundLithology::isIncompressible() const {

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   bool lithoIsIncompressible = true;

   while (m_lithoComponents.end() != componentIter) {
      lithoIsIncompressible &= (*componentIter)->isIncompressible();
      ++componentIter;
   }

   return lithoIsIncompressible;
}
//------------------------------------------------------------//


double GeoPhysics::CompoundLithology::porosity(const double sigma,
   const double sigma_max,
   const bool   includeChemicalCompaction,
   const double chemicalCompactionTerm) const {

   return m_porosity.calculate(sigma, sigma_max, includeChemicalCompaction, chemicalCompactionTerm);
}
//------------------------------------------------------------//


bool GeoPhysics::CompoundLithology::allowableMixing() const {

   bool Mixing_Okay = true;

   compContainer::const_iterator    Simple_Lithologies = m_lithoComponents.begin();
   percentContainer::const_iterator Percentages = m_componentPercentage.begin();

   if (m_lithoComponents.size() > 1) {
      // Only initialized to prevent compiler warning, the warning is erroneous. Code can be re-arranged to prevent the warning.
      DataAccess::Interface::PorosityModel First_Porosity_Model = DataAccess::Interface::EXPONENTIAL_POROSITY;
      bool              First_Active_Lithology = true;

      while (m_lithoComponents.end() != Simple_Lithologies) {

         if (*Percentages > 0.0) {

            if (First_Active_Lithology) {
               First_Porosity_Model = (*Simple_Lithologies)->getPorosityModel();
               First_Active_Lithology = false;
            }
            else if (First_Porosity_Model != (*Simple_Lithologies)->getPorosityModel()) {
               Mixing_Okay = false;
            }

         }

         ++Simple_Lithologies;
         ++Percentages;
      }

   }

   if (!Mixing_Okay) {
      Simple_Lithologies = m_lithoComponents.begin();
      Percentages = m_componentPercentage.begin();

      std::cout << "Basin_Error: ****************  ERROR  Incorrect mixing  ****************" << std::endl;
      std::cout << "   Lithotype           Porosity model   percentage" << std::endl;

      while (m_lithoComponents.end() != Simple_Lithologies) {
         std::cout << std::setw(20) << setiosflags(ios::left) << (*Simple_Lithologies)->getName() << resetiosflags(ios::left)
            << std::setw(10) << (*Percentages) << std::endl;

         ++Simple_Lithologies;
         ++Percentages;
      }



   }


   return Mixing_Okay;
}
//------------------------------------------------------------//


bool  GeoPhysics::CompoundLithology::reCalcProperties(){

   if (!allowableMixing()) {
      return false;
   }

   setIsLegacy(m_projectHandle.getRunParameters()->getLegacy());

   m_density                       = 0.0;
   m_seismicVelocitySolid          = 0.0;
   m_nExponentVelocity             = 0.0;
   m_depositionalPermeability      = 0.0;
   m_heatProduction                = 0.0;
   m_permeabilityincr              = 0.0;
   m_permeabilitydecr              = 0.0;
   m_thermalConductivityAnisotropy = 1.0;
   m_thermalConductivityValue      = 0.0;

   // loop through all the simple lithology and calculate the
   // properties for this lithology
   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   m_referenceSolidViscosity        = 0.0;
   m_lithologyActivationEnergy      = 0.0;
   m_lithologyFractureGradient      = 0.0;
   double minimumMechanicalPorosity = 0.0;
   m_quartzGrainSize                = 0.0;
   m_quartzFraction                 = 0.0;
   m_coatingClayFactor              = 0.0;
   m_igneousIntrusionTemperature    = Interface::RecordValueUndefined;

   //If atleast one simple lithology exists in the lithoComponent list
   if (m_lithoComponents[0] != nullptr ) {
       //1. Sill intrusion - temperature of intrusion
       // defined only if an igneous intrusion lithology is used and if there is only one component
       if ((double)(*percentIter) == 100)
       {
           m_igneousIntrusionTemperature = (*componentIter)->getIgneousIntrusionTemperature();
       }

       while (m_lithoComponents.end() != componentIter) {
           double pcMult = (double)(*percentIter) / 100;

           //2. Matrix Property calculated using the arithmetic mean
           m_density += (*componentIter)->getDensity() * pcMult;
           m_seismicVelocitySolid += (*componentIter)->getSeismicVelocity() * pcMult;
           m_nExponentVelocity += (*componentIter)->getSeismicVelocityExponent() * pcMult;
           m_depositionalPermeability += (*componentIter)->getDepoPerm() * pcMult;
           m_thermalConductivityValue += (*componentIter)->getThCondVal() * pcMult;
           m_heatProduction += (*componentIter)->getHeatProduction() * pcMult;
           m_referenceSolidViscosity += (*componentIter)->getReferenceSolidViscosity() * pcMult;
           m_lithologyActivationEnergy += (*componentIter)->getLithologyActivationEnergy() * pcMult;
           minimumMechanicalPorosity += (*componentIter)->getMinimumMechanicalPorosity() / 100.0 * pcMult;
           m_lithologyFractureGradient += (*componentIter)->getLithologyFractureGradient() * pcMult;
           m_quartzFraction += (*componentIter)->getQuartzFraction() * pcMult;
           m_coatingClayFactor += (*componentIter)->getClayCoatingFactor() * pcMult;

           //3. Matrix Property calculated using the geometric mean
           m_thermalConductivityAnisotropy *= pow((*componentIter)->getThCondAn(), pcMult);

           //4. Matrix Property calculated using the algebraic mean
           m_quartzGrainSize += pcMult * pow((*componentIter)->getQuartzGrainSize(), 3.0);

           ++componentIter;
           ++percentIter;
       }
   }

   m_quartzGrainSize = pow(m_quartzGrainSize, 1.0 / 3.0);

   //5. Porosity
   // temporary values before mixing
   DataAccess::Interface::PorosityModel porosityModel = DataAccess::Interface::PorosityModel::EXPONENTIAL_POROSITY;
   double surfacePorosity=.51;
   double surfaceVoidRatio = 0;
   double compactionincr = 5.6e-8;
   double compactionincrA = 0;
   double compactionincrB = 0;
   double compactiondecr = 5.6e-9;
   double compactiondecrA = 0;
   double compactiondecrB = 0;
   double compactionRatio = 0.5;
   double soilMechanicsCompactionCoefficient = 0.0;

   if ( m_lithoComponents[0] != nullptr ) {
       mixPorosityModel(porosityModel);
       mixSurfacePorosity(porosityModel, surfacePorosity, surfaceVoidRatio);

       mixCompactionCoefficients(compactionincr, compactionincrA, compactionincrB, compactiondecr, compactiondecrA, compactiondecrB, compactionRatio, soilMechanicsCompactionCoefficient);
   }
   //Create porosity object
   m_porosity = Porosity::create(porosityModel,
      surfacePorosity,
      minimumMechanicalPorosity,
      compactionincr,
      compactionincrA,
      compactionincrB,
      compactiondecr,
      compactiondecrA,
      compactiondecrB,
      compactionRatio,
      soilMechanicsCompactionCoefficient,
      m_projectHandle.getRunParameters()->getLegacy());

   if ( m_lithoComponents[0] != nullptr ) {
       mixCapillaryEntryPressureCofficients();
       mixBrooksCoreyParameters();

       createThCondTbl();

       setMinimumPorosity(porosityModel, surfaceVoidRatio, soilMechanicsCompactionCoefficient);
   }

   //6. Permeability
   if (surfacePorosity < 0.0299) {
      // Really less than 0.03 but some rounding may occur if user set a lithology with 0.03 surface porosity
      m_fracturedPermeabilityScalingValue = 100.0;
   }
   else {
      m_fracturedPermeabilityScalingValue = 10.0; // What to set this to?  10, 50 or whatever.
   }

   //7. Seismic velocity
   DataAccess::Interface::SeismicVelocityModel seismicVelocityModel = m_projectHandle.getRunParameters()->getSeismicVelocityAlgorithm();
   if ( m_lithoComponents[0] != nullptr ) {
       double mixedModulusSolid = this->mixModulusSolid();
       m_seismicVelocity = SeismicVelocity::create(seismicVelocityModel,
           m_seismicVelocitySolid,
           mixedModulusSolid,
           m_density,
           surfacePorosity,
           m_nExponentVelocity);


       const string lithoname = m_lithoComponents[0]->getName();
       m_isBasementLithology = false;

       if ((m_lithoComponents.size() == 1) && (lithoname == m_projectHandle.getCrustLithoName() || lithoname == m_projectHandle.getMantleLithoName() || lithoname == DataAccess::Interface::ALCBasalt)) {
           m_isBasementLithology = true;
       }

       std::vector<double> componentAnisotropy;

       if (m_lithoComponents.size() > 0) {
           componentAnisotropy.push_back(m_lithoComponents[0]->getPermAniso());
       }

       if (m_lithoComponents.size() > 1) {
           componentAnisotropy.push_back(m_lithoComponents[1]->getPermAniso());
       }

       if (m_lithoComponents.size() > 2) {
           componentAnisotropy.push_back(m_lithoComponents[2]->getPermAniso());
       }

       m_permeabilityMixer.reset(m_componentPercentage, componentAnisotropy, m_isLegacy, m_layeringIndex, m_mixmodeltype, m_isFaultLithology);
   }
   return true;
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::mixSurfacePorosity(DataAccess::Interface::PorosityModel porosityModel, double & surfacePorosity, double & surfaceVoidRatio) {

   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   if (porosityModel == DataAccess::Interface::SOIL_MECHANICS_POROSITY) {
      double Simple_Surface_Porosity;
      double Simple_Void_Ratio;
      double Mixed_Void_Ratio = 0.0;

      while (m_lithoComponents.end() != componentIter) {

         Simple_Surface_Porosity = (*componentIter)->getDepoPoro();
         Simple_Void_Ratio = Simple_Surface_Porosity / (1.0 - Simple_Surface_Porosity);
         Mixed_Void_Ratio = Mixed_Void_Ratio + Simple_Void_Ratio * (*percentIter) * 0.01;

         ++componentIter;
         ++percentIter;
      }

      surfacePorosity = Mixed_Void_Ratio / (1.0 + Mixed_Void_Ratio);
   }
   else {
      surfacePorosity = 0.0;

      /* Dominant litho-type defines Porosity Model and Compaction Coefficient value */
      while (m_lithoComponents.end() != componentIter) {

         surfacePorosity = surfacePorosity + (*componentIter)->getDepoPoro() * (*percentIter) * 0.01;

         ++componentIter;
         ++percentIter;
      }
   }

   surfaceVoidRatio = surfacePorosity / (1 - surfacePorosity);

}


//------------------------------------------------------------//


void GeoPhysics::CompoundLithology::mixCompactionCoefficients(
   double& compactionincr,
   double& compactionincrA,
   double& compactionincrB,
   double& compactiondecr,
   double& compactiondecrA,
   double& compactiondecrB,
   double& compactionRatio,
   double& soilMechanicsCompactionCoefficient) {
   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   soilMechanicsCompactionCoefficient = 0.0;
   compactionincr                     = 0.0;
   compactionincrA                    = 0.0;
   compactionincrB                    = 0.0;
   compactiondecr                     = 0.0;
   compactiondecrA                    = 0.0;
   compactiondecrB                    = 0.0;
   compactionRatio                    = 0.0;

   double Fraction;

   while (m_lithoComponents.end() != componentIter) {

      Fraction = 0.01 * (*percentIter);

      soilMechanicsCompactionCoefficient = soilMechanicsCompactionCoefficient + (*componentIter)->getCompCoeff() * Fraction;
      compactionincr  = compactionincr + (*componentIter)->getCompCoeff() * Fraction;
      compactionincrA = compactionincrA + (*componentIter)->getCompCoeffA() * Fraction;
      compactionincrB = compactionincrB + (*componentIter)->getCompCoeffB() * Fraction;

      compactiondecr  = compactiondecr + 0.1 * (*componentIter)->getCompCoeff() * Fraction;
      compactiondecrA = compactiondecrA + 0.1 * (*componentIter)->getCompCoeffA() * Fraction;
      compactiondecrB = compactiondecrB + 0.1 * (*componentIter)->getCompCoeffB() * Fraction;

      compactionRatio = compactionRatio + (*componentIter)->getCompRatio() * Fraction;


      ++componentIter;
      ++percentIter;
   }
}

//------------------------------------------------------------//

void   GeoPhysics::CompoundLithology::mixPorosityModel(DataAccess::Interface::PorosityModel & porosityModel) {

   double Largest_Volume_Fraction = -100.0;

   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   /* Dominant litho-type defines Porosity Model and Compaction Coefficient value */
   while (m_lithoComponents.end() != componentIter) {

      if ((double)(*percentIter) > Largest_Volume_Fraction) {

         Largest_Volume_Fraction = (double)(*percentIter);

         porosityModel = (*componentIter)->getPorosityModel();

      }

      ++componentIter;
      ++percentIter;

   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::getSurfacePorosity(CompoundProperty& porosity) const {

   int lithologyIndex = 0;
   compContainer::const_iterator componentIter = m_lithoComponents.begin();

   while (m_lithoComponents.end() != componentIter) {
      porosity(lithologyIndex) = (*componentIter)->getDepoPoro();
      ++componentIter;
      ++lithologyIndex;
   }

   porosity.setMixedProperty(m_porosity.getSurfacePorosity());
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::setMixModel(const std::string& mixmodel, double layeringIndex) {

   if (mixmodel == "Homogeneous") {
      m_mixmodeltype = Interface::HOMOGENEOUS;

      m_layeringIndex = Interface::DefaultUndefinedMapValue;
      m_mixHorizonExp = Interface::DefaultUndefinedMapValue;
      m_inverseMixHorizonExp = Interface::DefaultUndefinedMapValue;
      m_mixVerticalExp = Interface::DefaultUndefinedMapValue;
      m_inverseMixVerticalExp = Interface::DefaultUndefinedMapValue;

   } else if (mixmodel == "Layered") {
      m_mixmodeltype = Interface::LAYERED;

      m_layeringIndex = layeringIndex;
      assert(("Layering Index must be between 0 and 1 included for the Layered mixing", layeringIndex >= 0.0 && layeringIndex <= 1.0));
      //Equations from [Jennings, 2014]
      m_mixHorizonExp = ( 1.0 + 2.0 * layeringIndex ) / 3.0;
      m_inverseMixHorizonExp = 1.0 / m_mixHorizonExp;

      m_mixVerticalExp = ( 1.0 - 4.0 * layeringIndex ) / 3.0;
      m_inverseMixVerticalExp = 1.0 / m_mixVerticalExp;

   } else {
      m_mixmodeltype = Interface::UNDEFINED;
      throw fastCauldronException() << "MixModel not defined";
   }
}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::computeSegmentThickness(const double topMaxVes,
   const double botMaxVes,
   const double densityDifference,
   const double solidThickness) const {

   if (m_porosity.getPorosityModel() == DataAccess::Interface::EXPONENTIAL_POROSITY && densityDifference > 0.0 ) 
   {
       auto isLegacy = m_projectHandle.getRunParameters()->getLegacy();

       /// The "branchless" programming paradigm, no if's and but's
       double G_top = exponentialDecompactionFunction(topMaxVes) * isLegacy 
           + newExponentialDecompactionFunctionTop(topMaxVes) * (!isLegacy);// \ refer to the new Literature
           
       double G_btm = exponentialDecompactionFunction(botMaxVes) * isLegacy 
           + newExponentialDecompactionFunctionBottom(botMaxVes, topMaxVes) * (!isLegacy);// \ refer to the new Literature
       

       return (G_btm - G_top) / (AccelerationDueToGravity * densityDifference);
   }
   else {
       return computeSegmentThickness(topMaxVes, botMaxVes, topMaxVes, botMaxVes, densityDifference, solidThickness);
   }

}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::computeSegmentThickness(const double topMaxVes,
   const double bottomMaxVes,
   const double topVes,
   const double bottomVes,
   const double ,
   const double solidThickness) const {

   const bool   IncludeChemicalCompaction = false;
   const double ChemicalCompactionDummyValue = 0.0;

   double porosityTop = porosity(topVes, topMaxVes, IncludeChemicalCompaction, ChemicalCompactionDummyValue);
   double porosityBottom = porosity(bottomVes, bottomMaxVes, IncludeChemicalCompaction, ChemicalCompactionDummyValue);

   return 0.5 * (1.0 / (1.0 - porosityTop) + 1.0 / (1.0 - porosityBottom)) * solidThickness;

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkDensXHeatCapacity(const FluidType* fluid,
                                                              const double     Porosity,
                                                              const double     Pressure,
                                                              const double     Temperature,
                                                              const double     LithoPressure,
                                                              double&    BulkDensXHeatCapacity) const {

   bool LithoHasFluid = (fluid != 0);

   double MatrixDensXHeatCap = densityXheatcapacity(Temperature, LithoPressure);

   if (LithoHasFluid) {

      double FluidDensXHeatCap = fluid->densXheatCapacity(Temperature, Pressure);
      BulkDensXHeatCapacity = MatrixDensXHeatCap * (1.0 - Porosity) + FluidDensXHeatCap * Porosity;

   }
   else {
      // The only layers that have no fluid are the crust and mantle. Since they don't have
      // a porosity, the volumetric heat capacity is the same as that of the lithology.
      BulkDensXHeatCapacity = MatrixDensXHeatCap;

   }
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkDensXHeatCapacity ( const unsigned int  size,
                                                                const double*       fluidDensityXHeatCap,
                                                                const double*       porosity,
                                                                const double*       pressure,
                                                                const double*       temperature,
                                                                const double*       lithostaticPressure,
                                                                ArrayDefs::Real_ptr bulkDensXHeatCapacity,
                                                                ArrayDefs::Real_ptr matrixDensXHeatCapWorkSpace ) const {

   densityXheatcapacity ( size, temperature, lithostaticPressure, matrixDensXHeatCapWorkSpace );

   if ( fluidDensityXHeatCap != nullptr ) {

      for ( unsigned int i = 0; i < size; ++i ) {
         bulkDensXHeatCapacity [ i ] = matrixDensXHeatCapWorkSpace [ i ] * (1.0 - porosity [ i ]) + fluidDensityXHeatCap [ i ] * porosity [ i ];
      }

   } else {

      for ( unsigned int i = 0; i < size; ++i ) {
         bulkDensXHeatCapacity [ i ] = matrixDensXHeatCapWorkSpace [ i ];
      }

   }

}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::computeDensity(const double temperature,
                                                     const double lithoPressure) const {
   return (not isBasement() ? m_density : m_lithoComponents[0]->getDensity(temperature, lithoPressure));
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkDensity(const FluidType* fluid,
                                                    const double     porosity,
                                                    double&    bulkDensity) const {

   if (fluid != 0) {
      double FluidDensity = fluid->getConstantDensity();
      bulkDensity = m_density * (1.0 - porosity) + FluidDensity * porosity;
   }
   else {
      bulkDensity = m_density;
   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkDensity(const FluidType* fluid,
                                                    const double     porosity,
                                                    const double     porePressure,
                                                    const double     temperature,
                                                    double&    bulkDensity) const {

   if (fluid != 0) {
      double FluidDensity = fluid->density(temperature, porePressure);
      bulkDensity = (m_density * (1.0 - porosity)) + (FluidDensity * porosity);
   }
   else {
      bulkDensity = m_density;
   }

}

//------------------------------------------------------------//

// use this function in calculation of BulkDensity output property
void GeoPhysics::CompoundLithology::calcBulkDensity(const FluidType* fluid,
                                                    const double     porosity,
                                                    const double     porePressure,
                                                    const double     temperature,
                                                    const double     lithoPressure,
                                                    double&    bulkDensity) const {

   double MatrixDensity = (!isBasement() ? m_density : m_lithoComponents[0]->getDensity(temperature, lithoPressure));

   if (fluid != 0) {
      double FluidDensity = fluid->density(temperature, porePressure);
      bulkDensity = (MatrixDensity * (1.0 - porosity)) + (FluidDensity * porosity);
   }
   else {
      bulkDensity = MatrixDensity;
   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkThermCondNP(const FluidType* fluid,
                                                        const double     Porosity,
                                                        const double     Temperature,
                                                        const double     PorePressure,
                                                        double&    BulkTHCondN,
                                                        double&    BulkTHCondP) const {

   double MatrixTHCondN = thermalconductivityN(Temperature);
   double MatrixTHCondP = thermalconductivityP(Temperature);

   double FluidThCond = 1.0;

   if (fluid != 0) {
      FluidThCond = fluid->thermalConductivity(Temperature, PorePressure);
   }

   BulkTHCondN = pow(MatrixTHCondN, 1.0 - Porosity) * pow(FluidThCond, Porosity);
   BulkTHCondP = pow(MatrixTHCondP, 1.0 - Porosity) * pow(FluidThCond, Porosity);

}
//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkThermCondNP ( const unsigned int size,
                                                          const double* fluidThermalConductivity,
                                                          const double* porosity,
                                                          const double* temperature,
                                                          const double* porePressue,
                                                          double*       bulkThermalCondN,
                                                          double*       bulkThermalCondP ) const {

   double matrixThermalConductivityN;
   double matrixThermalConductivityP;

   if ( fluidThermalConductivity != nullptr ) {

      for ( unsigned int i = 0; i < size; ++i ) {
          matrixThermalConductivityN = thermalconductivityN ( temperature [ i ]);
          matrixThermalConductivityP = thermalconductivityP ( temperature [ i ]);

          bulkThermalCondN [ i ] = matrixThermalConductivityN * pow ( fluidThermalConductivity [ i ] / matrixThermalConductivityN, porosity [ i ]);
          bulkThermalCondP [ i ] = matrixThermalConductivityP * pow ( fluidThermalConductivity [ i ] / matrixThermalConductivityP, porosity [ i ]);
      }

   } else {

      for ( unsigned int i = 0; i < size; ++i ) {
          matrixThermalConductivityN = thermalconductivityN ( temperature [ i ]);
          matrixThermalConductivityP = thermalconductivityP ( temperature [ i ]);

          bulkThermalCondN [ i ] = pow ( matrixThermalConductivityN, 1.0 - porosity [ i ]);
          bulkThermalCondP [ i ] = pow ( matrixThermalConductivityP, 1.0 - porosity [ i ]);
      }

   }
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkHeatProd(const double Porosity, double &BulkHeatProd)  const {

   BulkHeatProd = (1.0 - Porosity) * m_heatProduction * MicroWattsToWatts;

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::getPorosity(const double            ves,
                                                const double            maxVes,
                                                const bool              includeChemicalCompaction,
                                                const double            chemicalCompactionTerm,
                                                CompoundProperty& Porosity) const {

   int lithologyIndex = 0;
   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   percentContainer::const_iterator percentIter = m_componentPercentage.begin();

   double componentPorosity;

   while (m_lithoComponents.end() != componentIter) {

      componentPorosity = (*componentIter)->porosity(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
      Porosity(lithologyIndex) = componentPorosity;

      ++componentIter;
      ++percentIter;
      ++lithologyIndex;
   }

   Porosity.setMixedProperty(this->porosity(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm));
}


//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::getPorosity ( const unsigned int       size,
                                                  ArrayDefs::ConstReal_ptr ves,
                                                  ArrayDefs::ConstReal_ptr maxVes,
                                                  const bool               includeChemicalCompaction,
                                                  ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                                                  ArrayDefs::Real_ptr      porosities ) const {

   m_porosity.calculate (size, ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm, porosities );
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::getPorosity ( const unsigned int       size,
                                                  ArrayDefs::ConstReal_ptr ves,
                                                  ArrayDefs::ConstReal_ptr maxVes,
                                                  const bool               includeChemicalCompaction,
                                                  ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                                                  MultiCompoundProperty&   porosities,
                                                  ArrayDefs::Real_ptr      porosityDerivative ) const {

   if ( porosities.getNumberOfLithologies () != m_lithoComponents.size ()) {
      throw formattingexception::GeneralException ()
         << "Number of lithologies in multi component property does not equal the number of lithologies in the compound lithology.";
   }

   for ( size_t i = 0; i < m_lithoComponents.size (); ++i ) {
      m_lithoComponents [ i ]->porosity (size, ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm, porosities.getSimpleData ( i ));
   }

   if ( porosityDerivative == nullptr ) {
      m_porosity.calculate (size, ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm, porosities.getMixedData ());
   } else {
      m_porosity.calculate (size, ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm, porosities.getMixedData (), porosityDerivative );
   }
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkPermeabilityNP ( const double            ves,
                                                             const double            maxVes,
                                                             const CompoundProperty& porosity,
                                                             double&                 permeabilityNormal,
                                                             double&                 permeabilityPlane ) const {

   int numLitho = m_lithoComponents.size();
   int componentIndex = 0;

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   percentContainer::const_iterator percentIter = m_componentPercentage.begin();

   //legacy behaviour
   if (m_isLegacy)
   {
      if (m_mixmodeltype == Interface::HOMOGENEOUS || m_isFaultLithology)
      {
         permeabilityNormal = 1.0;
         permeabilityPlane = 1.0;
      }
      else if (m_mixmodeltype == Interface::LAYERED)
      {
         permeabilityNormal = 0.0;
         permeabilityPlane = 0.0;

      }
      else
      {
         permeabilityNormal = Interface::DefaultUndefinedMapValue;
         permeabilityPlane = Interface::DefaultUndefinedMapValue;
         return;
      }

      while (m_lithoComponents.end() != componentIter)
      {
         double volFrac = (double) (*percentIter)*0.01;
         double permVal = (*componentIter)->permeability( ves, maxVes, NumericFunctions::Maximum( porosity( componentIndex ), minimumCompoundPorosity( componentIndex ) ) );
         double permAniso = (*componentIter)->getPermAniso();

         if (m_mixmodeltype == Interface::HOMOGENEOUS || m_isFaultLithology)
         {
            permeabilityNormal *= pow( permVal, volFrac );
            permeabilityPlane *= pow( permVal*permAniso, volFrac );
         }
         else if (m_mixmodeltype == Interface::LAYERED)
         {
            permeabilityNormal = permeabilityNormal + volFrac / permVal;
            permeabilityPlane = permeabilityPlane + volFrac * permAniso * permVal;
         }

         ++componentIndex;
         ++componentIter;
         ++percentIter;
      }

      if (!m_isFaultLithology && m_mixmodeltype == Interface::LAYERED)
      {
         permeabilityNormal = 1.0 / permeabilityNormal;
      }
   }
   //new rock property library
   else
   {
      switch (numLitho)
      {
      case 1:
         permeabilityNormal = (*componentIter)->permeability( ves, maxVes, NumericFunctions::Maximum( porosity( componentIndex ), minimumCompoundPorosity( componentIndex ) ) );
         permeabilityPlane = permeabilityNormal * (*componentIter)->getPermAniso();
         break;

      case 2:
      case 3:
         std::array<double,MaximumNumberOfLithologies> permVal;

         while (m_lithoComponents.end() != componentIter)
         {
            permVal[componentIndex] = (*componentIter)->permeability( ves, maxVes, NumericFunctions::Maximum( porosity( componentIndex ), minimumCompoundPorosity( componentIndex ) ) );

            ++componentIndex;
            ++componentIter;
         }

         m_permeabilityMixer.mixPermeability ( permVal, permeabilityNormal, permeabilityPlane );
         break;

      default:
         throw fastCauldronException() << "Wrong number of lithologies in CompoundLithology: should be between 1 and 3 ";
         break;
      }
   }

   permeabilityNormal *= MilliDarcyToM2;
   permeabilityPlane *= MilliDarcyToM2;
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkPermeabilityNP ( const unsigned int           size,
                                                             ArrayDefs::ConstReal_ptr     ves,
                                                             ArrayDefs::ConstReal_ptr     maxVes,
                                                             const MultiCompoundProperty& porosities,
                                                             ArrayDefs::Real_ptr          permeabilityNormal,
                                                             ArrayDefs::Real_ptr          permeabilityPlane,
                                                             PermeabilityWorkSpaceArrays& workSpace ) const {

   if ( porosities.getNumberOfLithologies () != m_lithoComponents.size ()) {
      throw formattingexception::GeneralException ()
         << "Number of lithologies in multi component property does not equal the number of lithologies in the compound lithology.";
   }

   if ( m_lithoComponents.size () == 1 ) {

      double permeabilityAnisotropy = m_lithoComponents [ 0 ]->getPermAniso ();
      m_lithoComponents [ 0 ]->permeability ( size, ves, maxVes, porosities.getSimpleData ( 0 ), permeabilityNormal );

      for ( unsigned int i = 0; i < size; ++i ) {
         permeabilityNormal [ i ] *= MilliDarcyToM2;
         permeabilityPlane [ i ] = permeabilityAnisotropy * permeabilityNormal [ i ];
      }

   } else {

      if ( m_lithoComponents.size () == 1 ) {
         m_lithoComponents [ 0 ]->permeability ( size, ves, maxVes, porosities.getSimpleData ( 0 ), workSpace.getData ( 0 ));
      } else if ( m_lithoComponents.size () == 2 ) {
         m_lithoComponents [ 0 ]->permeability ( size, ves, maxVes, porosities.getSimpleData ( 0 ), workSpace.getData ( 0 ));
         m_lithoComponents [ 1 ]->permeability ( size, ves, maxVes, porosities.getSimpleData ( 1 ), workSpace.getData ( 1 ));
      } else if ( m_lithoComponents.size () == 3 ) {
         m_lithoComponents [ 0 ]->permeability ( size, ves, maxVes, porosities.getSimpleData ( 0 ), workSpace.getData ( 0 ));
         m_lithoComponents [ 1 ]->permeability ( size, ves, maxVes, porosities.getSimpleData ( 1 ), workSpace.getData ( 1 ));
         m_lithoComponents [ 2 ]->permeability ( size, ves, maxVes, porosities.getSimpleData ( 2 ), workSpace.getData ( 2 ));
      } else {
         throw fastCauldronException () << "Incorrect number of lithologies in compound lithology.";
      }

      m_permeabilityMixer.mixPermeabilityArray ( size, workSpace, permeabilityNormal, permeabilityPlane );
   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkPermeabilityNP(const double  ves,
                                                           const double  maxVes,
                                                           const double  porosity,
                                                           double& permeabilityNormal,
                                                           double& permeabilityPlane) const {

   CompoundProperty compoundPorosity;

   compoundPorosity(0) = porosity;
   compoundPorosity(1) = porosity;
   compoundPorosity(2) = porosity;
   compoundPorosity.setMixedProperty(porosity);

   calcBulkPermeabilityNP(ves, maxVes, compoundPorosity, permeabilityNormal, permeabilityPlane);
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkPermeabilityNPDerivativeWRTVes(
   const double            ves,
   const double            maxVes,
   const CompoundProperty& Porosity,
   const double            porosityDerivativeWrtVes,
   double&           Permeability_Derivative_Normal,
   double&           Permeability_Derivative_Plane) const
{
   try
   {
      const int lithologiesNum = m_lithoComponents.size();

      if (m_isFaultLithology && (lithologiesNum > 1))
      {
         throw fastCauldronException() << "Cannot handle multiple lithologies with a fault";
      }

      std::array<double,MaximumNumberOfLithologies> derivativesWRTVes;
      std::array<double,MaximumNumberOfLithologies> permeabilities;

      for (int lithoIdx = 0; lithoIdx < lithologiesNum; ++lithoIdx)
      {
         m_lithoComponents[lithoIdx]->permeabilityDerivative( ves,
                                                              maxVes,
                                                              NumericFunctions::Maximum( Porosity( lithoIdx ), minimumCompoundPorosity( lithoIdx ) ),
                                                              porosityDerivativeWrtVes,
                                                              permeabilities[lithoIdx],
                                                              derivativesWRTVes[lithoIdx] );
      }

      assert( m_componentPercentage.size() == lithologiesNum );

      //legacy behaviour
      if (m_isLegacy)
      {
         std::vector<double> volumeFraction( m_componentPercentage );
         std::vector<double> anisotropy( lithologiesNum );
         for (int lithoIdx = 0; lithoIdx < lithologiesNum; ++lithoIdx)
         {
            volumeFraction[lithoIdx] *= 0.01;
            anisotropy[lithoIdx] = m_lithoComponents[lithoIdx]->getPermAniso();
         }

         Permeability_Derivative_Normal = 0.0;
         Permeability_Derivative_Plane = 0.0;

         if (m_isFaultLithology)
         {
            Permeability_Derivative_Normal = derivativesWRTVes[0];
            Permeability_Derivative_Plane = derivativesWRTVes[0];
         }
         else if (m_mixmodeltype == Interface::LAYERED)
         {
            //
            // Mixed permeability derivative with respect to ves in the NORMAL direction (vertical permeability derivative)
            //
            // d k_n                                     w_i    d k_i
            // ----- = [sum( w_i / k_i )] ^ -2  *  sum( ----- * ----- )
            // d ves     i                          i   k_i^2   d ves
            //
            // Mixed permeability derivative with respect to ves in the PLANE direction (horizontal permeability derivative)
            //
            // d k_p                      d k_i
            // ----- = sum_i( w_i * a_i * ----- )
            // d ves                      d ves
            //
            // w_i : volume fraction
            // a_i : anisotropy

            double termA( 0.0 );
            double termB( 0.0 );
            for (int lithoIdx = 0; lithoIdx < lithologiesNum; ++lithoIdx)
            {
               double wOverK = volumeFraction[lithoIdx] / permeabilities[lithoIdx];
               termA += wOverK;
               termB += wOverK * derivativesWRTVes[lithoIdx] / permeabilities[lithoIdx];
               Permeability_Derivative_Plane += volumeFraction[lithoIdx] * derivativesWRTVes[lithoIdx] * anisotropy[lithoIdx];
            }
            Permeability_Derivative_Normal = termB / (termA * termA);
         }
         else if (m_mixmodeltype == Interface::HOMOGENEOUS)
         {
            //
            // Mixed permeability derivative with respect to ves in the NORMAL direction (vertical permeability derivative)
            //
            // d k_n                                  d k_i
            // ----- = sum( w_i * k_i ^ ( w_i - 1 ) * ----- * prod( k_j ^ w_j ) )
            // d ves    i                             d ves   j!=i
            //
            // Mixed permeability derivative with respect to ves in the PLANE direction (horizontal permeability derivative)
            //
            // d k_p                                                  d k_i
            // ----- = sum( w_i * a_i * ( a_i * k_i ) ^ ( w_i - 1 ) * ----- * prod( ( a_j * k_j ) ^ w_j ) )
            // d ves    i                                             d ves   j!=i
            //
            // w_i : volume fraction
            // a_i : anisotropy

            for (int lithoIdx1 = 0; lithoIdx1 < lithologiesNum; ++lithoIdx1)
            {
               double productsSequenceN( 1.0 );
               double productsSequenceP( 1.0 );
               for (int lithoIdx2 = 0; lithoIdx2 < lithologiesNum; ++lithoIdx2)
               {
                  if (lithoIdx1 != lithoIdx2)
                  {
                     productsSequenceN *= std::pow( permeabilities[lithoIdx2], volumeFraction[lithoIdx2] );
                     productsSequenceP *= std::pow( anisotropy[lithoIdx2] * permeabilities[lithoIdx2], volumeFraction[lithoIdx2] );
                  }
               }
               Permeability_Derivative_Normal += volumeFraction[lithoIdx1] * std::pow( permeabilities[lithoIdx1], volumeFraction[lithoIdx1] - 1.0 ) *
                  derivativesWRTVes[lithoIdx1] * productsSequenceN;
               Permeability_Derivative_Plane += volumeFraction[lithoIdx1] * anisotropy[lithoIdx1] *
                  std::pow( anisotropy[lithoIdx1] * permeabilities[lithoIdx1], volumeFraction[lithoIdx1] - 1.0 ) *
                  derivativesWRTVes[lithoIdx1] * productsSequenceP;
            }
         }
         else
         {
            throw fastCauldronException() << "Undefined mixing model type";
         }
      }
      //new rock property library behaviour
      else
      {

         Permeability_Derivative_Normal = 0.0;
         Permeability_Derivative_Plane = 0.0;

         switch (lithologiesNum)
         {
         case 1:
            Permeability_Derivative_Normal = derivativesWRTVes[0];
            Permeability_Derivative_Plane = m_lithoComponents[0]->getPermAniso()*derivativesWRTVes[0];
            break;

         case 2:
         case 3:
            m_permeabilityMixer.mixPermeabilityDerivatives ( permeabilities,
                                                             derivativesWRTVes,
                                                             Permeability_Derivative_Normal,
                                                             Permeability_Derivative_Plane );

            break;

         default:
            throw fastCauldronException() << "Wrong number of lithologies in CompoundLithology: should be between 1 and 3 ";
            break;
         }
      }
   }

   catch (fastCauldronException & ex)
   {
      throw ex;
   }
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::setChemicalCompactionTerms
(const double rockViscosity,
const double activationEnergy) {

   m_referenceSolidViscosity = rockViscosity;
   m_lithologyActivationEnergy = activationEnergy;
}

//------------------------------------------------------------//

bool GeoPhysics::CompoundLithology::hasHydraulicallyFractured(const double hydrostaticPressure,
                                                              const double porePressure,
                                                              const double lithostaticPressure) const {

   return porePressure > fracturePressure(hydrostaticPressure, lithostaticPressure);

}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::fracturePressure(const double hydrostaticPressure,
                                                       const double lithostaticPressure) const {
   return  m_lithologyFractureGradient * (lithostaticPressure - hydrostaticPressure) + hydrostaticPressure;
}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::computePorosityDerivativeWRTVes(const double ves,
                                                                      const double maxVes,
                                                                      const bool   includeChemicalCompaction,
                                                                      const double chemicalCompactionTerm) const {

   return m_porosity.calculateDerivative(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::mixCapillaryEntryPressureCofficients()
{

   m_capC1 = m_capC2 = 0.0;
   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();
   double pcMult;

   //legacy behaviour
   if (m_isLegacy)
   {
      if (m_mixmodeltype == Interface::HOMOGENEOUS)
      {
         while (m_lithoComponents.end() != componentIter)
         {
            pcMult = (double) (*percentIter) / 100;

            m_capC1 += (*componentIter)->getCapC1() * pcMult;
            m_capC2 += (*componentIter)->getCapC2() * pcMult;

            ++componentIter;
            ++percentIter;
         }
      }
      else
      {
         // if (m_mixmodeltype == Interface::LAYERED){
#ifdef _MSC_VER
#undef max
         // To make sure the next statement is using the correct max() function
#endif
         double depoPerm = numeric_limits<double>::max();
         double minDepoPerm = numeric_limits<double>::max();

         while ( m_lithoComponents.end() != componentIter )
         {
            depoPerm = (*componentIter)->getDepoPerm();
            if ( depoPerm < minDepoPerm )
            {
               m_capC1 = (*componentIter)->getCapC1();
               m_capC2 = (*componentIter)->getCapC2();
               minDepoPerm = depoPerm;
            }
            ++componentIter;
         }
      }
   }
   //new rock property behaviour
   else
   {
      // Capillary entry pressure coefficients are mixed with an arithmetic mean
      //Note:  mixing type is not considered (homogeneous, layered)
      while (m_lithoComponents.end() != componentIter)
      {
         pcMult = (double) (*percentIter) / 100;

         m_capC1 += (*componentIter)->getCapC1() * pcMult;
         m_capC2 += (*componentIter)->getCapC2() * pcMult;

         ++componentIter;
         ++percentIter;
      }
   }

   m_tenPowerCapC2 = pow( 10.0, -m_capC2 );

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::mixBrooksCoreyParameters()
{
   std::vector<double> lambdaPc, lambdaKr;

   compContainer::iterator componentIter = m_lithoComponents.begin();
   while (m_lithoComponents.end() != componentIter)
   {
      lambdaPc.push_back ( (*componentIter)->getLambdaPc() );
      lambdaKr.push_back ( (*componentIter)->getLambdaKr() );

      ++componentIter;
   }

   GeoPhysics::LambdaMixer pcMixer;
   m_LambdaPc = pcMixer.mixLambdas(m_componentPercentage, lambdaPc);

   GeoPhysics::LambdaMixer krMixer;
   m_LambdaKr = krMixer.mixLambdas(m_componentPercentage, lambdaKr);

   return;
}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::mixModulusSolid() const
{
   double modulusSolid;
   double currentModlusSolid = 0;
   double currentWeight = 0;

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   percentContainer::const_iterator percentIter = m_componentPercentage.begin();

   //legacy behaviour
   if (m_isLegacy)
   {
      switch (m_mixmodeltype)
      {
      case Interface::HOMOGENEOUS || Interface::UNDEFINED:
         modulusSolid = 1;
         break;
      case Interface::LAYERED:
         modulusSolid = 0;
         break;
      default:
         modulusSolid = 1;
      }
      while (m_lithoComponents.end() != componentIter)
      {
         currentWeight = (double) (*percentIter) / 100;
         currentModlusSolid = (*componentIter)->getDensity()*pow( (*componentIter)->getSeismicVelocity(), 2 );

         switch (m_mixmodeltype)
         {
         case Interface::HOMOGENEOUS || Interface::UNDEFINED:
            // geometric mean
            modulusSolid *= pow( currentModlusSolid, currentWeight );
            break;
         case Interface::LAYERED:
            // harmonic mean
            modulusSolid += currentWeight / currentModlusSolid;
            break;
         default:
            // geometric mean
            modulusSolid *= pow( currentModlusSolid, currentWeight );
         }

         ++componentIter;
         ++percentIter;
      }

      switch (m_mixmodeltype)
      {
      case Interface::HOMOGENEOUS || Interface::UNDEFINED:
         return modulusSolid;
      case Interface::LAYERED:
         return 1 / modulusSolid;
      default:
         return modulusSolid;
      }
   }
   //new rock property behaviour
   else
   {
      //Note:  mixing type is not considered (homogeneous, layered)
      // Mixed with geometric mean
      modulusSolid = 1;

      while (m_lithoComponents.end() != componentIter)
      {
         currentWeight = (double) (*percentIter) / 100;
         currentModlusSolid = (*componentIter)->getDensity()*pow( (*componentIter)->getSeismicVelocity(), 2 );
         modulusSolid *= pow( currentModlusSolid, currentWeight );
         ++componentIter;
         ++percentIter;
      }
      return modulusSolid;
   }
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkThermCondNPBasement(const FluidType* fluid,
                                                                double           Porosity,
                                                                double           Temperature,
                                                                double           LithoPressure,
                                                                double&          BulkTHCondN,
                                                                double&          BulkTHCondP) const {

   bool LithoHasFluid = false;
   if (fluid != 0) LithoHasFluid = true;

   if (this->m_lithoComponents.size() != 1) {
      cerr << "Few lithologies in basement." << endl;
   }

   SimpleLithology * currentLitho = m_lithoComponents[0];

   if (m_lithoComponents[0]->getName() == DataAccess::Interface::ALCBasalt) {
      BulkTHCondN = currentLitho->basaltThermalConductivity(Temperature, LithoPressure);
      BulkTHCondP = BulkTHCondN;
   } else {
      double MatrixTHCondN = 0.0, MatrixTHCondP = 0.0;

      if ((currentLitho->getThermalCondModel() == Interface::TABLE_MODEL or currentLitho->getThermalCondModel() == Interface::CONSTANT_MODEL)) {
         MatrixTHCondN = thermalconductivityN(Temperature);
         MatrixTHCondP = thermalconductivityP(Temperature);
      }
      else {
         MatrixTHCondP = (MatrixTHCondN = currentLitho->thermalconductivity(Temperature, LithoPressure));
      }

      double FluidThCond = 1.0;
      /* There is no fluid in the basement
         if (LithoHasFluid) {
         FluidThCond = fluid->thermalConductivity(Temperature);
         }
      */
      BulkTHCondN = pow(MatrixTHCondN, 1.0 - Porosity) * pow(FluidThCond, Porosity);
      BulkTHCondP = pow(MatrixTHCondP, 1.0 - Porosity) * pow(FluidThCond, Porosity);
   }
}

void GeoPhysics::CompoundLithology::calcBulkThermCondNPBasement ( const unsigned int size,
                                                                  const double* temperature,
                                                                  const double* lithostaticPressure,
                                                                  double*       bulkThermalCondN,
                                                                  double*       bulkThermalCondP ) const {

   if (this->m_lithoComponents.size() != 1) {
      cerr << "Few lithologies in basement." << endl;
   }

   SimpleLithology * currentLitho = m_lithoComponents[0];

   if ( currentLitho->getName() == DataAccess::Interface::ALCBasalt) {

      for ( unsigned int i = 0; i < size; ++i ) {
         bulkThermalCondN [ i ] = currentLitho->basaltThermalConductivity ( temperature [ i ], lithostaticPressure [ i ]);
         bulkThermalCondP [ i ] = bulkThermalCondN [ i ];
      }

   } else {

      if ( currentLitho->getThermalCondModel() == Interface::TABLE_MODEL or
           currentLitho->getThermalCondModel() == Interface::CONSTANT_MODEL ) {

         for ( unsigned int i = 0; i < size; ++i ) {
            bulkThermalCondN [ i ] = thermalconductivityN ( temperature [ i ]);
            bulkThermalCondP [ i ] = thermalconductivityP ( temperature [ i ]);
         }

      } else {

         for ( unsigned int i = 0; i < size; ++i ) {
            bulkThermalCondN [ i ] = currentLitho->thermalconductivity ( temperature [ i ], lithostaticPressure [ i ]);
            bulkThermalCondP [ i ] = bulkThermalCondN [ i ];
         }

      }

   }

}


//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkThermCondNPBasalt(double           Temperature,
                                                              double           LithoPressure,
                                                              double&          BulkTHCondN,
                                                              double&          BulkTHCondP) const {

   if (this->m_lithoComponents.size() != 1) {
      cerr << "Few lithologies in basement." << endl;
   }
   // here we assume that the lithology is ALCBasalt
   BulkTHCondN = (BulkTHCondP = m_lithoComponents[0]->basaltThermalConductivity(Temperature, LithoPressure));
}
