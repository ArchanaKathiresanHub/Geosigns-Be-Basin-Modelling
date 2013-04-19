/// Class ComponentsReaderPVT inherits from ComponentsReader
/// It also reads component masses but then performs a PVT calculation
/// It handles different combinations of PVT calculation using functors
/// which are set through the public methods

#ifndef __componentsreaderpvt__
#define __componentsreaderpvt__

#include "componentsreader.h"

// library references
#include "ComponentManager.h"
using namespace CBMGenerics;

#include "globalnumbers.h"
using namespace Cauldron_Values;

#include "globalstrings.h"
using namespace Graph_Properties;

class QMessageBox;

class ComponentsReaderPVT : public ComponentsReader
{
public:
   ComponentsReaderPVT () 
      : ComponentsReader (), 
      m_currentComputePvtType (&m_computePvtMass),
      m_currentWritePvtToGraphType (&m_writePvtTotal),
      m_reservoirConditions (true),
      m_errMsg (0)
   {}
   virtual ~ComponentsReaderPVT (); 
   
   void setReservoirConditions () { m_reservoirConditions = true; }
   void setStockTankConditions () { m_reservoirConditions = false; }
   
   void setOilPhase () { m_currentWritePvtToGraphType = &m_writePvtOil; }
   void setGasPhase () { m_currentWritePvtToGraphType = &m_writePvtGas; }
   void setTotalPhase () { m_currentWritePvtToGraphType = &m_writePvtTotal; }
   
   void setComputePvtMasses () { m_currentComputePvtType = &m_computePvtMass; }
   void setComputePvtVolumes () { m_currentComputePvtType = &m_computePvtVolume; }
   
protected:
   virtual void readRecord (Record *rec);

private:
   int validatePvtInput (double *masses, double temperature, double pressure);
   void initPvtOutput (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
   void showPvtErrMsg (double temperature, double pressure);
   void initErrMsg (const char *caption);
   
   // nested classes for combining user-requested behaviour
   class ComputePvtMass
   {
   public:
      virtual void computePVT(
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], 
            double *phasesDensity,
            double *masses,  
            double temperature, 
            double pressure); 
   };
   
   class ComputePvtVolume : public ComputePvtMass
   {
   public:
      virtual void computePVT (
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], 
            double *phasesDensity,
            double *masses,  
            double temperature, 
            double pressure);  
   };
   
   // type of pvt computation requested
   ComputePvtMass *m_currentComputePvtType;
   ComputePvtMass m_computePvtMass;
   ComputePvtVolume m_computePvtVolume;
   
   // pvt write to graph line behaviour
   class WritePvtTotalToGraph
   {
   public:
      virtual void writeToGraph (
            LineGroup& lineGroup, double transIdAge,
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
   protected:
      double getWetGasVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
      double getTotalGasVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
      double getTotalOilVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
   };
   
   class WritePvtGasToGraph : public WritePvtTotalToGraph
   {
   public:
      virtual void writeToGraph (
            LineGroup& lineGroup, double transIdAge,
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
   protected:
      double getWetGasVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
      double getTotalGasVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
      double getTotalOilVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
   };
   
   class WritePvtOilToGraph : public WritePvtTotalToGraph
   {
   public:
      virtual void writeToGraph (
            LineGroup& lineGroup, double transIdAge,
            double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
   protected:
      double getWetGasVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
      double getTotalGasVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
      double getTotalOilVolume (double phasesComponents [ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]);
   };
   
   // write pvt type requested
   WritePvtTotalToGraph *m_currentWritePvtToGraphType;
   WritePvtTotalToGraph m_writePvtTotal;
   WritePvtGasToGraph m_writePvtGas;
   WritePvtOilToGraph m_writePvtOil;
      
   bool m_reservoirConditions;
   enum ERROR_TYPE { NULL_MASS, INVALID_INPUT, NO_ERROR };
   QMessageBox *m_errMsg;
};

#endif
