#ifdef USEOTGC
#include "OilToGasCracker.h"
#include "rankings.h"

#include "Composition.h"
#include "migration.h"
#include "OTGC_kernel6/src/SimulatorState.h"
#include "Simulator.h"
#include "Constants.h"

#include "Immobiles.h"
#include <iostream>

namespace migration {

OilToGasCracker::OilToGasCracker (bool containSulphur) : m_theSimulator(0)
{
   int runType = ( containSulphur ? Genex6::Constants::SIMOTGC : (Genex6::Constants::SIMOTGC | Genex6::Constants::SIMOTGC5) );

   char *OTGCDIR = getenv("OTGCDIR");  

   if(OTGCDIR) {
      if(!containSulphur) {
         m_theSimulator = new Genex6::Simulator(OTGCDIR, runType);
      } else {
         // H/C = 1.8, S/C = 0.035
         m_theSimulator = new Genex6::Simulator(OTGCDIR, runType, "TypeII_GX6", 1.8, 0.035);
      }
      if(GetRank() == 0) {
         cout << "Configuration File: " << (containSulphur ? "TypeII_GX6" : "TypeII") << endl;
      } 
   }
   else
   {
      //should throw instead...
      std::string s = "!!!Warning OTGCDIR environment variable is not set. No OTGC functionality is available";
      throw s;
   }

   for(int i = 0; i < migration::NumComponents; ++i) {
      if(strcmp( ComponentNames[i], "H2S" ) == 0) {
         ComponentsUsedInOTGC[i] = true;
      } else {
         ComponentsUsedInOTGC[i] = ComponentsUsed[i];
      }
   }
}

OilToGasCracker::~OilToGasCracker (void)
{
   delete m_theSimulator;
}

void OilToGasCracker::compute (const Composition & saraIn, Immobiles &immobilesIn,
      double startTime, double endTime,
      double startPressure, double endPressure,
      double startTemperature, double endTemperature,
      Composition & saraOut, Immobiles &immobilesOut)
{
   if(m_theSimulator)
   {
      typedef std::map < std::string, double >::iterator itC;
      std::map < std::string, double >concentrationsByName;

      double totalWeight = 0.0;

      for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
      {
         if (!ComponentsUsedInOTGC[componentId]) continue;
         double weight = saraIn.getWeight ((ComponentId) componentId);

         totalWeight += weight;
         concentrationsByName[ComponentNames[componentId]] = weight;
      }

      for (unsigned int immobileId = 0; immobileId < NumImmobiles; ++immobileId)
      {
         double weight = immobilesIn.getWeight ((ImmobilesId) immobileId);

         totalWeight += weight;
         concentrationsByName[ImmobileNames[immobileId]] = weight;
      }

      //normalize weights
      for (itC it = concentrationsByName.begin (), itEnd = concentrationsByName.end (); it != itEnd; ++it)
      {
         it->second /= totalWeight;
      }

      //create the Simulator State
      OTGC6::SimulatorState theState (startTime, m_theSimulator->getSpeciesInChemicalModel (), concentrationsByName);

      //compute
      m_theSimulator->computeInterval (theState,
                                       startTemperature, endTemperature, startPressure, endPressure, startTime, endTime);

      //get normalized results
      theState.GetSpeciesStateConcentrations (&m_theSimulator->getChemicalModel(), concentrationsByName);

      itC itOutEnd = concentrationsByName.end ();

      //compute masses and update saraOut and immobilesOut
      for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
      {
         if (!ComponentsUsedInOTGC[componentId]) continue;

         itC it = concentrationsByName.find (ComponentNames[componentId]);

         double weightOut = (it == itOutEnd) ? 0.0 : (it->second * totalWeight);

         saraOut.set ((ComponentId) componentId, weightOut);
      }

      for (unsigned int immobileId = 0; immobileId < NumImmobiles; ++immobileId)
      {
         itC it = concentrationsByName.find (ImmobileNames[immobileId]);

         double weightOut = (it == itOutEnd) ? 0.0 : (it->second * totalWeight);

         immobilesOut.set ((ImmobilesId) immobileId, weightOut);
      }
   }
}


} // namespace migration
#endif
