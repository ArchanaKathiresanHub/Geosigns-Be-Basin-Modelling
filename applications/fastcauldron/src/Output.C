#include "Output.h"
#include "layer_iterators.h"
#include "utils.h"
#include "timefilter.h"


double getPropValOfFirstNodeOnSurface(DM a_DA,
                                      Vec* a_vec,
                                      const SurfacePosition surfacePosition, 
                                      const double factor,
                                      const Boolean2DArray& validNeedle )
{

  //Get the info about our part of the distributed array
  int start[3]  = {0, 0, 0}; 
  int count[3]  = {1, 1, 1}; 

  DMDAGetCorners(a_DA,&start[0],&start[1],&start[2],&count[0],&count[1],&count[2]);

  //*** get the info about the total model
  int size[3] = {0, 0, 0};
  DMDAGetInfo(a_DA, PETSC_IGNORE,&size[0],&size[1],&size[2], 
              PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE, 
              PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE);
  
  double returnValue = CAULDRONIBSNULLVALUE;

  
  //each process gets its first node (but only rank0 process writes the project-file)
  int yCnt = start[1];    
  int xCnt = start[0]; 
  
  if ( size[ 2 ] == 1 )
  {

    PETSC_2D_Array x( a_DA, *a_vec, INSERT_VALUES, true );

    if ( ! validNeedle ( xCnt, yCnt ) || x(yCnt,xCnt) == CAULDRONIBSNULLVALUE) 
    {
       returnValue = CAULDRONIBSNULLVALUE;
    } 
    else 
    {
       returnValue = x(yCnt,xCnt) * factor;
    }

  }
  else
  {
    PETSC_3D_Array x( a_DA, *a_vec, INSERT_VALUES, true );
    
    int zCnt;
    if (surfacePosition == TOPSURFACE) 
    {
       zCnt = start[2]+count[2]-1; 
    } 
    else 
    {
       zCnt = 0;
    }

    if ( ! validNeedle ( xCnt, yCnt ) || x(zCnt,yCnt,xCnt) == CAULDRONIBSNULLVALUE) 
    {
       returnValue = CAULDRONIBSNULLVALUE;
    } 
    else 
    {
       returnValue = x(zCnt,yCnt,xCnt) * factor;
    }
  }      

  return returnValue;
}



//output of 1D needle for DepthIoTbl
void savePropertiesOnSegmentNodes1D( AppCtx*              basinModel,
				     const SnapshotEntry* snapshot )
{

   //relevant snapshot?
   if ( snapshot->time() != Present_Day )
      return;

   //create list with properties to be output
   list<PropertyList> outputProperties1D;
   
   outputProperties1D.push_back ( DEPTH );
   outputProperties1D.push_back ( POROSITYVEC );
   outputProperties1D.push_back ( PERMEABILITYVEC );
//    outputProperties1D.push_back ( PERMEABILITYHVEC );
   outputProperties1D.push_back ( TEMPERATURE );
   outputProperties1D.push_back ( DIFFUSIVITYVEC );
   outputProperties1D.push_back ( VR );
   outputProperties1D.push_back ( HEAT_FLOW );
//    outputProperties1D.push_back ( HEAT_FLOWY );
//    outputProperties1D.push_back ( HEAT_FLOWZ );
   outputProperties1D.push_back ( BULKDENSITYVEC );
   outputProperties1D.push_back ( VELOCITYVEC );
   outputProperties1D.push_back ( SONICVEC );
   outputProperties1D.push_back ( REFLECTIVITYVEC );
   outputProperties1D.push_back ( THCONDVEC );
   //outputProperties1D.push_back ( BIOMARKERS );
   outputProperties1D.push_back ( STERANEAROMATISATION );
   outputProperties1D.push_back ( STERANEISOMERISATION );
   outputProperties1D.push_back ( HOPANEISOMERISATION );
   outputProperties1D.push_back ( ILLITEFRACTION );

   if ( ! basinModel->IsCalculationCoupled ) 
   {
      outputProperties1D.push_back ( VES );
      outputProperties1D.push_back ( MAXVES );
      outputProperties1D.push_back ( OVERPRESSURE );
      outputProperties1D.push_back ( PRESSURE );
      outputProperties1D.push_back ( HYDROSTATICPRESSURE );
      outputProperties1D.push_back ( LITHOSTATICPRESSURE );
   }

   //loop over properties
   list<PropertyList>::const_iterator iProp;
   for ( iProp = outputProperties1D.begin(); iProp != outputProperties1D.end() ; ++iProp)
   {
 
      //loop over layers
      /* Select the output range */
      OutputOption outputOption = basinModel -> timefilter.PropertyOutputOption[ *iProp ];

      /* Select the appropriate layers */
      LayerStack selectedLayers;
      basinModel -> selectLayers( *iProp, outputOption, selectedLayers );

      /* Check if property is a vector property */
      /* e.g. Porosity is a Vector - Temperature is a Scalar */
      bool isVectorProperty = ( *iProp < DEPTH );

      /* Determine property name */
      string propertyName = basinModel -> timefilter.getPropertyName( *iProp );

      double factor = 1.0;
      
      /* property related modification */
      if ( propertyName == "PorosityVec" )       factor         = 100.0;
      if ( propertyName == "HeatFlow" )          propertyName += "X";
      if ( propertyName == "FluidVelocity" )     propertyName += "X";
      if ( isVectorProperty )                    propertyName += "2";
      if ( basinModel -> DoHighResDecompaction ) propertyName += "HighRes";
      
      list<double> topPropVals,  bottomPropVals, topDepthVals,  bottomDepthVals;//DepthioTbl: vector-properties-index 0==top, 1==bottom
      
      for (size_t iLayer = 0; iLayer < selectedLayers.size(); ++iLayer ) 
      {
         int layerNumber  = selectedLayers[ iLayer ];
         LayerProps_Ptr currentLayer = basinModel -> layers[ layerNumber ];

         //loop over all (segment-)nodes in layer
         
         PetscBool validVector;

         VecValid ( *currentLayer -> vectorList.VecArray[ *iProp ], &validVector );

         if ( ! validVector ) 
         {
            /// Cannot write out the vector, since it is not valid.
            return;
         }

         
         list<double> layerPropsOnNodeColumn;
         getPropValsOfFirstColumnInLayer(currentLayer->layerDA, 
                                         currentLayer -> vectorList.VecArray[ *iProp ],
                                         factor, 
                                         basinModel->getValidNeedles (),
                                         layerPropsOnNodeColumn);
         list<double> layerDepthsOnNodeColumn;
         getPropValsOfFirstColumnInLayer(currentLayer->layerDA, 
                                         currentLayer -> vectorList.VecArray[ DEPTH ],
                                         1.0, 
                                         basinModel->getValidNeedles (),
                                         layerDepthsOnNodeColumn);

         list<double>::const_iterator iNodeVal, iNodeDepth, topNodeDepth, bottomNodeDepth;

         topNodeDepth = layerDepthsOnNodeColumn.begin();
         bottomNodeDepth = layerDepthsOnNodeColumn.end(); --bottomNodeDepth; //point to last valid element in list
          
         for ( iNodeVal  = layerPropsOnNodeColumn.begin(),  iNodeDepth = layerDepthsOnNodeColumn.begin() ; 
               iNodeVal != layerPropsOnNodeColumn.end()  || iNodeDepth != layerDepthsOnNodeColumn.end(); 
               ++iNodeVal, ++ iNodeDepth )
         {
            bool isTopNodeInLayer    = ( iNodeDepth  == topNodeDepth );
            bool isBottomNodeInLayer = ( iNodeDepth  == bottomNodeDepth );
            
            if (! isBottomNodeInLayer)
            {
               topPropVals.push_back(*iNodeVal);
               topDepthVals.push_back(*iNodeDepth);
            }
            if (! isTopNodeInLayer)
            {
               bottomPropVals.push_back(*iNodeVal);
               bottomDepthVals.push_back(*iNodeDepth);
            }
         }
      }

      //output into DepthIoTbl   
      
      list<double>::const_iterator iNodeVal, iNodeDepth;
      
      string topExtension    = (isVectorProperty?"[0]" : ""); 
      string bottomExtension = (isVectorProperty?"[1]" : ""); 

      for ( iNodeVal  = topPropVals.begin(),  iNodeDepth  = topDepthVals.begin() ;  
            iNodeVal != topPropVals.end()  || iNodeDepth != topDepthVals.end(); 
            ++iNodeVal, ++iNodeDepth )
      {
         basinModel -> addDepthIoRecord ( propertyName+topExtension, snapshot->time(), *iNodeDepth, *iNodeVal );
      }
      
      if ( !isVectorProperty ) //for scalar properties only output the last element of the bottom lists
      {
         unsigned int nItemsToDelete = ( ( bottomDepthVals.size() > 0 ) ? bottomDepthVals.size() - 1 : 0 );
         for ( unsigned int i = 0; i < nItemsToDelete ;++i )
         {   
            bottomPropVals.pop_front ();
            bottomDepthVals.pop_front();
         }
      }
      
      for ( iNodeVal  = bottomPropVals.begin(),  iNodeDepth =  bottomDepthVals.begin() ;  
            iNodeVal != bottomPropVals.end()  || iNodeDepth != bottomDepthVals.end(); 
            ++iNodeVal, ++ iNodeDepth )
      {
         basinModel -> addDepthIoRecord ( propertyName+bottomExtension, snapshot->time(), *iNodeDepth, *iNodeVal );
      }
   }
}


void getPropValsOfFirstColumnInLayer(DM a_DA,
                                     Vec* a_vec,
                                     const double factor,
                                     const Boolean2DArray& validNeedle,
                                     list<double>& returnValList)
{
   returnValList.clear();

  //Get the info about our part of the distributed array
  int start[3]  = {0, 0, 0}; 
  int count[3]  = {1, 1, 1}; 

  DMDAGetCorners(a_DA,&start[0],&start[1],&start[2],&count[0],&count[1],&count[2]);

  //*** get the info about the total model
  int size[3] = {0, 0, 0};
  DMDAGetInfo(a_DA, PETSC_IGNORE,&size[0],&size[1],&size[2], 
	    PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE, 
	    PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE);

 
  //each process gets its first node (but only rank0 process writes the project-file)
  int yCnt = start[1];    
  int xCnt = start[0]; 
  
  if ( size[ 2 ] == 1 )
  {

    PETSC_2D_Array x( a_DA, *a_vec, INSERT_VALUES, true );

    if ( ! validNeedle ( xCnt, yCnt ) || x(yCnt,xCnt) == CAULDRONIBSNULLVALUE) 
    {
       returnValList.push_back( CAULDRONIBSNULLVALUE ); 
    } 
    else 
    {
       returnValList.push_back( x(yCnt,xCnt) * factor ); 
    }

  }
  else
  {
    PETSC_3D_Array x( a_DA, *a_vec, INSERT_VALUES, true );
    
    for (int zCnt =  start[2]+count[2]-1 ; zCnt >= start[2]  ; --zCnt) 
    {
       if ( ! validNeedle ( xCnt, yCnt ) || x(zCnt,yCnt,xCnt) == CAULDRONIBSNULLVALUE) 
       {
          returnValList.push_back( CAULDRONIBSNULLVALUE ); 
       } 
       else 
       {
          returnValList.push_back( x(zCnt,yCnt,xCnt) * factor );
       }
    }
  }      
}


