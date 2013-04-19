#include   "thermalindicator.h"
#include   "globaldefs.h"
#include   "petscvec.h"
#include   "PetscVectors.h"
#include   "layer_iterators.h"
#include   "property_manager.h"
#include "propinterface.h"
#include "CalibrationNode.h"
#include "NodeInput.h"

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include "FissionTrackCalculator.h"

#include "ftparameters.h"
#include "ftgrain.h"
#include "histogram.h"
#include "ftsampleprediction.h"
#include "ftsampleobservation.h"
#include "ftanalysis.h"

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include "Output.h"

using namespace Basin_Modelling;

#include <iostream>

using std::endl;
using std::cout;
using Calibration::FissionTracks::FtAnalysis;

FissionTrackCalculator::FissionTrackCalculator( AppCtx* Application_Context ):
m_Basin_Model(Application_Context),m_theFtAnalysis(0)
{
   if(m_Basin_Model->isModellingMode1D())
   {
      //construct the FtAnalysis object
      m_theFtAnalysis = new FtAnalysis;

      //Populate m_theFtAnalysis with project file input
      database::Table * FtSampleIoTbl = 0;
      FtSampleIoTbl = m_Basin_Model->database->getTable ("FtSampleIoTbl");
      assert(FtSampleIoTbl);

      for (database::Table::iterator tblIter = FtSampleIoTbl->begin (), tblIterEnd = FtSampleIoTbl->end ();  tblIter != tblIterEnd; ++tblIter)
      {
         database::Record * FtSampleIoTblRecord = * tblIter;
         assert(FtSampleIoTblRecord);
         const string & sampleId = database::getFtSampleId(FtSampleIoTblRecord);
         const double & zeta = database::getFtZeta(FtSampleIoTblRecord);
         const double & UstglTrackDensity = database::getFtUstglTrackDensity(FtSampleIoTblRecord);

         m_theFtAnalysis->addSample(sampleId, zeta, UstglTrackDensity);

         const double & depthIndex = database::getDepthIndex(FtSampleIoTblRecord);

         m_sampleTrackingData.insert(std::make_pair(sampleId, new SampleTrackingData(depthIndex, "", 0, 0, 0.0, 0.0, 0, 0, 0.0 ) ) );
      }

      database::Table * FtGrainIoTbl = 0;
      FtGrainIoTbl = m_Basin_Model->database->getTable ("FtGrainIoTbl");
      assert(FtGrainIoTbl);

      for (database::Table::iterator tblIter = FtGrainIoTbl->begin (), tblIterEnd = FtGrainIoTbl->end ();  tblIter != tblIterEnd; ++tblIter)
      {
         database::Record * FtGrainIoTblRecord = * tblIter;
         assert(FtGrainIoTblRecord);
         const string & sampleId       = database::getFtSampleId(FtGrainIoTblRecord);
         const int & grainId           = database::getFtGrainId(FtGrainIoTblRecord);
         const int & nSpontTracks      = database::getFtSpontTrackNo(FtGrainIoTblRecord);
         const int & nInducedTracks    = database::getFtInducedTrackNo(FtGrainIoTblRecord);
         const double & ClWeightPerc = database::getFtClWeightPerc(FtGrainIoTblRecord);

         m_theFtAnalysis->addGrain(sampleId, grainId, nSpontTracks, nInducedTracks, ClWeightPerc);		
      }

      database::Table * FtLengthIoTbl = 0;
      FtLengthIoTbl = m_Basin_Model->database->getTable ("FtLengthIoTbl");
      assert(FtLengthIoTbl);

      for (database::Table::iterator tblIter = FtLengthIoTbl->begin (), tblIterEnd = FtLengthIoTbl->end ();  tblIter != tblIterEnd; ++tblIter)
      {
         database::Record * FtLengthIoTblRecord = * tblIter;
         assert(FtLengthIoTblRecord);
         const string & sampleId      = database::getFtSampleId(FtLengthIoTblRecord);
         const int & grainId          = database::getFtGrainId(FtLengthIoTblRecord);
         const double & length        = database::getFtLength(FtLengthIoTblRecord);

         m_theFtAnalysis->addTrackLength(sampleId, grainId, length);	
      }
      
      m_theFtAnalysis->initialize();

      //cout<<"initializeSampleTrackingData(); "<<endl;
      initializeSampleTrackingData();

      //cout<<"End of construction "<<endl;
   }

};
void FissionTrackCalculator::clearSampleInputHistory(void)
{
   typedef std::map<std::string, SampleTrackingData*>::iterator SampleIt;

   for(SampleIt it = m_sampleTrackingData.begin(), itEnd = m_sampleTrackingData.end(); it != itEnd; ++it)
   {
      it->second->m_inputNodes.clear();
   }

   typedef std::map<std::string, LayerTrackingData*>::iterator LayerIt;

   for(LayerIt it = m_layerTrackingData.begin(), itEnd = m_layerTrackingData.end(); it != itEnd; ++it)
   {
      it->second->m_referenceTimes.clear();
      it->second->m_depthAtLayerNodes.clear(); 
      it->second-> m_temperatureAtLayerNodes.clear(); 
      it->second->m_VES_atLayerNodes.clear();
      it->second->m_maxVES_atLayerNodes.clear();
      it->second->m_chemicalCompactionAtLayerNodes.clear(); 
   }

}
void FissionTrackCalculator::clearAll()
{
   delete m_theFtAnalysis;

   typedef std::map<std::string, SampleTrackingData*>::iterator SampleIt;
   typedef std::map<std::string, LayerTrackingData* >::iterator LayerIt;

   for(SampleIt it = m_sampleTrackingData.begin(), itEnd = m_sampleTrackingData.end(); it != itEnd; ++it)
   {
      delete (it->second);
   }
   for(LayerIt it = m_layerTrackingData.begin(), itEnd = m_layerTrackingData.end(); it != itEnd; ++it)
   {
      delete (it->second);
   }
   m_sampleTrackingData.clear();
   m_layerTrackingData.clear();
   m_SamplesByLayerName.clear();
};
FissionTrackCalculator::~FissionTrackCalculator()
{
   if(m_Basin_Model->isModellingMode1D())
   {
	   clearAll();
   }
};
void FissionTrackCalculator::initializeSampleTrackingData()
{
   //determine layer in which sample is located 
   //(before geometric loop the corresponding element itself  cannot be determined in advance
   //since element heights (real thicknesses) change from one geometric iteration to another)
   double seaBottomDepth = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( 0, 0, 0.0 );
   double upperSurfaceDepth = seaBottomDepth;
   double lowerSurfaceDepth = upperSurfaceDepth;

   for(size_t i = 0; i < m_Basin_Model -> layers.size() - 2; ++i)
   {
      for(int j = m_Basin_Model -> layers[i]->getMaximumNumberOfElements () -  1; j >= 0; --j)
      {
         if ( m_Basin_Model -> layers[i] -> getSolidThickness ( 0, 0, j, 0.0) > 0.0 )
         {
            lowerSurfaceDepth += m_Basin_Model -> layers[i]->getRealThickness( 0, 0, j, 0.0 ); //real thickness available at present day
         }
      }

      typedef std::map<std::string, SampleTrackingData*>::iterator SampleIt;

      for(SampleIt it = m_sampleTrackingData.begin(), itEnd = m_sampleTrackingData.end(); it != itEnd; ++it)
      {
         SampleTrackingData *currentSampleTrackData = it->second;
         if (currentSampleTrackData->m_formationName ==  "" ) //is sample not related to a layer, yet?
         {
            if( (lowerSurfaceDepth >= currentSampleTrackData->m_depth) && (upperSurfaceDepth <= currentSampleTrackData->m_depth) )
            {
               currentSampleTrackData->m_formationName     = m_Basin_Model->layers[i]->layername;
               currentSampleTrackData->m_layer             = m_Basin_Model->layers[i]; 
               m_SamplesByLayerName.insert(std::make_pair(m_Basin_Model -> layers[i]->layername, it->first));

               std::map<std::string,LayerTrackingData*>::iterator layerTrackIt =  m_layerTrackingData.find( currentSampleTrackData->m_formationName );
               if ( layerTrackIt == m_layerTrackingData.end() )
               {
                  LayerTrackingData* layerTrackingData = new LayerTrackingData(m_Basin_Model->layers[i]); 
                  currentSampleTrackData->m_layerTrackingData = layerTrackingData;
                  m_layerTrackingData[ currentSampleTrackData->m_formationName ] = layerTrackingData; 
               }
               else
               {
                  currentSampleTrackData->m_layerTrackingData = layerTrackIt->second;
               }
            }                       
         }
      }
      upperSurfaceDepth = lowerSurfaceDepth; 
   }
}

void FissionTrackCalculator::determineUpperAndLowerNodeForSample()
{
   //loop over all samples
     typedef std::map<std::string, SampleTrackingData*>::iterator SampleIt;
     for (SampleIt sampleIt = m_sampleTrackingData.begin(); sampleIt!=m_sampleTrackingData.end(); ++sampleIt )
     {
        SampleTrackingData* sampleData = sampleIt->second;

        size_t nTime  = sampleData -> m_layerTrackingData -> m_referenceTimes.size();
        size_t nNodes = sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[nTime-1].size();

        //is sample depth deeper than depth of first (deepest) node in layer (geometric loop inaccuracy)? Then the first is the sample element
        if ( sampleData->m_depth > sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[nTime-1][0])
        {
           sampleData -> m_lowerNodeIndex = 0;
           sampleData -> m_upperNodeIndex = 1;
        }
        //is sample depth above (smaller than) depth of last (highest) node in layer (geometric loop inaccuracy)? Then the last is the sample element
        else if ( sampleData->m_depth < sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[nTime-1][nNodes-1])
        {
           sampleData ->m_lowerNodeIndex = nNodes - 2;
           sampleData ->m_upperNodeIndex = nNodes - 1;
        }
        else
        {
           //loop over all elements in layer of this sample (from top to bottom)
           for (size_t iNode = nNodes - 1; iNode >= 1 ; --iNode )
           {
              //is sample-depth within element
              if (    ( sampleData->m_depth >= sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[nTime-1][iNode]   )
                   && ( sampleData->m_depth <= sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[nTime-1][iNode-1] ) )
              {
                 sampleData ->m_lowerNodeIndex = iNode - 1;
                 sampleData ->m_upperNodeIndex = iNode;
                 break;
              }
           }
        }
        
        sampleData -> m_lowerNodeDepth = sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[nTime-1][sampleData->m_lowerNodeIndex];
        sampleData -> m_upperNodeDepth = sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[nTime-1][sampleData->m_upperNodeIndex];
        
     }

}


void FissionTrackCalculator::collectSampleTrackingData(const double time)
{
   Layer_Iterator Layers;
   Layers.Initialise_Iterator ( m_Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );

   while ( ! Layers.Iteration_Is_Done () ) 
   {
      LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

      if (m_layerTrackingData.find( Current_Layer->layername ) != m_layerTrackingData.end() )
      {

         LayerTrackingData* layerData = m_layerTrackingData[ Current_Layer->layername ];

         //if a mobile layer has zero solid thickness (i.e. it's first element), than erase the whole layer history so far 
         if (Current_Layer->isMobile() && Current_Layer->getSolidThickness (0, 0, 0, time) <= 0.0 )
//          if (Current_Layer->isMobile() && Current_Layer->FCT(0, 0, 0).F(time) <= 0.0 )
         {
            layerData->m_referenceTimes.clear();
            layerData->m_depthAtLayerNodes.clear(); 
            layerData->m_temperatureAtLayerNodes.clear(); 
            layerData->m_VES_atLayerNodes.clear();
            layerData->m_maxVES_atLayerNodes.clear();
            layerData->m_chemicalCompactionAtLayerNodes.clear(); 
            Layers++;
            continue;
         }
         
         //save tracking data: time and all nodal temperatures of layer 

         layerData -> m_referenceTimes.push_back(time);

         list<double>::reverse_iterator iNodeProp;
         list<double> layerPropsOnNodeColumn ;

         //get depths
         Vec propVec = Current_Layer->Current_Properties( Basin_Modelling::Depth );
         getPropValsOfFirstColumnInLayer(Current_Layer->layerDA, 
                                         &propVec,
                                         1.0, 
                                         m_Basin_Model->getValidNeedles (),
                                         layerPropsOnNodeColumn);

         layerData -> m_depthAtLayerNodes.push_back( std::vector<double>() );

         for (iNodeProp = layerPropsOnNodeColumn.rbegin(); iNodeProp !=layerPropsOnNodeColumn.rend(); ++iNodeProp)
         {
            layerData -> m_depthAtLayerNodes[ layerData -> m_depthAtLayerNodes.size()-1 ].push_back(*iNodeProp); 
         }
         layerPropsOnNodeColumn.clear();

         //get temperatures
         propVec = Current_Layer->Current_Properties( Basin_Modelling::Temperature );
         getPropValsOfFirstColumnInLayer(Current_Layer->layerDA, 
                                         &propVec,
                                         1.0, 
                                         m_Basin_Model->getValidNeedles (),
                                         layerPropsOnNodeColumn);

         layerData -> m_temperatureAtLayerNodes.push_back( std::vector<double>() );

         for (iNodeProp = layerPropsOnNodeColumn.rbegin(); iNodeProp !=layerPropsOnNodeColumn.rend(); ++iNodeProp)
         {
            layerData -> m_temperatureAtLayerNodes[ layerData -> m_temperatureAtLayerNodes.size()-1 ].push_back(*iNodeProp); 
         }
         layerPropsOnNodeColumn.clear();

         //get VES
         propVec = Current_Layer->Current_Properties( Basin_Modelling::VES_FP );
         getPropValsOfFirstColumnInLayer(Current_Layer->layerDA, 
                                         &propVec,
                                         1.0, 
                                         m_Basin_Model->getValidNeedles (),
                                         layerPropsOnNodeColumn);
         
         layerData -> m_VES_atLayerNodes.push_back( std::vector<double>() );
       
         for (iNodeProp = layerPropsOnNodeColumn.rbegin(); iNodeProp !=layerPropsOnNodeColumn.rend(); ++iNodeProp)
         {
            layerData -> m_VES_atLayerNodes[ layerData -> m_VES_atLayerNodes.size()-1 ].push_back(*iNodeProp); 
         }
         layerPropsOnNodeColumn.clear();

         //get maxVES
         propVec = Current_Layer->Current_Properties( Basin_Modelling::Max_VES );
         getPropValsOfFirstColumnInLayer(Current_Layer->layerDA, 
                                         &propVec,
                                         1.0, 
                                         m_Basin_Model->getValidNeedles (),
                                         layerPropsOnNodeColumn);
         
         layerData -> m_maxVES_atLayerNodes.push_back( std::vector<double>() );
       
         for (iNodeProp = layerPropsOnNodeColumn.rbegin(); iNodeProp !=layerPropsOnNodeColumn.rend(); ++iNodeProp)
         {
            layerData -> m_maxVES_atLayerNodes[ layerData -> m_maxVES_atLayerNodes.size()-1 ].push_back(*iNodeProp); 
         }
         layerPropsOnNodeColumn.clear();
         
         //get chemical compaction
         propVec = Current_Layer->Current_Properties( Basin_Modelling::Chemical_Compaction );
         getPropValsOfFirstColumnInLayer(Current_Layer->layerDA, 
                                         &propVec,
                                         1.0, 
                                         m_Basin_Model->getValidNeedles (),
                                         layerPropsOnNodeColumn);
         
         layerData -> m_chemicalCompactionAtLayerNodes.push_back( std::vector<double>() );
       
         for (iNodeProp = layerPropsOnNodeColumn.rbegin(); iNodeProp !=layerPropsOnNodeColumn.rend(); ++iNodeProp)
         {
            layerData -> m_chemicalCompactionAtLayerNodes[ layerData -> m_chemicalCompactionAtLayerNodes.size()-1 ].push_back(*iNodeProp); 
         }
         layerPropsOnNodeColumn.clear();

      }
      Layers++;
   }
}

void FissionTrackCalculator::computeSolidThicknessSampleToLowerNode(SampleTrackingData* sampleData)
{
   //The solid thickness between the sample and the next node below is computed by integrating 
   //
   //  ds
   //  -- = 1 - phi(VES, maxVES,chemicalCompaction)
   //  dz
   //
   //using the trapezoidal rule.

   int iBot = sampleData -> m_lowerNodeIndex;
   int iTop = sampleData -> m_upperNodeIndex;

   size_t iTime = sampleData -> m_layerTrackingData -> m_referenceTimes.size() - 1 ;

   double VES_bottom               = sampleData -> m_layerTrackingData -> m_VES_atLayerNodes[iTime][iBot];
   double VES_top                  = sampleData -> m_layerTrackingData -> m_VES_atLayerNodes[iTime][iTop]; 
   double maxVES_bottom            = sampleData -> m_layerTrackingData -> m_maxVES_atLayerNodes[iTime][iBot];
   double maxVES_top               = sampleData -> m_layerTrackingData -> m_maxVES_atLayerNodes[iTime][iTop]; 
   
   const bool includeChemicalCompaction = (    ( m_Basin_Model -> Do_Chemical_Compaction ) 
                                            && ( sampleData -> m_layer -> Get_Chemical_Compaction_Mode () ) );
   double chemicalCompactionBottom = sampleData -> m_layerTrackingData -> m_chemicalCompactionAtLayerNodes[iTime][iBot];
   double chemicalCompactionTop    = sampleData -> m_layerTrackingData -> m_chemicalCompactionAtLayerNodes[iTime][iTop]; 
      
   double realThicknessElement = sampleData -> m_lowerNodeDepth - sampleData -> m_upperNodeDepth;

   const CompoundLithology* lithology = sampleData -> m_layer -> getCompoundLithologyArray ()( 0, 0, 0.0 );
   
   //get real thickness between sample and lower node
   double realThicknessLowerNodeToSample = sampleData->m_lowerNodeDepth - sampleData->m_depth; 

   const int numberOfSteps = numberOfStepsForCompactionEquation ( m_Basin_Model -> Optimisation_Level );
   const double dReal = realThicknessLowerNodeToSample / double ( numberOfSteps );

   double solidThickness = 0.0;
   double realThickness  = 0.0;

   double VES_start;
   double maxVES_start;
   double chemicalCompactionStart;

   double porosityStart, porosityEnd; //values at start and end of each integration interval
   porosityStart = lithology -> porosity ( VES_bottom, maxVES_bottom, includeChemicalCompaction, chemicalCompactionBottom );

   //integrate compaction equation over solid thickness to get real thickness 
   //start from bottom of element
   //use trapezoidal rule for each integration step (interval)

   for ( int iStep = 0; iStep < numberOfSteps; ++iStep ) 
   {
      double ratioEnd   = ( realThickness + dReal ) / realThicknessElement;

      double VES_end    = VES_bottom + ratioEnd * ( VES_top - VES_bottom );
      double maxVES_end = maxVES_bottom + ratioEnd * ( maxVES_top - maxVES_bottom ); 
      double chemicalCompactionEnd    = chemicalCompactionBottom + ratioEnd * ( chemicalCompactionTop - chemicalCompactionBottom );
      
      porosityEnd       = lithology -> porosity ( VES_end, maxVES_end, includeChemicalCompaction, chemicalCompactionEnd );
      
      double dSolid = 0.5 * ( ( 1.0 - porosityStart ) +  ( 1.0 - porosityEnd ) ) * dReal;
    
      solidThickness += dSolid;
      realThickness  += dReal;
      
      //copy values of end of integration step to start of next step
      VES_start = VES_end;
      maxVES_start = maxVES_end;
      chemicalCompactionStart = chemicalCompactionEnd;
      porosityStart = porosityEnd;
   } 
   
   int elementIndex = sampleData -> m_lowerNodeIndex; 
   double solidThicknessElement = sampleData -> m_layer->getSolidThickness ( 0, 0, elementIndex, 0.0 );
//    double solidThicknessElement = sampleData -> m_layer -> FCT( 0,0,elementIndex ).F( 0.0 );
   
   //make sure that solid thickness below sample is not out of range due to integratin errors
   solidThickness = ( solidThickness <= solidThicknessElement? solidThickness : solidThicknessElement );
   solidThickness = ( solidThickness >= 0.0 ? solidThickness : 0.0 );
   sampleData->m_solidThicknessLowerNodeToSample = solidThickness;

}

void FissionTrackCalculator::interpolateTemperatureToSampleLocationNonMobileLayer(SampleTrackingData* sampleData)
{
   //loop over all timesteps
   size_t nTimes = sampleData  -> m_layerTrackingData -> m_referenceTimes.size();
   for ( size_t iTime = 0; iTime < nTimes; ++iTime )
   {
      //get solid thickness of element in timestep
      int elementIndex = sampleData -> m_lowerNodeIndex;
      double time =  sampleData -> m_layerTrackingData -> m_referenceTimes[iTime];
      
      double ratio; 

      double solidThicknessElement = sampleData -> m_layer->getSolidThickness ( 0, 0, elementIndex, time );
//       double solidThicknessElement = sampleData -> m_layer -> FCT( 0,0,elementIndex ).F( time );

      if (sampleData->m_solidThicknessLowerNodeToSample <= solidThicknessElement) //sample already deposited?
      {

         //get real thickness from lower node to sample
         int iBot = sampleData -> m_lowerNodeIndex;
         int iTop = sampleData -> m_upperNodeIndex;

         double VES_bottom               = sampleData -> m_layerTrackingData -> m_VES_atLayerNodes[iTime][iBot];
         double VES_top                  = sampleData -> m_layerTrackingData -> m_VES_atLayerNodes[iTime][iTop]; 
         double maxVES_bottom            = sampleData -> m_layerTrackingData -> m_maxVES_atLayerNodes[iTime][iBot];
         double maxVES_top               = sampleData -> m_layerTrackingData -> m_maxVES_atLayerNodes[iTime][iTop]; 
      
         const bool includeChemicalCompaction = (   ( m_Basin_Model -> Do_Chemical_Compaction ) 
                                                    && ( sampleData -> m_layer -> Get_Chemical_Compaction_Mode ()));
         double chemicalCompactionBottom = sampleData -> m_layerTrackingData -> m_chemicalCompactionAtLayerNodes[iTime][iBot];
         double chemicalCompactionTop    = sampleData -> m_layerTrackingData -> m_chemicalCompactionAtLayerNodes[iTime][iTop]; 
         
         double realThicknessElement = sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[iTime][iBot]
                                     - sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[iTime][iTop];
         
         double realThicknessLowerNodeToSample = computeRealThicknessLowerNodeToSample( sampleData -> m_solidThicknessLowerNodeToSample, 
                                                                                        VES_bottom, 
                                                                                        VES_top,
                                                                                        maxVES_bottom,
                                                                                        maxVES_top,
                                                                                        includeChemicalCompaction,
                                                                                        chemicalCompactionBottom, 
                                                                                        chemicalCompactionTop, 
                                                                                        realThicknessElement,
                                                                                        sampleData->m_layer->getCompoundLithologyArray ()( 0, 0, time ));

         //compute real thickness ratio
         ratio = realThicknessLowerNodeToSample / realThicknessElement;

         ratio = (ratio < 0.0 ? 0.0 : ratio ) ; //inaccuracy in geometric loop might push sample out of element: push it back
         ratio = (ratio > 1.0 ? 1.0 : ratio ) ; //inaccuracy in geometric loop might push sample out of element: push it back

//          cout << "time " << time << " layer " << sampleData->m_formationName << " realRatio " << ratio 
//               << " real thickness below "  <<  realThicknessLowerNodeToSample << " real thickness element " <<   realThicknessElement 
//               << " FCT below " << sampleData -> m_solidThicknessLowerNodeToSample  << " FCT element " << solidThicknessElement 
//               << " zBotNode " << sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[iTime][iBot] 
//               << " zTopNode " << sampleData -> m_layerTrackingData -> m_depthAtLayerNodes[iTime][iTop]
//               << endl;

         //interpolate temperature (using real thickness ratio)
         std::vector<std::vector<double> >& temperatureAtLayerNodes = sampleData -> m_layerTrackingData -> m_temperatureAtLayerNodes;
         double temperature_bot = temperatureAtLayerNodes[iTime][sampleData->m_lowerNodeIndex];
         double temperature_top = temperatureAtLayerNodes[iTime][sampleData->m_upperNodeIndex];
         
         double temperature = temperature_bot + ratio * (temperature_top - temperature_bot);
         
         //save input for fission track kernel (temperature and time)
         sampleData -> m_inputNodes.push_back(Calibration::NodeInput( time, temperature ) );
      }
   }
}

void FissionTrackCalculator::interpolateTemperatureToSampleLocationMobileLayer(SampleTrackingData* sampleData)
{
   //loop over all timesteps
   size_t nTimes = sampleData -> m_layerTrackingData -> m_referenceTimes.size();
   for ( size_t iTime = 0; iTime < nTimes; ++iTime )
   {
      //get solid thickness of element in timestep
      int elementIndex = sampleData -> m_lowerNodeIndex;
      double time =  sampleData -> m_layerTrackingData -> m_referenceTimes[iTime];
      
      //for mobile layers just use the real thickness ratio of present day to interpolate paleo temperatures within the layer
      double ratio = ( sampleData -> m_lowerNodeDepth - sampleData -> m_depth ) / (sampleData -> m_lowerNodeDepth - sampleData ->m_upperNodeDepth ); 

      double solidThicknessElement = sampleData -> m_layer->getSolidThickness ( 0, 0, elementIndex, time );
//       double solidThicknessElement = sampleData -> m_layer -> FCT( 0,0,elementIndex ).F( time );

      if (sampleData->m_solidThicknessLowerNodeToSample <= solidThicknessElement) //sample already deposited?
      {
         //interpolate temperature (using real thickness ratio)
         std::vector<std::vector<double> >& temperatureAtLayerNodes = sampleData -> m_layerTrackingData -> m_temperatureAtLayerNodes;
         double temperature_bot = temperatureAtLayerNodes[iTime][sampleData->m_lowerNodeIndex];
         double temperature_top = temperatureAtLayerNodes[iTime][sampleData->m_upperNodeIndex];
         
         double temperature = temperature_bot + ratio * (temperature_top - temperature_bot);
         
         //save input for fission track kernel (temperature and time)
         sampleData -> m_inputNodes.push_back(Calibration::NodeInput( time, temperature ) );
      }
   }
}

double FissionTrackCalculator::computeRealThicknessLowerNodeToSample( const double solidThicknessLowerNodeToSample, 
                                                                      const double VES_bottom, 
                                                                      const double VES_top,
                                                                      const double maxVES_bottom,
                                                                      const double maxVES_top,
                                                                      const bool   includeChemicalCompaction,
                                                                      const double chemicalCompactionBottom, 
                                                                      const double chemicalCompactionTop, 
                                                                      const double realThicknessElement,
                                                                      const CompoundLithology* lithology  
                                                                      )
{
   const int numberOfSegments = numberOfStepsForCompactionEquation ( m_Basin_Model -> Optimisation_Level );
   const double dSolid = solidThicknessLowerNodeToSample / double ( numberOfSegments );
  
   double realThickness = 0.0;
   
   //integrate compaction equation over solid thickness to get real thickness; start from bottom of element
   for ( int iStep = 0; iStep < numberOfSegments; ++iStep ) 
   {
      double ratioStart   = realThickness / realThicknessElement;
      
      double VES_start    = VES_bottom + ratioStart * ( VES_top - VES_bottom );
      double maxVES_start = maxVES_bottom + ratioStart * ( maxVES_top - maxVES_bottom ); 
      double chemicalCompactionStart    = chemicalCompactionBottom + ratioStart * ( chemicalCompactionTop - chemicalCompactionBottom );
      
      double porosityStart = lithology -> porosity ( VES_start, maxVES_start, includeChemicalCompaction, chemicalCompactionStart );
      
      double dRealGuess = dSolid / ( 1.0 - porosityStart );

      double ratioEnd   = ( realThickness + dRealGuess ) / realThicknessElement;

      double VES_end    = VES_bottom + ratioEnd * ( VES_top - VES_bottom );
      double maxVES_end = maxVES_bottom + ratioEnd * ( maxVES_top - maxVES_bottom ); 
      double chemicalCompactionEnd    = chemicalCompactionBottom + ratioEnd * ( chemicalCompactionTop - chemicalCompactionBottom );
      
      double porosityEnd = lithology -> porosity ( VES_end, maxVES_end, includeChemicalCompaction, chemicalCompactionEnd );
      
      double dReal = 0.5 * ( 1.0 / ( 1.0 - porosityStart ) +  1.0 / ( 1.0 - porosityEnd ) ) * dSolid;
    
      realThickness += dReal;
      
      //cout << "integration step " << iStep << " porosityStart " << porosityStart << " porosityEnd " << porosityEnd << endl; 

   } 
   return realThickness;
}

bool FissionTrackCalculator::compute(void)
{
   typedef std::map<std::string, SampleTrackingData*>::iterator SampleIt; 

   typedef std::map<std::string, LayerTrackingData*>::const_iterator LayerIt;

   typedef std::vector<Calibration::NodeInput>::const_iterator InputIt;


   determineUpperAndLowerNodeForSample();

   for (SampleIt it = m_sampleTrackingData.begin(), itEnd = m_sampleTrackingData.end(); it != itEnd; ++it)
   {
      SampleTrackingData *theTrackingData = it->second;

      computeSolidThicknessSampleToLowerNode(theTrackingData);
     
      if (! theTrackingData->m_layer->isMobile() )
      {
         interpolateTemperatureToSampleLocationNonMobileLayer(theTrackingData);
      }
      else
      {
         interpolateTemperatureToSampleLocationMobileLayer(theTrackingData);
      }

      Calibration::FissionTracks::FtSamplePrediction* theFtPredSample =   m_theFtAnalysis->getPredSample(it->first); 
      //cout<<"Computing sample: "<<it->first<<endl;

      if( theTrackingData && theTrackingData->m_inputNodes.size() > 1 )
      {
         InputIt itInputNode     = theTrackingData->m_inputNodes.begin(), 
                 itInputNodesEnd = theTrackingData->m_inputNodes.end() ;

         //get reference time from the start
         double previousAge = itInputNode -> getReferenceTime();

         //get the next one
         ++itInputNode;

         Calibration::FissionTracks::FtParameters::FtParameters& params = Calibration::FissionTracks::FtParameters::getInstance();

         for(;itInputNode != itInputNodesEnd; ++itInputNode)
         {
            double timestepSizeMa  = previousAge - itInputNode->getReferenceTime();
            double timestepSizeSec = timestepSizeMa * params.getSecsInMa();
            double temperatureCelsius = itInputNode->getTemperatureCelsius() ; 
            double temperatureKelvin = temperatureCelsius + params.getKelvinAt0Celsius();

            int elementIndex = theTrackingData -> m_lowerNodeIndex; 
            double solidThicknessElement = theTrackingData -> m_layer->getSolidThickness ( 0, 0, elementIndex, previousAge );
//             double solidThicknessElement = theTrackingData -> m_layer -> FCT( 0,0,elementIndex ).F( previousAge );
   
            if (theTrackingData->m_solidThicknessLowerNodeToSample <= solidThicknessElement) //sample already deposited?
            {
               theFtPredSample->advanceState (timestepSizeSec, temperatureKelvin);
            }

            //cout<<it->first<<" "<<timestepSizeSec<<" "<<temperatureKelvin<<endl;
            previousAge = itInputNode->getReferenceTime();
         }
      }
   }
      
   m_theFtAnalysis->finalize();

   return true;

}


#undef __FUNCT__  
#define __FUNCT__ "FissionTrackCalculator::writeFissionTrackResultsToDatabaseTables"

 bool FissionTrackCalculator::writeFissionTrackResultsToDatabaseTables(database::Table &FtSampleIoTbl, 
                                                                       database::Table &FtGrainIoTbl,
                                                                       database::Table &FtPredLengthCountsHistIoTbl,
                                                                       database::Table &FtPredLengthCountsHistDataIoTbl,
                                                                       database::Table &FtClWeightPercBinsIoTbl
                                                                       ) const
{
   using namespace Calibration::FissionTracks;
   using namespace database;

   //write results to FtSampleIoTbl
   for (size_t i = 0; i < FtSampleIoTbl.size (); ++i)
   {
      Record* record = FtSampleIoTbl.getRecord (i);
      assert (record);

      string sampleId = getFtSampleId(record);

      FtSamplePrediction*  ftPredSample = m_theFtAnalysis -> getPredSample ( sampleId );
      FtSampleObservation* ftObsSample  = m_theFtAnalysis -> getObsSample  ( sampleId );
       
      setFtPredictedAge    ( record, ftPredSample    -> getPredAge()          );
      setFtPooledAge       ( record, ftObsSample     -> getPooledAge()        );
      setFtPooledAgeErr    ( record, ftObsSample     -> getPooledAgeErr()     );
      setFtAgeChi2         ( record, ftObsSample     -> getAgeChi2()          );
      setFtDegreeOfFreedom ( record, ftObsSample     -> getDegreesOfFreedom() );
      setFtPAgeChi2        ( record, ftObsSample     -> getPAgeChi2()         );
      setFtCorrCoeff       ( record, ftObsSample     -> getCorrCoeff()        );
      setFtVarianceSqrtNs  ( record, ftObsSample     -> getVarSqrtNs()        );
      setFtVarianceSqrtNi  ( record, ftObsSample     -> getVarSqrtNi()        );
      setFtNsDivNi         ( record, ftObsSample     -> getNsDivNi()          );
      setFtNsDivNiErr      ( record, ftObsSample     -> getNsDivNiErr()       );
      setFtMeanRatio       ( record, ftObsSample     -> getMeanRatioNsNi()    );
      setFtMeanRatioErr    ( record, ftObsSample     -> getMeanRatioNsNiErr() );
      setFtCentralAge      ( record, ftObsSample     -> getCentralAge()       );
      setFtCentralAgeErr   ( record, ftObsSample     -> getCentralAgeErr()    );
      setFtMeanAge         ( record, ftObsSample     -> getMeanAge()          );
      setFtMeanAgeErr      ( record, ftObsSample     -> getMeanAgeErr()       );
      setFtLengthChi2      ( record, m_theFtAnalysis -> getLengthChi2( sampleId ) ); 
      setFtApatiteYield    ( record, ftObsSample     -> getApatiteYield()     );
   }

   //write results to FtGrainIoTbl
   for (size_t i = 0; i < FtGrainIoTbl.size (); ++i)
   {
      Record * record = FtGrainIoTbl.getRecord (i);
      assert (record);

      string sampleId = getFtSampleId( record );

      FtSampleObservation* ftObsSample  = m_theFtAnalysis -> getObsSample  ( sampleId );
      
      FtGrain* grain = ftObsSample -> getGrain( getFtGrainId( record ) );
      
      if ( grain )
      {
         setFtGrainAge    ( record, grain -> getGrainAge()    );
         setFtGrainAgeErr ( record, grain -> getGrainAgeErr() );
      }
   }

   //write predicted fission track length distributions
   FtParameters& params = FtParameters::getInstance();

   FtPredLengthCountsHistIoTbl.clear();
   FtPredLengthCountsHistDataIoTbl.clear();

   int histogramId = 0;
   //loop over all samples in the FtSampleIoTbl
   for (size_t iSample = 0; iSample < FtSampleIoTbl.size(); ++iSample )
   {
      Record* sampleRecord = FtSampleIoTbl.getRecord(iSample);
      assert (sampleRecord);

      string sampleId = getFtSampleId(sampleRecord);

      FtSamplePrediction*  ftPredSample = m_theFtAnalysis -> getPredSample ( sampleId );
      
      //loop over all Cl-Weight bins
      for (int iClBin = 0; iClBin < params.getNumBinClWeightPercent() ; ++iClBin )
      {
         Histogram * predHist = ftPredSample -> getTrackLengths(iClBin);
         if (predHist)
         {
            ++histogramId;

            //create new histogram with layout
            Record* histRecord = FtPredLengthCountsHistIoTbl.createRecord();
            assert(histRecord);
         
            //predicted ages are always computed with the center value of the Cl-Bin
            double ClWeightPerc = ( iClBin + 0.5 ) * ( params.getStepBinClWeightPercent() );

            setFtPredLengthHistId   ( histRecord, histogramId );
            setFtSampleId           ( histRecord, sampleId );
            setFtClWeightPerc       ( histRecord, ClWeightPerc );
            setFtPredLengthBinStart ( histRecord, predHist -> getBinStart() );
            setFtPredLengthBinWidth ( histRecord, predHist -> getBinWidth() );
            setFtPredLengthBinNum   ( histRecord, predHist -> getBinNum() );

            //dump histogram data
            for( int iLengthBin = 0; iLengthBin < predHist -> getBinNum(); ++iLengthBin )
            {
               if ( predHist -> BinCount (iLengthBin) > 0.0 )
               {
                  Record* dataRecord = FtPredLengthCountsHistDataIoTbl.createRecord ();
                  assert(dataRecord);

                  setFtPredLengthHistId   ( dataRecord, histogramId ); 
                  setFtPredLengthBinIndex ( dataRecord, iLengthBin );
                  setFtPredLengthBinCount ( dataRecord, predHist -> BinCount (iLengthBin) );
               }
            }
         }
      }
   }

   //write layout of Cl-bins (FtClWeightPercBinsIoTbl)
   FtClWeightPercBinsIoTbl.clear();
   Record* record = FtClWeightPercBinsIoTbl.createRecord();
   assert(record);
   
   setFtClWeightBinStart(record, params.getStartBinClWeightPercent() );
   setFtClWeightBinWidth(record, params.getStepBinClWeightPercent()  );

   return true;
}

int FissionTrackCalculator::numberOfStepsForCompactionEquation ( const int optimisationLevel ) const 
{
   static const int NumberOfSteps [ NumberOfOptimisationLevels ] = { 3, 3, 4, 4, 5 };
   //static const int NumberOfSteps [ NumberOfOptimisationLevels ] = { 1, 1, 1, 1, 1 };

  return NumberOfSteps [ optimisationLevel - 1 ];
}
