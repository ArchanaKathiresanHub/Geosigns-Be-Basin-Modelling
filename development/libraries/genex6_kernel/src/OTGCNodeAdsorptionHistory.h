#ifndef _GENEX6_KERNEL__OTGC_NODE_ADSORPTION_HISTORY_H_
#define _GENEX6_KERNEL__OTGC_NODE_ADSORPTION_HISTORY_H_

#include <string>
#include <vector>

#include "ProjectHandle.h"

#include "NodeAdsorptionHistory.h"
#include "SourceRockNode.h"

#include "PVTCalculator.h"

namespace Genex6 {

   /// \brief Contains the components that are required to be saved at a time-step for a particular node.
   class OTGCNodeAdsorptionHistory : public Genex6::NodeAdsorptionHistory {

      struct HistoryItem {

         HistoryItem ();

         double m_time;
         double m_thickness;
         double m_temperature;
         double m_pressure;
         double m_porosity;
         double m_effectivePorosity;
         double m_permeability;

         double m_toc;
         double m_vre;

         double m_VLSRTemperature;
         double m_VLReferenceTemperature;

         double m_c1adsorped;
         double m_adsorpedFraction;
         double m_c1desorped;
         double m_adsorptionCapacity;
         double m_bitumen;

         double m_gasRetainedSTSCF;
         double m_oilRetainedSTBarrels;
         double m_gasRetainedSTBCF;
         double m_oilRetainedSTMBarrels;

         double m_hcLiquidSaturation;
         double m_hcVapourSaturation;
         double m_waterSaturation;

         double m_subSurfaceLiquidDensity;
         double m_subSurfaceVapourDensity;

         double m_irreducibleWaterSat;
         double m_hcSaturation;

         Genex6::PVTComponents m_expelledMasses;
         Genex6::PVTComponents m_retainedMasses;
         Genex6::PVTComponents m_genex5ExpelledMasses;

         double m_retainedOilApiSr;
         double m_retainedCondensateApiSr;
         double m_retainedGorSr;
         double m_retainedCgrSr;
         double m_overChargeFactor;

         double m_hOverC;
         double m_oOverC;

         double m_h2sRisk;

      };

      typedef std::vector<HistoryItem*> HistoryItemList;

   public :

      OTGCNodeAdsorptionHistory ( const SpeciesManager& speciesManager,
                                  DataAccess::Interface::ProjectHandle& projectHandle );

      ~OTGCNodeAdsorptionHistory ();

      /// Collect adsorption results.
      void collect ( Genex6::SourceRockNode* node );

      /// Write collected results to the stream.
      void write ( std::ostream& str );

   private :

      HistoryItemList m_history;

   };


   /// \brief Allocator of a history object for the shale-gas simulator.
   Genex6::NodeAdsorptionHistory* allocateOTGCNodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                                                                      DataAccess::Interface::ProjectHandle& projectHandle );

}

#endif // _GENEX6_KERNEL__OTGC_NODE_ADSORPTION_HISTORY_H_
