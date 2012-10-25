#ifndef _GENEX5_OTGC_NODE_ADSORPTION_HISTORY_H_
#define _GENEX5_OTGC_NODE_ADSORPTION_HISTORY_H_

#include <string>
#include <vector>

#include "NodeAdsorptionHistory.h"
#include "SourceRockNode.h"

#include "PVTCalculator.h"

class OTGCNodeAdsorptionHistory : public Genex5::NodeAdsorptionHistory {

   struct HistoryItem {

      HistoryItem ();

      double m_time;
      double m_thickness;
      double m_temperature;
      double m_pressure;
      double m_porosity;
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

      double m_subSurfaceLiquidDensity;
      double m_subSurfaceVapourDensity;

      double m_irreducibleWaterSat;
      double m_hcSaturation;

      Genex5::PVTComponents m_expelledMasses;
      Genex5::PVTComponents m_retainedMasses;
      Genex5::PVTComponents m_genex5ExpelledMasses;
   };

   typedef std::vector<HistoryItem*> HistoryItemList;

public :

   ~OTGCNodeAdsorptionHistory ();

   /// Collect adsorption results.
   void collect ( Genex5::SourceRockNode* node );

   /// Write collected results to the stream.
   void write ( std::ostream& str );

private :

   HistoryItemList m_history;

};


Genex5::NodeAdsorptionHistory* allocateOTGCNodeAdsorptionHistory ();

#endif // _GENEX5_OTGC_NODE_ADSORPTION_HISTORY_H_
