#ifndef FISSIONTRACK_CALCULATOR_H_
#define FISSIONTRACK_CALCULATOR_H_

#include <string>
#include <map>
#include <vector>
#include "NodeInput.h"

class AppCtx;

namespace Calibration
{
   namespace FissionTracks
   {
	   class FtAnalysis;
   }
}
namespace database
{
   class Table;
}

class LayerProps;

class FissionTrackCalculator
{
public:
   FissionTrackCalculator( AppCtx* Application_Context );
   ~FissionTrackCalculator(void);

   bool compute();
   void collectSampleTrackingData(const double time);

   bool writeFissionTrackResultsToDatabaseTables(database::Table &FtSampleIoTbl, 
                                                 database::Table &FtGrainIoTbl,
                                                 database::Table &FtPredLengthCountsHistIoTbl,
                                                 database::Table &FtPredLengthCountsHistDataIoTbl,
                                                 database::Table &FtClWeightPercBinsIoTbl
                                                 )const;
   void clearSampleInputHistory(void);
 
private:

   struct LayerTrackingData
   {
     LayerTrackingData(LayerProps_Ptr in_layer):
       m_layer(in_layer){}
     ~LayerTrackingData(){}

     LayerProps_Ptr m_layer;
     std::vector< double > m_referenceTimes;
     std::vector< std::vector<double> > m_depthAtLayerNodes;            //indexing: [timestep][nodeId(top->bottom)]
     std::vector< std::vector<double> > m_temperatureAtLayerNodes;         //indexing: [timestep][nodeId(top->bottom)]
     std::vector< std::vector<double> > m_VES_atLayerNodes;                 //indexing: [timestep][nodeId(top->bottom)]
     std::vector< std::vector<double> > m_maxVES_atLayerNodes;              //indexing: [timestep][nodeId(top->bottom)]
     std::vector< std::vector<double> > m_chemicalCompactionAtLayerNodes;  //indexing: [timestep][nodeId(top->bottom)]
                                                                       //we do not know upper/lower node before end of geometric loop
                                                                       //so we have to store all nodal temperatures in layer for each timestep
   };

   std::map<std::string, LayerTrackingData*> m_layerTrackingData;


   struct SampleTrackingData
   {
     SampleTrackingData(double in_depth,
			std::string in_formationName,
			int in_upperNodeIndex,
			int in_lowerNodeIndex,
			double in_upperNodeDepth,
			double in_lowerNodeDepth,
			LayerProps_Ptr in_layer,
			LayerTrackingData* in_layerTrackingData,
			double in_solidThicknessLowerNodeToSample):
       m_depth(in_depth),
       m_formationName(in_formationName),
       m_upperNodeIndex(in_upperNodeIndex),
       m_lowerNodeIndex(in_lowerNodeIndex),
       m_upperNodeDepth(in_upperNodeDepth),
       m_lowerNodeDepth(in_lowerNodeDepth),
       m_layer(in_layer),
       m_layerTrackingData(in_layerTrackingData),
       m_solidThicknessLowerNodeToSample(in_solidThicknessLowerNodeToSample){}

     ~SampleTrackingData(){}
      
     double m_depth;
     std::string m_formationName;
     int m_upperNodeIndex;
     int m_lowerNodeIndex;
     double m_upperNodeDepth;
     double m_lowerNodeDepth;
     LayerProps_Ptr m_layer;
     LayerTrackingData* m_layerTrackingData;

     double m_solidThicknessLowerNodeToSample;
     std::vector<Calibration::NodeInput> m_inputNodes; //input to fission track kernel
     
   };

   std::map<std::string, SampleTrackingData*> m_sampleTrackingData;



   void initializeSampleTrackingData();
   void clearAll();
   void determineUpperAndLowerNodeForSample();
   void interpolateTemperatureToSampleLocationNonMobileLayer(SampleTrackingData* sampleData);
   void interpolateTemperatureToSampleLocationMobileLayer(SampleTrackingData* sampleData);
   void computeSolidThicknessSampleToLowerNode(SampleTrackingData* sampleData);
   double computeRealThicknessLowerNodeToSample( const double solidThicknessLowerNodeToSample, 
						 const double VES_bottom, 
						 const double VES_top,
						 const double maxVES_bottom,
						 const double maxVES_top,
						 const bool   includeChemicalCompaction,
						 const double chemicalCompactionBottom, 
						 const double chemicalCompactionTop, 
						 const double realThicknessElement,
						 const CompoundLithology* lithology  
						 );
   int numberOfStepsForCompactionEquation ( const int optimisationLevel ) const ;


   AppCtx*     m_Basin_Model;
   Calibration::FissionTracks::FtAnalysis *m_theFtAnalysis;

   std::multimap<std::string, std::string > m_SamplesByLayerName;

};

#endif 
