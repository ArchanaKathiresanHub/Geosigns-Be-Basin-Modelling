#include "componentsreaderpvt.h"
#include "globalnumbers.h"

// qt includes
#include <qmessagebox.h>

#include "EosPack.h"


//
// PUBLIC METHODS
//
ComponentsReaderPVT::~ComponentsReaderPVT ()
{ 
   if ( m_errMsg ) delete m_errMsg; 
}

//
// PROTECTED METHODS
//

/// ComponentsReaderPVT::readRecord reads component masses from a record,
/// then performs PVT calculation
void ComponentsReaderPVT::readRecord (Record *rec)
{ 
   // get reference to trap group
   LineGroup& lineGroup = (plotLines()) [*m_currReservoir][m_persisIt->first];
  
   // get masses
   double masses [ComponentManager::NumberOfOutputSpecies];  

   masses[ComponentManager::asphaltene]   = getMassasphaltenes (rec);
   masses[ComponentManager::resin]        = getMassresins (rec);
   masses[ComponentManager::C15PlusAro]   = getMassC15Aro (rec);
   masses[ComponentManager::C15PlusSat]   = getMassC15Sat (rec);
   masses[ComponentManager::C6Minus14Aro] = getMassC6_14Aro (rec);
   masses[ComponentManager::C6Minus14Sat] = getMassC6_14Sat (rec);
   masses[ComponentManager::C5]           = getMassC5 (rec);
   masses[ComponentManager::C4]           = getMassC4 (rec);
   masses[ComponentManager::C3]           = getMassC3 (rec); 
   masses[ComponentManager::C2]           = getMassC2 (rec); 
   masses[ComponentManager::C1]           = getMassC1 (rec); 
   masses[ComponentManager::COx]          = getMassCOx (rec); 
   masses[ComponentManager::N2]           = getMassN2 (rec); 

   // get temperature and pressure values for reservoir or stock tank conditions
   double pressure = m_reservoirConditions ? getPressure (rec) : (ST_Pressure * PA_TO_MPA);
   double temperature = m_reservoirConditions ? getTemperature (rec) : ST_Temperature;

   // init pvt output
   double phasesDensity[ComponentManager::NumberOfPhases];
   double phaseComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies];
   
   initPvtOutput (phaseComponents);  // zero's phaseComponents

   // validate pvt input and call pvt pack
   int error = validatePvtInput (masses, temperature, pressure);
   if  (error == NO_ERROR )
   {
      // input data okay, so call PVT
      m_currentComputePvtType->computePVT( phaseComponents, 
                                           phasesDensity,
                                           masses,
                                           temperature,
                                           pressure);
   }
   else if  (error == INVALID_INPUT )
   {
      showPvtErrMsg (temperature, pressure);
   }
   
   // file line graph
   m_currentWritePvtToGraphType->writeToGraph (lineGroup, m_transIt->first, phaseComponents);
}

/// ComponentsReaderPVT::validatePvtInput checks that all PVT input data is valid and returns T/F
int ComponentsReaderPVT::validatePvtInput (double *masses, double temperature, double pressure)
{
   // check mass values
   double massSum = 0;
   for ( int i=0; i < ComponentManager::NumberOfOutputSpecies; ++i )
   {
      if ( masses[i] == Null_Values::Cauldron_Null_Float )
      {
         return NULL_MASS;
      }
      massSum += masses[i];
   } 
   
   if ( massSum < Cauldron_Values::MinimumWeight )
   {
      return NULL_MASS;
   }
   
   // check pressure and temperature
   if ( pressure < 0.1 || temperature == Null_Values::Cauldron_Null_Float )
   {
      return INVALID_INPUT;
   }
   
   return NO_ERROR;
}

/// ComponentsReaderPVT::initPvtOutput takes care of initialising PVT output
void ComponentsReaderPVT::initPvtOutput (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   int i,j;
   
   for ( i=0; i < ComponentManager::NumberOfPhases; ++i ) 
   {
      for ( j=0; j < ComponentManager::NumberOfOutputSpecies; ++j ) 
      {
         phasesComponents[i][j] = 0.0;
      }
   }
}

void ComponentsReaderPVT::showPvtErrMsg (double temperature, double pressure)
{
   initErrMsg ("Trap Tracking PVT Error");

   char msg[100];
   sprintf(msg, "Invalid PVT Input: Pressure  = %f, Temperature = %f", pressure, temperature);
   m_errMsg->setText (msg);
   m_errMsg->show();
}

void ComponentsReaderPVT::initErrMsg (const char *caption)
{
   if ( ! m_errMsg )
   {
      m_errMsg = new QMessageBox (); 
      m_errMsg->setCaption (caption);
   }
}

//
// COMPUTE PVT METHODS
//
/// ComponentsReaderPVT::ComputePvtMass::computePVT computes a straight forward PVT
/// to get Oil and Gas phase separation
void ComponentsReaderPVT::ComputePvtMass::computePVT(
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], 
            double *phasesDensity,
            double *masses,  
            double temperature, 
            double pressure)
{
   double phasesViscosity [ComponentManager::NumberOfPhases]; // dummy, not used

   pvtFlash::EosPack::getInstance().computeWithLumping (temperature + 273.15,// input in K 
                                                        pressure * 1.00e+06, // input in Pa
                                                        masses,              // input in kg
                                                        phasesComponents,    // output in kg
                                                        phasesDensity,       // output in kg/m3
                                                        phasesViscosity );   // output in Pa
}

/// ComponentsReaderPVT::ComputePvtVolume::computePVT calls
/// ComponentsReaderPVT::ComputePvtMass::computePVT to computes mass PVT
/// and then multiplys mass by density to get volume
void ComponentsReaderPVT::ComputePvtVolume::computePVT(
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], 
            double *phasesDensity,
            double *masses,  
            double temperature, 
            double pressure)
{
   // get pvt masses using base class
   ComputePvtMass::computePVT (phasesComponents, phasesDensity, masses, temperature, pressure);
   
   // work out volumes
   int i, j;
   for ( i=0; i < ComponentManager::NumberOfPhases; ++i ) 
   {
      for ( j=0; j < ComponentManager::NumberOfOutputSpecies; ++j ) 
      {
         phasesComponents [i][j] /= phasesDensity[i];
      }
   }
}

//
// WRITE TO GRAPH METHODS
//
/// ComponentsReaderPVT::WritePvtTotalToGraph::writeToGraph writes added Oil and Gas
/// phase masses to graph (each component has a line. Age is the X coordinate and mass is the Y
void ComponentsReaderPVT::WritePvtTotalToGraph::writeToGraph (
            LineGroup& lineGroup, 
            double transIdAge,
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
    double age = transIdAge - (2*transIdAge);
    
    ComponentManager &tcm = ComponentManager::getInstance();

    lineGroup[tcm.GetSpeciesName(ComponentManager::C1)].add  (age, 
			phasesComponents [ComponentManager::Gas][ComponentManager::C1] + 
			phasesComponents [ComponentManager::Oil][ComponentManager::C1]);

    lineGroup[tcm.GetSpeciesName(ComponentManager::C2)].add  (age, 
                            phasesComponents [ComponentManager::Gas][ComponentManager::C2] + 
                            phasesComponents [ComponentManager::Oil][ComponentManager::C2]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::C3)].add  (age, 
                            phasesComponents [ComponentManager::Gas][ComponentManager::C3] + 
                            phasesComponents [ComponentManager::Oil][ComponentManager::C3]);
     
    lineGroup[tcm.GetSpeciesName(ComponentManager::C4)].add  (age, 
                            phasesComponents [ComponentManager::Gas][ComponentManager::C4] + 
                            phasesComponents [ComponentManager::Oil][ComponentManager::C4]);
     
    lineGroup[tcm.GetSpeciesName(ComponentManager::C5)].add  (age, 
                            phasesComponents [ComponentManager::Gas][ComponentManager::C5] + 
                            phasesComponents [ComponentManager::Oil][ComponentManager::C5]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::N2)].add  (age, 
                            phasesComponents [ComponentManager::Gas][ComponentManager::N2] + 
                            phasesComponents [ComponentManager::Oil][ComponentManager::N2]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::COx)].add  (age, 
                             phasesComponents [ComponentManager::Gas][ComponentManager::COx] + 
                             phasesComponents [ComponentManager::Oil][ComponentManager::COx]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::asphaltene)].add  (age, 
                               phasesComponents [ComponentManager::Gas][ComponentManager::asphaltene] + 
                               phasesComponents [ComponentManager::Oil][ComponentManager::asphaltene]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::resin)].add  (age, 
                               phasesComponents [ComponentManager::Gas][ComponentManager::resin] + 
                               phasesComponents [ComponentManager::Oil][ComponentManager::resin]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusAro)].add  (age, 
                               phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusAro] + 
                               phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusAro]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusSat)].add  (age, 
                               phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusSat] + 
                               phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusSat]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Aro)].add  (age, 
                               phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Aro] + 
                               phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Aro]);
    
    lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Sat)].add  (age, 
                               phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Sat] + 
                               phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Sat]);
    
    lineGroup[TotalGas].add (age, getTotalGasVolume (phasesComponents));
    lineGroup[WetGas].add (age, getWetGasVolume (phasesComponents));
    lineGroup[TotalOil].add (age, getTotalOilVolume (phasesComponents));
}

/// ComponentsReaderPVT::WritePvtTotalToGraph::getWetGasVolume computes
/// total volumes adding oil and gas phase volumes together (for Wet Gas components only)
double ComponentsReaderPVT::WritePvtTotalToGraph::getWetGasVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return 
         phasesComponents [ComponentManager::Gas][ComponentManager::C2] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C2] +
         phasesComponents [ComponentManager::Gas][ComponentManager::C3] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C3] +     
         phasesComponents [ComponentManager::Gas][ComponentManager::C4] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C4] +     
         phasesComponents [ComponentManager::Gas][ComponentManager::C5] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C5];         
}

/// ComponentsReaderPVT::WritePvtTotalToGraph::getTotalGasVolume computes 
/// total volume adding oil and gas phase volumes together (for all Gas components)
double ComponentsReaderPVT::WritePvtTotalToGraph::getTotalGasVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return
         phasesComponents [ComponentManager::Gas][ComponentManager::C1] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C1] +
         getWetGasVolume (phasesComponents);
}

/// ComponentsReaderPVT::WritePvtTotalToGraph::getTotalOilVolume 
/// addes gas and oil phase volumes for Oil components only
double ComponentsReaderPVT::WritePvtTotalToGraph::getTotalOilVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return
         phasesComponents [ComponentManager::Gas][ComponentManager::asphaltene] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::asphaltene] +
         phasesComponents [ComponentManager::Gas][ComponentManager::resin] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::resin] +
         phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusAro] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusAro] +
         phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusSat] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusSat] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Aro] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Aro] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Sat] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Sat];
}

/// ComponentsReaderPVT::WritePvtGasToGraph::writeToGraph writes gas 
/// and oil masses for the gas phase only
void ComponentsReaderPVT::WritePvtGasToGraph::writeToGraph (
            LineGroup& lineGroup, 
            double transIdAge,
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   double age = transIdAge - (2*transIdAge);
   ComponentManager &tcm = ComponentManager::getInstance();
   
   lineGroup[tcm.GetSpeciesName(ComponentManager::C1)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C1]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C2)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C2]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C3)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C3]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C4)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C4]);     
   lineGroup[tcm.GetSpeciesName(ComponentManager::C5)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C5]);    
   lineGroup[tcm.GetSpeciesName(ComponentManager::N2)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::N2]);    
   lineGroup[tcm.GetSpeciesName(ComponentManager::COx)].add (age,
                    phasesComponents [ComponentManager::Gas][ComponentManager::COx]);    
   lineGroup[tcm.GetSpeciesName(ComponentManager::asphaltene)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::asphaltene]);    
   lineGroup[tcm.GetSpeciesName(ComponentManager::resin)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::resin]);    
   lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusAro)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusAro]);    
   lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusSat)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusSat]);    
   lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Aro)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Aro]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Sat)].add (age, 
                    phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Sat]);

   lineGroup[TotalGas].add (age, getTotalGasVolume (phasesComponents));
   lineGroup[WetGas].add (age, getWetGasVolume (phasesComponents));
   lineGroup[TotalOil].add (age, getTotalOilVolume (phasesComponents));
}

/// ComponentsReaderPVT::WritePvtGasToGraph::getWetGasVolume computes wet gas
/// volumes for the wet gas phase only
double ComponentsReaderPVT::WritePvtGasToGraph::getWetGasVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return 
         phasesComponents [ComponentManager::Gas][ComponentManager::C2] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C3] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C4] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C5];
}

/// ComponentsReaderPVT::WritePvtGasToGraph::getWetGasVolume computes total gas
/// phase for the gas components only
double ComponentsReaderPVT::WritePvtGasToGraph::getTotalGasVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return
         phasesComponents [ComponentManager::Gas][ComponentManager::C1] + 
         getWetGasVolume (phasesComponents);
}

/// ComponentsReaderPVT::WritePvtGasToGraph::getTotalOilVolume computes 
/// total gas phase for oil components only
double ComponentsReaderPVT::WritePvtGasToGraph::getTotalOilVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return
         phasesComponents [ComponentManager::Gas][ComponentManager::asphaltene] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::resin] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusAro] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C15PlusSat] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Aro] + 
         phasesComponents [ComponentManager::Gas][ComponentManager::C6Minus14Sat]; 
}

/// ComponentsReaderPVT::WritePvtOilToGraph::writeToGraph writes oil and gass phase 
/// components to the Graph, for the oil phase only
void ComponentsReaderPVT::WritePvtOilToGraph::writeToGraph (
            LineGroup& lineGroup, 
            double transIdAge,
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   double age = transIdAge - (2*transIdAge);
   ComponentManager &tcm = ComponentManager::getInstance();
   
   lineGroup[tcm.GetSpeciesName(ComponentManager::C1)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C1]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C2)].add (age,
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C2]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C3)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C3]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C4)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C4]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C5)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C5]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::N2)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::N2]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::COx)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::COx]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::asphaltene)].add (age,
                                 phasesComponents [ComponentManager::Oil][ComponentManager::asphaltene]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::resin)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::resin]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusAro)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusAro]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusSat)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusSat]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Aro)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Aro]);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Sat)].add (age, 
                                 phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Sat]);

   lineGroup[TotalGas].add (age, getTotalGasVolume (phasesComponents));
   lineGroup[WetGas].add (age, getWetGasVolume (phasesComponents));
   lineGroup[TotalOil].add (age, getTotalOilVolume (phasesComponents));
}

/// ComponentsReaderPVT::WritePvtOilToGraph::getWetGasVolume computes wet
/// gas components for oil phase only
double ComponentsReaderPVT::WritePvtOilToGraph::getWetGasVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return 
         phasesComponents [ComponentManager::Oil][ComponentManager::C2] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C3] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C4] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C5];
}

/// ComponentsReaderPVT::WritePvtOilToGraph::getTotalGasVolume computes total
/// gas volume for oil phase only
double ComponentsReaderPVT::WritePvtOilToGraph::getTotalGasVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])     
{
   return
         phasesComponents [ComponentManager::Oil][ComponentManager::C1] + 
         getWetGasVolume (phasesComponents);
}

/// ComponentsReaderPVT::WritePvtOilToGraph::getTotalOilVolume computes 
/// oil phase volume for oil components only
double ComponentsReaderPVT::WritePvtOilToGraph::getTotalOilVolume (
      double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies])
{
   return
         phasesComponents [ComponentManager::Oil][ComponentManager::asphaltene] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::resin] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusAro] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C15PlusSat] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Aro] + 
         phasesComponents [ComponentManager::Oil][ComponentManager::C6Minus14Sat];
}
