#include "CompoundLithology.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <vector>

#include "Interface/Interface.h"

#include "capillarySealStrength.h"
#include "GeoPhysicsProjectHandle.h"
#include "NumericFunctions.h"
#include "Quadrature.h"


using namespace DataAccess;
using namespace CBMGenerics;
using namespace capillarySealStrength;

//#define NOPRESSURE 1

GeoPhysics::CompoundLithology::CompoundLithology(GeoPhysics::ProjectHandle* projectHandle) : m_projectHandle(projectHandle)
, m_porosity()
{

   m_mixmodeltype = HOMOGENEOUS;
   m_lithologyFractureGradient = 1.0;
   m_isFaultLithology = false;

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

   if (m_mixmodeltype == HOMOGENEOUS) {
      mixingModel = "Homogeneous";
   }
   else if (m_mixmodeltype == LAYERED) {
      mixingModel = "Layered";
   }
   else {
      mixingModel = "Undefined";
   }

   if (m_lithoComponents.size() == 1) {
      fraction1 = m_componentPercentage[0];
      fraction2 = 0.0;
      fraction3 = 0.0;
      simpleName1 = m_lithoComponents[0]->getName();
      simpleName2 = "";
      simpleName3 = "";
   }
   else if (m_lithoComponents.size() == 2) {
      fraction1 = m_componentPercentage[0];
      fraction2 = m_componentPercentage[1];
      fraction3 = 0.0;
      simpleName1 = m_lithoComponents[0]->getName();
      simpleName2 = m_lithoComponents[1]->getName();
      simpleName3 = "";
   }
   else if (m_lithoComponents.size() == 3) {
      fraction1 = m_componentPercentage[0];
      fraction2 = m_componentPercentage[1];
      fraction3 = m_componentPercentage[2];
      simpleName1 = m_lithoComponents[0]->getName();
      simpleName2 = m_lithoComponents[1]->getName();
      simpleName3 = m_lithoComponents[2]->getName();
   }
   else {
      fraction1 = 0.0;
      fraction2 = 0.0;
      fraction3 = 0.0;
      simpleName1 = "";
      simpleName2 = "";
      simpleName3 = "";
   }

   return CompoundLithologyComposition(simpleName1, simpleName2, simpleName3, fraction1, fraction2, fraction3, mixingModel);
}
//------------------------------------------------------------//


bool GeoPhysics::CompoundLithology::isBasement() const {

   const string lithoname = m_lithoComponents[0]->getName();
   if ((this->m_lithoComponents.size() == 1) && (lithoname == "Crust" || lithoname == "Litho. Mantle" || lithoname == DataAccess::Interface::ALCBasalt)) {
      return true;
   }
   return false;
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

   // Only initialised to prevent compiler warning, the warning is erroneous.
   double ThermalCondN = 0.0;

   // Only initialised to prevent compiler warning, the warning is erroneous.
   double ThermalCondP = 0.0;

   vector<ibs::XF> mixThermCondNTbl, mixThermCondPTbl;
   vector<ibs::XF>& thermCondTbl1 = (*componentIter)->getThermCondTbl();
   vector<ibs::XF>::iterator XYIter = thermCondTbl1.begin();

   Table_Size = thermCondTbl1.size();
   Array_Size = min(Array_Size, Table_Size);

   while (thermCondTbl1.end() != XYIter) {

      if (m_mixmodeltype == HOMOGENEOUS) {
         ThermalCondN = ThermalCondP = 1.0;
         ThermalCondN *= pow((*XYIter).getF(), (*percentIter)*0.01);
         ThermalCondP *= pow((*XYIter).getF()*anisotropy, (*percentIter)*0.01);
      }
      else if (m_mixmodeltype == LAYERED) {
         ThermalCondN = ThermalCondP = 0.0;
         ThermalCondN += (*percentIter)*0.01 / (*XYIter).getF();
         ThermalCondP += (*percentIter)*0.01 * (*XYIter).getF() * anisotropy;
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
      //The following line should be uncommented in FastCauldron Release - OFM
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

         if (m_mixmodeltype == HOMOGENEOUS) {
            ThermalCondN *= pow((*XYIter).getF(), (*percentIter)*0.01);
            ThermalCondP *= pow((*XYIter).getF()*anisotropy, (*percentIter)*0.01);
         }
         else if (m_mixmodeltype == LAYERED) {
            ThermalCondN += (*percentIter)*0.01 / (*XYIter).getF();
            ThermalCondP += (*percentIter)*0.01 * (*XYIter).getF() * anisotropy;
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
      if (m_mixmodeltype == LAYERED && (*mixXYnIter).getF() != 0.0) {
         addThermCondPointN((*mixXYnIter).getX(), 1 / (*mixXYnIter).getF());
      }
      else {
         addThermCondPointN((*mixXYnIter).getX(), (*mixXYnIter).getF());
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


double GeoPhysics::CompoundLithology::hydrostatFullCompThickness(const double maxVes,
   const double thickness,
   const double densitydiff,
   const bool   overpressuredCompaction) const {

   /*This module is based on Lithology::CalcFullCompactedThickness method of Cauldron */

   /* if the compaction coefficient is nil then the full compacted thickness = thickness */

   ///
   /// If running an overpressure calculation then make an estimate how much 
   /// overpressure there might be. More overpressure implies less ves. The scaling
   /// factor is based on the amount of ves there would be if the basin was hydrostatically
   /// pressured.
   ///
   const double vesScaleFactor = 0.5;

#if 0
   const double vesScaleFactor = m_projectHandle->getCommandLineParameters()->getVesScaling();
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

   return m_porosity.porosity(sigma, sigma_max, includeChemicalCompaction, chemicalCompactionTerm);
}
//------------------------------------------------------------//


bool GeoPhysics::CompoundLithology::allowableMixing() const {

   bool Mixing_Okay = true;

   compContainer::const_iterator    Simple_Lithologies = m_lithoComponents.begin();
   percentContainer::const_iterator Percentages = m_componentPercentage.begin();

   if (m_lithoComponents.size() > 1) {
      // Only initialised to prevent compiler warning, the warning is erroneous. Code can be re-arranged to prevent the warning.
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

      std::cout << "****************  ERROR  Incorrect mixing  ****************" << std::endl;
      std::cout << "   Lithotype           Porosity model   percentage" << std::endl;

      while (m_lithoComponents.end() != Simple_Lithologies) {
         std::cout << std::setw(20) << setiosflags(ios::left) << (*Simple_Lithologies)->getName() << resetiosflags(ios::left)
            //            << std::setw ( 18 ) << PorosityModelTypeImage [(*Simple_Lithologies)->getPorosityModel ()]
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

   m_density = 0.0;
   m_seismicVelocitySolid = 0.0;

   m_depositionalPermeability       = 0.0;
   m_heatProduction                 = 0.0;
   m_permeabilityincr               = 0.0;
   m_permeabilitydecr               = 0.0;
   m_thermalConductivityAnisotropy  = 1.0;
   m_thermalConductivityValue       = 0.0;
   m_specificSurfaceArea            = 1.0;
   m_geometricVariance              = 1.0;

   // loop through all the simple lithologies and calculate the
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

   while (m_lithoComponents.end() != componentIter) {
      double pcMult = (double)(*percentIter) / 100;

      // Matrix Property calculated using the arithmetic mean
      m_density                     += (*componentIter)->getDensity()  * pcMult;
      m_depositionalPermeability    += (*componentIter)->getDepoPerm() * pcMult;
      m_thermalConductivityValue    += (*componentIter)->getThCondVal() * pcMult;
      m_heatProduction              += (*componentIter)->getHeatProduction() * pcMult;
      m_seismicVelocitySolid        += (*componentIter)->getSeismicVelocity() * pcMult;
      m_referenceSolidViscosity     += (*componentIter)->getReferenceSolidViscosity() * pcMult;
      m_lithologyActivationEnergy   += (*componentIter)->getLithologyActivationEnergy() * pcMult;
      minimumMechanicalPorosity     += (*componentIter)->getMinimumMechanicalPorosity() / 100.0 * pcMult;
      m_lithologyFractureGradient   += (*componentIter)->getLithologyFractureGradient() * pcMult;
      m_quartzFraction              += (*componentIter)->getQuartzFraction() * pcMult;
      m_coatingClayFactor           += (*componentIter)->getClayCoatingFactor() * pcMult;

      // Matrix Property calculated using the geometric mean
      m_thermalConductivityAnisotropy *= pow((*componentIter)->getThCondAn(), pcMult);

      m_specificSurfaceArea *= pow((*componentIter)->getSpecificSurfArea(), pcMult);
      m_geometricVariance *= pow((*componentIter)->getGeometricVariance(), pcMult);

      //Matrix Property calculated using the algebraic mean
      m_quartzGrainSize += pcMult * pow((*componentIter)->getQuartzGrainSize(), 3.0);
      ++componentIter;
      ++percentIter;
   }
   m_quartzGrainSize = pow(m_quartzGrainSize, 1.0 / 3.0);
   //Temporary values before mixing
   DataAccess::Interface::PorosityModel porosityModel;
   double surfacePorosity;
   double surfaceVoidRatio;
   double compactionincr;
   double compactionincrA;
   double compactionincrB;
   double compactiondecr;
   double compactiondecrA;
   double compactiondecrB;
   double soilMechanicsCompactionCoefficient;

   mixPorosityModel(porosityModel);
   mixSurfacePorosity(porosityModel, surfacePorosity, surfaceVoidRatio);
   mixCompactionCoefficients(compactionincr, compactionincrA, compactionincrB, compactiondecr, compactiondecrA, compactiondecrB, soilMechanicsCompactionCoefficient);
    
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
   soilMechanicsCompactionCoefficient);

   reSetBehaviorForHysteresis();
   mixCapillaryEntryPressureCofficients();
   mixBrooksCoreyParameters();
   createThCondTbl();

   setMinimumPorosity(porosityModel, surfaceVoidRatio, soilMechanicsCompactionCoefficient);



   if (surfacePorosity < 0.0299) {
      // Really less than 0.03 but some rounding may occur if user set a litho with 0.03 surface porosity
      m_fracturedPermeabilityScalingValue = 100.0;
   }
   else {
      m_fracturedPermeabilityScalingValue = 10.0; // What to set this to?  10, 50 or whatever.
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
      surfacePorosity = 0.0 ;

      /* Dominant lithotype defines Porosity Model and Compaction Coefficient value */
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
		double& soilMechanicsCompactionCoefficient) {
   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   soilMechanicsCompactionCoefficient = 0.0;
   compactionincr  = 0.0;
   compactionincrA = 0.0;
   compactionincrB = 0.0;
   compactiondecr  = 0.0;
   compactiondecrA = 0.0;
   compactiondecrB = 0.0;

   double Fraction;

   while (m_lithoComponents.end() != componentIter) {
      
      Fraction = 0.01 * (*percentIter);
 
      soilMechanicsCompactionCoefficient = soilMechanicsCompactionCoefficient + (*componentIter)->getCompCoeff() * Fraction;
      compactionincr = compactionincr + (*componentIter)->getCompCoeff() * Fraction;
      compactionincrA = compactionincrA + (*componentIter)->getCompCoeffA() * Fraction;
      compactionincrB = compactionincrB + (*componentIter)->getCompCoeffB() * Fraction;
       
      compactiondecr = compactiondecr + 0.1 * (*componentIter)->getCompCoeff() * Fraction;
      compactiondecrA = compactiondecrA + 0.1 * (*componentIter)->getCompCoeffA() * Fraction;
      compactiondecrB = compactiondecrB + 0.1 * (*componentIter)->getCompCoeffB() * Fraction;
      
      ++componentIter;
      ++percentIter;
   }
}
//------------------------------------------------------------//


void   GeoPhysics::CompoundLithology::mixPorosityModel(DataAccess::Interface::PorosityModel & porosityModel) {

   double Largest_Volume_Fraction = -100.0;

   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   /* Dominant lithotype defines Porosity Model and Compaction Coefficient value */
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


void GeoPhysics::CompoundLithology::reSetBehaviorForHysteresis() {

   double VolFrac = 0.0, SandVolFrac = 0.0, ShaleVolFrac = 0.0;
   bool   NoPerm = false;

   // loop through all the simple lithologies and calculate the
   // properties for this lithology
   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();
   while (m_lithoComponents.end() != componentIter) {
      VolFrac = (double)(*percentIter) / 100;

      switch ((*componentIter)->getPermeabilityModel())
      {
      case DataAccess::Interface::SANDSTONE_PERMEABILITY: {
         SandVolFrac += VolFrac;
         break;
      }
      case DataAccess::Interface::MUDSTONE_PERMEABILITY: {
         ShaleVolFrac += VolFrac;
         break;
      }
      case DataAccess::Interface::MULTIPOINT_PERMEABILITY: {
         ShaleVolFrac += VolFrac;
         break;
      }
      case DataAccess::Interface::IMPERMEABLE_PERMEABILITY: {
         NoPerm = true;
         break;
      }
      case DataAccess::Interface::NONE_PERMEABILITY: {
         NoPerm = true;
         break;
      }
      default: {
         assert(false);
      }
      }
      ++componentIter;
      ++percentIter;
   }

   if (m_mixmodeltype == HOMOGENEOUS) {
      if (!NoPerm)
      {
         if (SandVolFrac > 50.0)
         {
            m_hysteresisbehaviour = SAND_BEHAVIOR;
         }
         else
         {
            m_hysteresisbehaviour = SHALE_BEHAVIOR;
         }
      }
      else
      {
         m_hysteresisbehaviour = NO_PERM_BEHAVIOR;
      }
   }
   else if (m_mixmodeltype == LAYERED) {
      if (!NoPerm)
      {
         if (ShaleVolFrac > 0.0)
         {
            m_hysteresisbehaviour = SHALE_BEHAVIOR;
         }
         else
         {
            m_hysteresisbehaviour = SAND_BEHAVIOR;
         }
      }
      else
      {
         m_hysteresisbehaviour = NO_PERM_BEHAVIOR;
      }
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

void GeoPhysics::CompoundLithology::setMixModel(const std::string& mixmodel) {

   if (mixmodel == "Homogeneous") {
      m_mixmodeltype = HOMOGENEOUS;
   }
   else if (mixmodel == "Layered") {
      m_mixmodeltype = LAYERED;
   }
   else {
      m_mixmodeltype = UNDEFINED;
      std::cout << "mixmodel not defined: " << mixmodel << std::endl;
   }

}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::computeSegmentThickness(const double topMaxVes,
   const double botMaxVes,
   const double densityDifference,
   const double solidThickness) const {

   if (m_porosity.getPorosityModel() == DataAccess::Interface::EXPONENTIAL_POROSITY) {

      double d1 = exponentialDecompactionFunction(topMaxVes);
      double d2 = exponentialDecompactionFunction(botMaxVes);
      return (d2 - d1) / (AccelerationDueToGravity * densityDifference);
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
   const double densityDifference,
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

      double FluidDensXHeatCap = fluid->densXheatCapacity(Temperature, Pressure, m_projectHandle->getPermafrost());
      BulkDensXHeatCapacity = MatrixDensXHeatCap * (1.0 - Porosity) + FluidDensXHeatCap * Porosity;

   }
   else {
      // The only layers that have no fluid are the crust and mantle. Since they don't have
      // a porosity, the volumetric heat capacity is the same as that of the lithology.
      BulkDensXHeatCapacity = MatrixDensXHeatCap;

   }
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkDensity(const FluidType* fluid,
   const double     Porosity,
   double&    BulkDensity) const {

   double MatrixDensity = m_density;

   if (fluid != 0) {
      double FluidDensity = fluid->getConstantDensity();

      BulkDensity = MatrixDensity * (1.0 - Porosity) + FluidDensity * Porosity;
   }
   else {
      // Should this be multiplied by ( 1.0 - porosity )?
      BulkDensity = MatrixDensity;
   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkDensity(const FluidType* fluid,
   const double     Porosity,
   const double     porePressure,
   const double     temperature,
   const double     lithoPressure,
   double&    BulkDensity) const {

   bool LithoHasFluid = false;
   if (fluid != 0) LithoHasFluid = true;

   //double MatrixDensity = m_density;
   double MatrixDensity;

#ifdef NOPRESSURE
   MatrixDensity = (!isBasement() ? m_density : m_lithoComponents[0]->getDensity(temperature, 0.0));
#else
   // temporary remove pressure term from the density calculation
   MatrixDensity = (!isBasement() ? m_density : m_lithoComponents[0]->getDensity(temperature, lithoPressure));
#endif

   if (LithoHasFluid) {

      double FluidDensity = fluid->density(temperature, porePressure);

      BulkDensity = (MatrixDensity * (1.0 - Porosity))
         + (FluidDensity * Porosity);

   }
   else {

      // Should this be multiplied by ( 1.0 - porosity )?
      BulkDensity = MatrixDensity;

   }

}

//------------------------------------------------------------//

// use this function in calculation of BulkDensity output property
void GeoPhysics::CompoundLithology::calcBulkDensity1(const FluidType* fluid,
   const double     Porosity,
   const double     porePressure,
   const double     temperature,
   const double     lithoPressure,
   double&    BulkDensity) const {

   bool LithoHasFluid = false;
   if (fluid != 0) LithoHasFluid = true;

   //double MatrixDensity = m_density;
   double MatrixDensity = (!isBasement() ? m_density : m_lithoComponents[0]->getDensity(temperature, lithoPressure));

   if (LithoHasFluid) {

      double FluidDensity = fluid->density(temperature, porePressure);

      BulkDensity = (MatrixDensity * (1.0 - Porosity)) + (FluidDensity * Porosity);

   }
   else {

      // Should this be multiplied by ( 1.0 - porosity )?
      BulkDensity = MatrixDensity;

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

void GeoPhysics::CompoundLithology::getPermeability(const double  ves,
   const double  maxVes,
   const bool    includeChemicalCompaction,
   const double  chemicalCompactionValue,
   double& permeabilityNormal,
   double& permeabilityPlane) const {

   int componentIndex = 0;

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   percentContainer::const_iterator percentIter = m_componentPercentage.begin();

   if (m_mixmodeltype == HOMOGENEOUS or m_isFaultLithology) {
      permeabilityNormal = 1.0;
      permeabilityPlane = 1.0;
   }
   else if (m_mixmodeltype == LAYERED) {
      permeabilityNormal = 0.0;

#if 1
      permeabilityPlane = 0.0;
#else
      permeabilityPlane = 1.0;
#endif

   }
   else {
      permeabilityNormal = Interface::DefaultUndefinedMapValue;
      permeabilityPlane = Interface::DefaultUndefinedMapValue;
      return;
   }

   double volumeFraction;
   double permeabilityValue;
   double permeabilityAnisotropy;
   double porosity;

   while (m_lithoComponents.end() != componentIter) {

      const SimpleLithology* lithology = *componentIter;

      volumeFraction = (double)(*percentIter)*0.01;
      porosity = lithology->porosity(ves, maxVes, includeChemicalCompaction, chemicalCompactionValue);
      porosity = NumericFunctions::Maximum(porosity, minimumCompoundPorosity(componentIndex));

      permeabilityValue = lithology->permeability(ves, maxVes, porosity);
      permeabilityAnisotropy = lithology->getPermAniso();

      if (m_mixmodeltype == HOMOGENEOUS or m_isFaultLithology) {
         permeabilityNormal *= std::pow(permeabilityValue, volumeFraction);
         permeabilityPlane *= std::pow(permeabilityValue * permeabilityAnisotropy, volumeFraction);
      }
      else if (m_mixmodeltype == LAYERED) {

#if 1
         // Average vertical permeabilities using harmonic mean.
         permeabilityNormal += volumeFraction / permeabilityValue;

         // Average parallel permeabilities using arithmetic mean.
         permeabilityPlane += volumeFraction * permeabilityAnisotropy * permeabilityValue;
#else
         // Average vertical permeabilities using harmonic mean.
         permeabilityNormal += volumeFraction / permeabilityValue;

         // Average parallel permeabilities using geometric mean.
         permeabilityPlane *= std::pow(permeabilityValue * permeabilityAnisotropy, volumeFraction);
#endif

      }

      ++componentIndex;
      ++componentIter;
      ++percentIter;
   }

   if (not m_isFaultLithology and m_mixmodeltype == LAYERED) {
      permeabilityNormal = 1.0 / permeabilityNormal;
   }

#if 0
   permeabilityNormal = NumericFunctions::Minimum(permeabilityNormal, 1000.0);
   permeabilityPlane = NumericFunctions::Minimum(permeabilityPlane, 1000.0);
#endif

   permeabilityNormal *= MILLIDARCYTOM2;
   permeabilityPlane *= MILLIDARCYTOM2;

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithology::calcBulkPermeabilityNP(const double            ves,
   const double            maxVes,
   const CompoundProperty& porosity,
   double&           permeabilityNormal,
   double&           permeabilityPlane) const {

   int componentIndex = 0;

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   percentContainer::const_iterator percentIter = m_componentPercentage.begin();

   if (m_mixmodeltype == HOMOGENEOUS || m_isFaultLithology) {
      permeabilityNormal = 1.0;
      permeabilityPlane = 1.0;
   }
   else if (m_mixmodeltype == LAYERED) {
      permeabilityNormal = 0.0;

#if 1
      permeabilityPlane = 0.0;
#else
      permeabilityPlane = 1.0;
#endif

   }
   else {
      permeabilityNormal = Interface::DefaultUndefinedMapValue;
      permeabilityPlane = Interface::DefaultUndefinedMapValue;
      return;
   }

   while (m_lithoComponents.end() != componentIter) {
      double VolFrac = (double)(*percentIter)*0.01;
      double PermVal = (*componentIter)->permeability(ves, maxVes, NumericFunctions::Maximum(porosity(componentIndex), minimumCompoundPorosity(componentIndex)));
      double PermAniso = (*componentIter)->getPermAniso();

      if (m_mixmodeltype == HOMOGENEOUS || m_isFaultLithology) {
         permeabilityNormal *= pow(PermVal, VolFrac);
         permeabilityPlane *= pow(PermVal*PermAniso, VolFrac);
      }
      else if (m_mixmodeltype == LAYERED) {

#if 1
         permeabilityNormal = permeabilityNormal + VolFrac / PermVal;
         permeabilityPlane = permeabilityPlane + VolFrac * PermAniso * PermVal;
#else
         permeabilityNormal += VolFrac / PermVal;
         permeabilityPlane *= pow(PermVal*PermAniso, VolFrac);
#endif

      }

      ++componentIndex;
      ++componentIter;
      ++percentIter;
   }

   if (!m_isFaultLithology && m_mixmodeltype == LAYERED) {
      permeabilityNormal = 1.0 / permeabilityNormal;
   }

#if 0
   permeabilityNormal = NumericFunctions::Minimum(permeabilityNormal, 1000.0);
   permeabilityPlane = NumericFunctions::Minimum(permeabilityPlane, 1000.0);
#endif

   permeabilityNormal *= MILLIDARCYTOM2;
   permeabilityPlane *= MILLIDARCYTOM2;
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


void GeoPhysics::CompoundLithology::calcBulkPermeabilityNPDerivative(const double            ves,
   const double            maxVes,
   const CompoundProperty& Porosity,
   const double            porosityDerivativeWrtVes,
   double&           Permeability_Derivative_Normal,
   double&           Permeability_Derivative_Plane) const {

   double Derivatives[MaximumNumberOfLithologies];
   double Permeabilities[MaximumNumberOfLithologies];
   double fraction;
   double derivativeTerm;
   int componentCount = 0;

   compContainer::const_iterator componentIter = m_lithoComponents.begin();
   while (m_lithoComponents.end() != componentIter) {

      (*componentIter)->permeabilityDerivative(ves, maxVes,
         NumericFunctions::Maximum(Porosity(componentCount), minimumCompoundPorosity(componentCount)),
         porosityDerivativeWrtVes,
         Permeabilities[componentCount],
         Derivatives[componentCount]);
      ++componentCount;
      ++componentIter;
   }

   componentIter = m_lithoComponents.begin();

   if (m_mixmodeltype == LAYERED && !m_isFaultLithology)
   {
      // We should not take the minimum, but add with weight percentages
      Permeability_Derivative_Normal = 0.0;
      Permeability_Derivative_Plane = 0.0;
      componentCount = 0;
      percentContainer::const_iterator percentIter = m_componentPercentage.begin();

      while (m_lithoComponents.end() != componentIter && m_componentPercentage.end() != percentIter)
      {
         fraction = (double)(*percentIter)*0.01;
         Permeability_Derivative_Normal += fraction * Derivatives[componentCount];
         Permeability_Derivative_Plane += fraction * Derivatives[componentCount] * (*componentIter)->getPermAniso();

         ++componentIter;
         ++percentIter;
         ++componentCount;
      }
   }
   else {
      compContainer::const_iterator componentIter2;
      percentContainer::const_iterator percentIter = m_componentPercentage.begin();
      percentContainer::const_iterator percentIter2;

      int I;
      int J;

      Permeability_Derivative_Normal = 0.0;
      Permeability_Derivative_Plane = 0.0;

      I = 0;

      while (m_lithoComponents.end() != componentIter) {
         percentIter2 = m_componentPercentage.begin();
         componentIter2 = m_lithoComponents.begin();
         J = 0;
         derivativeTerm = 1.0;

         while (m_lithoComponents.end() != componentIter2) {
            fraction = (double)(*percentIter2)*0.01;

            if (componentIter != componentIter2) {
               derivativeTerm = derivativeTerm * fraction * pow(Permeabilities[J], fraction);
            }

            ++componentIter2;
            ++percentIter2;
            ++J;
         }

         fraction = 0.01 * double(*percentIter);
         derivativeTerm = derivativeTerm * fraction * Derivatives[I] * pow(Permeabilities[I], fraction - 1.0);

         Permeability_Derivative_Normal = Permeability_Derivative_Normal + derivativeTerm;
         Permeability_Derivative_Plane = Permeability_Derivative_Plane + derivativeTerm * (*componentIter)->getPermAniso();
         ++componentIter;
         ++percentIter;
         ++I;
      }

   }

   Permeability_Derivative_Normal = Permeability_Derivative_Normal * MILLIDARCYTOM2;
   Permeability_Derivative_Plane = Permeability_Derivative_Plane  * MILLIDARCYTOM2;

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


double GeoPhysics::CompoundLithology::computePorosityDerivativeWRTPressure(const double ves,
   const double maxVes,
   const bool   includeChemicalCompaction,
   const double chemicalCompactionTerm) const {

   return m_porosity.getPorosityDerivative(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

}
//------------------------------------------------------------//
void GeoPhysics::CompoundLithology::mixCapillaryEntryPressureCofficients()
{
   m_capC1 = m_capC2 = 0.0;

   compContainer::iterator componentIter = m_lithoComponents.begin();

   if (m_mixmodeltype == HOMOGENEOUS){
      percentContainer::iterator percentIter = m_componentPercentage.begin();
      double pcMult;

      while (m_lithoComponents.end() != componentIter) {
         pcMult = (double)(*percentIter) / 100;

         m_capC1 += (*componentIter)->getCapC1() * pcMult;
         m_capC2 += (*componentIter)->getCapC2() * pcMult;

         ++componentIter;
         ++percentIter;
      }
   } else { // if (m_mixmodeltype == LAYERED){
#ifdef _MSC_VER
#undef max	// To make sure the next statement is using the correct max() function
#endif
	   double depoPerm,  minDepoPerm = numeric_limits<double>::max();

      while (m_lithoComponents.end() != componentIter) {
         depoPerm = (*componentIter)->getDepoPerm();
         if (depoPerm < minDepoPerm) {
            m_capC1 = (*componentIter)->getCapC1();
            m_capC2 = (*componentIter)->getCapC2();
            minDepoPerm = depoPerm;
         }
         ++componentIter;
      }
   }

   m_tenPowerCapC2 = pow(10.0, -m_capC2);
}

void GeoPhysics::CompoundLithology::mixBrooksCoreyParameters()
{
   //Note:  mixing type is not considered (homogeneous, layered)
   //Maximum percentage is considered
   //TODO: what when equal percentage (50 - 50)?

   m_LambdaPc = m_LambdaKr = 0;
   m_PcKrModel = "Brooks_Corey";

   compContainer::iterator componentIter = m_lithoComponents.begin();
   percentContainer::iterator percentIter = m_componentPercentage.begin();

   double percent = (double)(*percentIter) / 100;
   m_LambdaPc = (*componentIter)->getLambdaPc();
   m_LambdaKr = (*componentIter)->getLambdaKr();
   //get pckrmodel
   //?? is the PcKrModel different for different lithology ? 
   m_PcKrModel = (*componentIter)->getPcKrModel();

   ++componentIter;
   ++percentIter;

   while (m_lithoComponents.end() != componentIter)
   {

      // If the volume fraction of the current lithology is greater than
      // the maximum volume fraction so far then use the exponents of the 
      // current lithology.
      if (percent < ((double)(*percentIter) / 100))
      {
         m_LambdaPc = (*componentIter)->getLambdaPc();
         m_LambdaKr = (*componentIter)->getLambdaKr();
         percent = (double)(*percentIter) / 100;
      }
      //if percentage are equal, find smaller exponent
      else if (percent == ((double)(*percentIter) / 100))
      {

         if ((*componentIter)->getLambdaPc() < m_LambdaPc)
         {
            m_LambdaPc = (*componentIter)->getLambdaPc();
         }
         if ((*componentIter)->getLambdaKr() < m_LambdaKr)
         {
            m_LambdaKr = (*componentIter)->getLambdaKr();
         }

      }

      ++componentIter;
      ++percentIter;
   }
}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::capillaryPressure(const pvtFlash::PVTPhase phase,
   const double             densityBrine,
   const double             densityHc,
   const double             saturationBrine,
   const double             saturationHc,
   const double             porosity) const {

   double result = 99999.0;

#if 0
   double densityDiff = densityBrine - densityHc;
   double interfacialTension;
   double bulkDensity = porosity * densityBrine + (1.0 - porosity) * m_density;
   double cosContactAngle;
   double reducedTemperature;
   double A;
   double B;

   if (phase == pvtFlash::OIL) {
      cosContactAngle = m_cosOilContactAngle;
   }
   else {
      cosContactAngle = m_cosGasContactAngle;
   }

   interfacialTension = std::pow(A * std::pow(densityDiff, B + 1) / reducedTemperature(phase), 4);
   result = interfacialTension * cosContactAngle * m_specificSurfaceArea * bulkDensity * std::exp(-(1.0 - saturationHc) * m_geometricVariance) * (1.0 - porosity) / porosity;
#endif

   return result;
}

//------------------------------------------------------------//

double GeoPhysics::CompoundLithology::capillaryPressure(const unsigned int phaseId,
   const double& density_H2O,
   const double& density_HC,
   const double& T_K,
   const double& T_c_HC_K,
   const double& wettingSaturation,
   const double& porosity) const {

   double capP = CBMGenerics::capillarySealStrength::capPressure(phaseId, density_H2O, density_HC,
      T_K, T_c_HC_K,
      m_specificSurfaceArea,
      m_geometricVariance, wettingSaturation,
      porosity, m_density);

   return capP;
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

   double MatrixTHCondN = 0.0, MatrixTHCondP = 0.0;
   SimpleLithology * currentLitho = m_lithoComponents[0];

   if ((currentLitho->getThermalCondModel() == Interface::TABLE_MODEL ||
      currentLitho->getThermalCondModel() == Interface::CONSTANT_MODEL) && m_lithoComponents[0]->getName() != DataAccess::Interface::ALCBasalt) {
      MatrixTHCondN = thermalconductivityN(Temperature);
      MatrixTHCondP = thermalconductivityP(Temperature);

   }
   else {
      // cout << "calcBulkThermCondNPBasement for " <<  m_lithoComponents [ 0 ]->getName() << "; model = " << currentLitho->getThermalCondModel() << endl;
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

