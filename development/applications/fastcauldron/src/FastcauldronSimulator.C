#include "FastcauldronSimulator.h"
#include <sys/stat.h>

#include <algorithm>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "GenexResultManager.h"
#include "ComponentManager.h"

#include "propinterface.h"
#include "FastcauldronFactory.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsSourceRock.h"
#include "Property.h"
#include "PropertyManager.h"

#include "cauldronschemafuncs.h"

#include "Interface/Grid.h"
#include "Interface/RunParameters.h"
#include "Interface/Surface.h"
#include "Interface/OutputProperty.h"
#include "Interface/ObjectFactory.h"
#include "Interface/Property.h"
#include "Interface/OutputProperty.h"
#include "Interface/Interface.h"
#include "Interface/ConstrainedOverpressureInterval.h"

#include "NumericFunctions.h"
#include "globaldefs.h"

#include "EosPack.h"
#include "PVTCalculator.h"

#include "MultiComponentFlowHandler.h"

#ifndef _MSC_VER
#include "h5merge.h"
#endif

//------------------------------------------------------------//

bool SnapshotRecordLess ( database::Record* left,
// bool database::SnapshotRecordLess ( database::Record* left,
                                    database::Record* right ) {
   return database::getTime ( left ) < database::getTime ( right );
}

//------------------------------------------------------------//

FastcauldronSimulator* FastcauldronSimulator::m_fastcauldronSimulator = 0;

//------------------------------------------------------------//

FastcauldronSimulator::FastcauldronSimulator (database::Database * database, const std::string & name, const std::string & accessMode) 
   : GeoPhysics::ProjectHandle (database, name, accessMode) {

   m_calculationMode = NO_CALCULATION_MODE;
   m_lateralStressInterpolator = 0;

   m_mcfHandler = new MultiComponentFlowHandler;
   m_relativePermeabilityType = DefaultRelativePermeabilityFunction;

   m_minimumHcSaturation = DefaultMinimumHcSaturation;
   m_minimumWaterSaturation = DefaultMinimumWaterSaturation;
   m_waterCurveExponent = DefaultWaterCurveExponent;
   m_hcVapourCurveExponent = DefaultHcCurveExponent;
   m_hcLiquidCurveExponent = DefaultHcCurveExponent;
   m_printCommandLine = false;
   m_computeCapillaryPressure = false;

   m_fctCorrectionScalingWeight = 1.0;
}

//------------------------------------------------------------//

FastcauldronSimulator::~FastcauldronSimulator () {

   if ( m_lateralStressInterpolator == 0 ) {
      delete m_lateralStressInterpolator;
   }

   delete m_mcfHandler;

}

//------------------------------------------------------------//

FastcauldronSimulator* FastcauldronSimulator::CreateFrom ( AppCtx* cauldron ) {


   if ( m_fastcauldronSimulator == 0 ) {
      m_fastcauldronSimulator = (FastcauldronSimulator*) Interface::OpenCauldronProject ( cauldron->getProjectFileName (), "rw" );

   }

   m_fastcauldronSimulator->m_cauldron = cauldron;

   // Now load anything that was not loaded by default in the project handles constructor.
   // Now load anything that was not loaded by DAL and GeoPhysics project handle constructors.

   m_fastcauldronSimulator->loadRelatedProjects ();
   m_fastcauldronSimulator->setConstrainedOverpressureIntervals ();

   return m_fastcauldronSimulator;
}

//------------------------------------------------------------//

void FastcauldronSimulator::setFormationElementHeightScalingFactors () {

   // 2 * because there will be the formation and the scaling factor.
   const int MaximumNumberOfFormations = 1000;

   const int MaximumNumberOfFormationRefinementPairs = 2 * MaximumNumberOfFormations;

   char* namedFormationScalingArray [ MaximumNumberOfFormationRefinementPairs ];
   PetscInt namedFormationCount = MaximumNumberOfFormationRefinementPairs;
   PetscBool namedFormationRangeInput = PETSC_FALSE;

   int numberedFormationScalingArray [ MaximumNumberOfFormationRefinementPairs ];
   PetscInt numberedFormationCount = MaximumNumberOfFormationRefinementPairs;
   PetscBool numberedFormationRangeInput = PETSC_FALSE;

   // Array containing depth-refinement information for domain.
   int formationRefinement [ MaximumNumberOfFormations ];

   char* strEnd;
   int radix = 10;
   int refinementLevel;
   int formationPosition;

   int i;

   PetscOptionsGetStringArray ( PETSC_NULL, "-refinenamedforms", namedFormationScalingArray, &namedFormationCount, &namedFormationRangeInput );
   PetscOptionsGetIntArray ( PETSC_NULL, "-refinenumberedforms", numberedFormationScalingArray, &numberedFormationCount, &numberedFormationRangeInput );

   // Initialise the array containing the formation refinement information.
   for ( i = 0; i < m_formations.size (); ++i ) {
      GeoPhysics::Formation* formation = const_cast<GeoPhysics::Formation*>(dynamic_cast<const GeoPhysics::Formation*>( m_formations [ i ]));
      formationRefinement [ i ] = formation->getDepthRefinementFactor ();
   }

   for ( i = 0; i < namedFormationCount; ++i ) {
      cout << namedFormationScalingArray [ i ] << "  ";

      if ( i % 2 == 1 ) {
         cout << endl;
      }

   }

   if ( numberedFormationRangeInput ) {

      if ( numberedFormationCount %2 != 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " ERROR Command line parameter -refinumberedforms does not have the correct number of parameters. \n");
      } else {

         for ( i = 0; i < numberedFormationCount; i += 2 ) {

            if ( numberedFormationScalingArray [ i + 1 ] <= 0 ) {
               PetscPrintf ( PETSC_COMM_WORLD, " ERROR Cannot refine formation %s by %i \n",
                             m_formations [numberedFormationScalingArray [ i ]]->getName ().c_str (),
                             numberedFormationScalingArray [ i + 1 ]);
            } else {
               PetscPrintf ( PETSC_COMM_WORLD, " Refining numbered layer %s by %i \n",
                             m_formations [numberedFormationScalingArray [ i ]]->getName ().c_str (),
                             numberedFormationScalingArray [ i + 1 ]);

               formationRefinement [ numberedFormationScalingArray [ i ]] = numberedFormationScalingArray [ i + 1 ];
            }

         }

      }

   }

   if ( namedFormationRangeInput ) {

      if ( namedFormationCount %2 != 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " ERROR Command line parameter -refinenamedforms does not have the correct number of parameters. \n" );
      } else {

         for ( i = 0; i < namedFormationCount; i += 2 ) {

            if ( namedFormationScalingArray [ i + 1 ] <= 0 ) {
               PetscPrintf ( PETSC_COMM_WORLD, " ERROR Cannot refinement formation %s by %s \n",
                             namedFormationScalingArray [ i ],
                             namedFormationScalingArray [ i + 1 ]);
            } else {
               GeoPhysics::Formation* formation = const_cast<GeoPhysics::Formation*>(dynamic_cast<const GeoPhysics::Formation*>( findFormation ( namedFormationScalingArray [ i ])));
               refinementLevel = int ( std::strtol ( namedFormationScalingArray [ i + 1 ], &strEnd, radix ));
               formationPosition = getCauldron ()->getLayerIndex ( formation->getName ());

               if ( formation == 0 ) {
                  PetscPrintf ( PETSC_COMM_WORLD, " ERROR Cannot find formation named %s. \n",
                                namedFormationScalingArray [ i ]);
               } else if ( strEnd == namedFormationScalingArray [ i + 1 ]) {
                  PetscPrintf ( PETSC_COMM_WORLD, " ERROR Cannot convert %s to an integer value. \n",
                                namedFormationScalingArray [ i + 1 ]);
               } else if ( formationPosition == -1 ) {
                  PetscPrintf ( PETSC_COMM_WORLD, " ERROR Cannot find formation named %s (searching for %s). \n",
                                namedFormationScalingArray [ i + 1 ],
                                formation->getName ().c_str ());
               } else {
                  PetscPrintf ( PETSC_COMM_WORLD, " Refining named layer %s by %i \n",
                                formation->getName ().c_str (),
                                refinementLevel );

                  formationRefinement [ formationPosition ] = refinementLevel;
               }

            }

         }

      }

   }

   // Now set the depth-refinement factor for each formation.
   // Updating the value defined in the project file is required by the command-line parameters.
   for ( i = 0; i < m_formations.size (); ++i ) {
      GeoPhysics::Formation* formation = const_cast<GeoPhysics::Formation*>(dynamic_cast<const GeoPhysics::Formation*>( m_formations [ i ]));
      formation->setDepthRefinementFactor ( formationRefinement [ i ]);
   }

}

void FastcauldronSimulator::clear1DTimeIoTbl () {
   database::Table * timeIoTbl = getTable ("1DTimeIoTbl");
   PETSC_ASSERT (timeIoTbl);
   timeIoTbl->clear ();
}

void FastcauldronSimulator::clearDepthIoTbl () {
   database::Table * depthIoTbl = getTable ("DepthIoTbl");
   PETSC_ASSERT (depthIoTbl);
   depthIoTbl->clear ();
}


//------------------------------------------------------------//

void FastcauldronSimulator::initialiseFastcauldronLayers () {

   Interface::MutableFormationList::iterator formationIter;
   Interface::MutableSurfaceList::iterator   surfaceIter;

   for (formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter)
   {
      Interface::Formation* formation = * formationIter;

      LayerProps* layer = dynamic_cast <LayerProps*>( formation );
      layer->initialise ();
   }


   for ( surfaceIter = m_surfaces.begin (); surfaceIter != m_surfaces.end (); ++surfaceIter )
   {
      Interface::Surface* surface = * surfaceIter;

      // Is there another way than const_cast?
      LayerProps* layerAbove = const_cast<LayerProps*>(dynamic_cast <const LayerProps*>( surface->getTopFormation ()));
      LayerProps* layerBelow = const_cast<LayerProps*>(dynamic_cast <const LayerProps*>( surface->getBottomFormation ()));

      if ( layerBelow != 0 and layerAbove != 0 ) {
         layerBelow->connectElements ( layerAbove );
      }

   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::initialiseElementGrid ( bool& hasActiveElements ) {

   int i;
   int j;
   int localActiveElementCount = 0;
   int globalActiveElementCount;

   m_mapElements.create ( m_elementGrid.getDa ());


   Vec activeElementsVec;
   DMCreateGlobalVector ( m_elementGrid.getDa (), &activeElementsVec );

   PETSC_2D_Array activeElements ( m_elementGrid.getDa (), activeElementsVec, INSERT_VALUES, true );
   bool isActive;

   bool isLeftGhost;
   bool isRightGhost;
   bool isFrontGhost;
   bool isBackGhost;

   bool onFrontBoundary;
   bool onBackBoundary;
   bool onLeftBoundary;
   bool onRightBoundary;

   bool isBackLeft;
   bool isBackRight;
   bool isFrontLeft;
   bool isFrontRight;


   VecSet ( activeElementsVec, 0.0 );

   // First collect all active element data.
   for ( i = m_elementGrid.firstI (); i <= m_elementGrid.lastI (); ++i ) {

      for ( j = m_elementGrid.firstJ (); j <= m_elementGrid.lastJ (); ++j ) {

         // A element is active only if all its corner nodes are active.
         isActive = getNodeIsValid ( i, j );
         isActive = isActive and getNodeIsValid ( i + 1, j );
         isActive = isActive and getNodeIsValid ( i + 1, j + 1 );
         isActive = isActive and getNodeIsValid ( i, j + 1 );

#if 0
         isActive = nodeIsDefined ( i, j );
         isActive = isActive and nodeIsDefined ( i + 1, j );
         isActive = isActive and nodeIsDefined ( i + 1, j + 1 );
         isActive = isActive and nodeIsDefined ( i, j + 1 );
#endif

         activeElements ( j, i ) = ( isActive ? 1.0 : 0.0 );
         // Count the number of elements that are active in this partition.
         localActiveElementCount += ( isActive ? 1 : 0 );
      }

   }

   // Sum all the locally active element counts.
   MPI_Allreduce ( &localActiveElementCount, &globalActiveElementCount, 1, MPI_INT, MPI_SUM, PETSC_COMM_WORLD );
   // Are there are any active elements in the global 2d mesh.
   hasActiveElements = globalActiveElementCount > 0;

   activeElements.Restore_Global_Array ( Update_Including_Ghosts );
   activeElements.Set_Global_Array ( m_elementGrid.getDa (), activeElementsVec, INSERT_VALUES, true );

   // Next:
   //    i) set active map-elements;
   //   ii) set map-element position;
   //  iii) determine whether element lies on this processor or is a ghost element;
   //   iv) determine whether the boundaries of the element (0..3) are on the partition boundary;
   //    v) set the map-element node positions.
   //
   //
   // The on-X-boundary variable are for the processor boundary in this section
   for ( i = m_elementGrid.firstI ( true ); i <= m_elementGrid.lastI ( true ); ++i ) {

      onLeftBoundary = ( i == m_elementGrid.firstI ());
      onRightBoundary = ( i == m_elementGrid.lastI ());

      isLeftGhost = ( m_elementGrid.firstI ( true ) != m_elementGrid.firstI ( false ) and i == m_elementGrid.firstI ( true ));
      isRightGhost = ( m_elementGrid.lastI ( true ) != m_elementGrid.lastI ( false ) and i == m_elementGrid.lastI ( true ));

      for ( j = m_elementGrid.firstJ ( true ); j <= m_elementGrid.lastJ ( true ); ++j ) {

         onFrontBoundary = ( j == m_elementGrid.firstJ ());
         onBackBoundary = ( j == m_elementGrid.lastJ ());

         isFrontGhost = ( m_elementGrid.firstJ ( true ) != m_elementGrid.firstJ ( false ) and j == m_elementGrid.firstJ ( true ));
         isBackGhost = ( m_elementGrid.lastJ ( true ) != m_elementGrid.lastJ ( false ) and j == m_elementGrid.lastJ ( true ));

         isBackLeft = isLeftGhost and isBackGhost;
         isBackRight = isRightGhost and isBackGhost;
         isFrontLeft = isLeftGhost and isFrontGhost;
         isFrontRight = isRightGhost and isFrontGhost;

         if ( isBackLeft or isBackRight or isFrontLeft or isFrontRight ) {
            isActive = false;
         } else {
            // Check what happens on a partition that shares only a corner node.
            isActive = ( activeElements ( j, i ) > 0.0 );
         }

         m_mapElements ( i, j ).setIsValid ( isActive );
         m_mapElements ( i, j ).setPosition ( i, j );
         m_mapElements ( i, j ).setIsOnProcessor ( not ( isLeftGhost or isRightGhost or isFrontGhost or isBackGhost ));

         if ( m_mapElements ( i, j ).isOnProcessor ()) {
            m_mapElements ( i, j ).setNodePosition ( MapElement::Front, i,     j );
            m_mapElements ( i, j ).setNodePosition ( MapElement::Right, i + 1, j );
            m_mapElements ( i, j ).setNodePosition ( MapElement::Back,  i + 1, j + 1 );
            m_mapElements ( i, j ).setNodePosition ( MapElement::Left,  i,     j + 1 );

            m_mapElements ( i, j ).setIsOnProcessorBoundary ( MapElement::Front, onFrontBoundary );
            m_mapElements ( i, j ).setIsOnProcessorBoundary ( MapElement::Right, onRightBoundary );
            m_mapElements ( i, j ).setIsOnProcessorBoundary ( MapElement::Back,  onBackBoundary );
            m_mapElements ( i, j ).setIsOnProcessorBoundary ( MapElement::Left,  onLeftBoundary );

         }

      }

   }

   activeElements.Restore_Global_Array ( No_Update );
   VecDestroy ( &activeElementsVec );

   // Finally determine whether the elemenmt boundaries lie on the active domain boundary.
   //
   // The on-X-boundary variable are for the domain boundary in this section
   for ( i = m_elementGrid.firstI (); i <= m_elementGrid.lastI (); ++i ) {
      onLeftBoundary = ( m_elementGrid.firstI ( true ) == m_elementGrid.firstI ( false ) and i == m_elementGrid.firstI ());
      onRightBoundary = ( m_elementGrid.lastI ( true ) == m_elementGrid.lastI ( false )  and i == m_elementGrid.lastI ());

      for ( j = m_elementGrid.firstJ (); j <= m_elementGrid.lastJ (); ++j ) {
         onFrontBoundary = ( m_elementGrid.firstJ ( true ) == m_elementGrid.firstJ ( false ) and j == m_elementGrid.firstJ ());
         onBackBoundary = ( m_elementGrid.lastJ ( true ) == m_elementGrid.lastJ ( false ) and j == m_elementGrid.lastJ ());

         m_mapElements ( i, j ).setIsOnDomainBoundary ( MapElement::Front, onFrontBoundary or not m_mapElements ( i, j - 1 ).isValid ());
         m_mapElements ( i, j ).setIsOnDomainBoundary ( MapElement::Right, onRightBoundary or not m_mapElements ( i + 1, j ).isValid ());
         m_mapElements ( i, j ).setIsOnDomainBoundary ( MapElement::Back,  onBackBoundary  or not m_mapElements ( i, j + 1 ).isValid ());
         m_mapElements ( i, j ).setIsOnDomainBoundary ( MapElement::Left,  onLeftBoundary  or not m_mapElements ( i - 1, j ).isValid ());
      }

   }
}

void FastcauldronSimulator::printElementValidityMap(const std::string & fileName ) const
{
   char chars [2][2][2][2] = {{{{ '.', '.' }, { '.', '.' }}, {{ '.', '.' }, { '.', '.' }}},
                              {{{ '.', '.' }, { '.', '.' }}, {{ '.', '.' }, { '.', '.' }}}};

   // Top 
   chars [ 0 ][ 0 ][ 1 ][ 0 ] = '-';

   // Bottom
   chars [ 1 ][ 0 ][ 0 ][ 0 ] = '-';

   // Right
   chars [ 0 ][ 1 ][ 0 ][ 0 ] = '|';

   // Left
   chars [ 0 ][ 0 ][ 0 ][ 1 ] = '|';

   // Corners
   chars [ 1 ][ 1 ][ 0 ][ 0 ] = '+';
   chars [ 0 ][ 1 ][ 1 ][ 0 ] = '+';
   chars [ 0 ][ 0 ][ 1 ][ 1 ] = '+';
   chars [ 1 ][ 0 ][ 0 ][ 1 ] = '+';


   std::stringstream buffer;
   std::string line ( m_elementGrid.getNumberOfXElements (), '.' );
   std::string line2 ( m_elementGrid.getNumberOfXElements (), '.' );

   if ( getRank () == 0 ) {
      buffer << endl;
      buffer << " Map partitioning for project: " << getProjectName () << endl;
      buffer << " Number of processes         : " << getSize () << endl;
      buffer << endl;
   }

   buffer << " process " << getRank () << " element validity map "<< endl << endl;
   buffer << "     * - active element"  << endl
          << "     x - active ghost element " << endl
          << "     # - domain boundary element" << endl
          << "  +--+ - partition boundary " << endl
          << "     . - inactive element or on other partition" << endl << endl;

   for ( int j = m_elementGrid.lastJ () + 1; j < m_elementGrid.getNumberOfYElements (); ++j ) {
      buffer << line2 << endl;
   }

   for ( int j = m_elementGrid.lastJ ( true ); j >= m_elementGrid.firstJ ( true ); --j ) {

      for ( int i = m_elementGrid.firstI ( true ); i <= m_elementGrid.lastI ( true ); ++i ) {

         bool isActive = m_mapElements ( i, j ).isValid ();

         if ( not m_mapElements ( i, j ).isOnProcessor ()) {
            line.at ( i ) = ( isActive ? 'x' : '.' );
         } else {
            line.at ( i ) = ( isActive ? '*' : '.' );
         }

         if ( m_mapElements ( i, j ).isValid () and 
              ( m_mapElements ( i, j ).isOnDomainBoundary ( MapElement::Front ) or m_mapElements ( i, j ).isOnDomainBoundary ( MapElement::Right ) or
                m_mapElements ( i, j ).isOnDomainBoundary ( MapElement::Back  ) or m_mapElements ( i, j ).isOnDomainBoundary ( MapElement::Left ))) {
            line.at ( i ) = '#';
         } else if (( not m_mapElements ( i, j ).isValid ()) and m_mapElements ( i, j ).isOnProcessor ()) {
            line.at ( i ) = chars [ m_mapElements ( i, j ).isOnProcessorBoundary ( MapElement::Front )]
                                  [ m_mapElements ( i, j ).isOnProcessorBoundary ( MapElement::Right )]
                                  [ m_mapElements ( i, j ).isOnProcessorBoundary ( MapElement::Back  )]
                                  [ m_mapElements ( i, j ).isOnProcessorBoundary ( MapElement::Left  )];
         }

      }

      buffer << line;
      buffer << endl;
   }

   for ( int j = 0; j < m_elementGrid.firstJ () - 1; ++j ) {
      buffer << line2 << endl;
   }

   buffer << endl;
   buffer << endl;


   FILE* outputFile= 0;

   PetscFOpen ( PETSC_COMM_WORLD, fileName.c_str (), "w", &outputFile);
   PetscSynchronizedFPrintf ( PETSC_COMM_WORLD, outputFile, buffer.str ().c_str ());
   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
   PetscFClose ( PETSC_COMM_WORLD, outputFile );

   PetscPrintf ( PETSC_COMM_WORLD, " Saved unit test data in file: %s \n ", fileName.c_str ());
}


//------------------------------------------------------------//

const AppCtx* FastcauldronSimulator::getCauldron () const
{
   return m_cauldron;
}

//------------------------------------------------------------//

bool FastcauldronSimulator::setCalculationMode ( const CalculationMode mode)
{

   bool started = false;

   m_calculationMode = mode;

   switch ( mode ) {

      case HYDROSTATIC_DECOMPACTION_MODE :
         started = startActivity ( DecompactionRunStatusStr, getLowResolutionOutputGrid ());
         break;
      
      case HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE :
         started = startActivity ( HighResDecompactionRunStatusStr, getHighResolutionOutputGrid ());
         break;

      case HYDROSTATIC_TEMPERATURE_MODE :
         started = startActivity ( HydrostaticTemperatureRunStatusStr, getLowResolutionOutputGrid ());
         break;

      case OVERPRESSURE_MODE :
         started = startActivity ( OverpressureRunStatusStr, getLowResolutionOutputGrid ());
         break;

      case OVERPRESSURED_TEMPERATURE_MODE :
         started = startActivity ( OverpressuredTemperatureRunStatusStr, getLowResolutionOutputGrid ());
         break;

      case COUPLED_HIGH_RES_DECOMPACTION_MODE :
         started = startActivity ( HighResDecompactionRunStatusStr, getHighResolutionOutputGrid ());
         break;

      case PRESSURE_AND_TEMPERATURE_MODE :
         started = startActivity ( CoupledPressureTemperatureRunStatusStr, getLowResolutionOutputGrid ());
         break;

      case HYDROSTATIC_DARCY_MODE :
         started = startActivity ( HydrostaticDarcyRunStatusStr, getLowResolutionOutputGrid ());
         break;

      case COUPLED_DARCY_MODE :
         started = startActivity ( CoupledDarcyRunStatusStr, getLowResolutionOutputGrid ());
         break;

      case NO_CALCULATION_MODE :
         started = startActivity ( "NO_CALCULATION", getLowResolutionOutputGrid ());
         break;

      default :
         started = false;

   }

   if( not started ) {
      PetscPrintf ( PETSC_COMM_WORLD, " MeSsAgE ERROR Could not open the output file.\n");    
      return started;
   }
   setToConstantDensity ();

   // now that we have the calculation mode, we can initialise the property-constraints.
   m_propertyConstraints.initialise ( getCalculationMode (), getModellingMode ());

   started = started and GeoPhysics::ProjectHandle::initialise ( getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or
                                                                 getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE,
                                                                 true );
 
   if( GeoPhysics::ProjectHandle::determinePermafrost( m_cauldron->m_permafrostTimeSteps, m_cauldron->m_permafrostAges ) ) {

      m_cauldron->setPermafrost( );
      PetscPrintf ( PETSC_COMM_WORLD, "Permafrost is on.\n");
   }

   m_elementGrid.construct ( getActivityOutputGrid (), getRank ());
   m_nodalGrid.construct   ( getActivityOutputGrid (), getRank ());

   bool gridHasActiveElements = false;
   initialiseElementGrid ( gridHasActiveElements );

   if (m_cauldron->debug2)
   {
      std::stringstream fileNameBuffer;
      fileNameBuffer << getProjectName () << "_" << getSize () << "_element_activity_map.txt";
      printElementValidityMap( fileNameBuffer.str() );
   }

   if ( not gridHasActiveElements ) {
      PetscPrintf ( PETSC_COMM_WORLD, " MeSsAgE ERROR there are no active elements in the mesh.\n");
   }

   return started and gridHasActiveElements;
}

//------------------------------------------------------------//

void FastcauldronSimulator::updateSnapshotFileCreationFlags () {

   if ( getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE ) {
      if( !H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) {
         Interface::MutableSnapshotList::const_iterator snapshotIter;
         
         for ( snapshotIter = m_snapshots.begin (); snapshotIter != m_snapshots.end (); ++snapshotIter ) {
            
            if ( (*snapshotIter)->getFileName () != "" ) {
               (*snapshotIter)->setAppendFile ( File_Exists ( m_cauldron->getOutputDirectory () + (*snapshotIter)->getFileName ()));
            }
            
         } 
         
      }
   }
}

//------------------------------------------------------------//

void FastcauldronSimulator::setConstrainedOverpressureIntervals () {

   // The constrained overpressure should be added during formation (LayerProps) construction.
   Interface::MutableFormationList::iterator formationIter;
   Interface::ConstrainedOverpressureIntervalList::const_iterator copIter;

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      LayerProps* formation = dynamic_cast<LayerProps*>(*formationIter);

      Interface::ConstrainedOverpressureIntervalList* constrainedOverpressureList = getConstrainedOverpressureIntervalList ( formation );

      for ( copIter = constrainedOverpressureList->begin (); copIter != constrainedOverpressureList->end (); ++copIter ) {
         const Interface::ConstrainedOverpressureInterval* interval = *copIter;

         formation->setConstrainedOverpressureInterval ( interval->getStartAge ()->getTime (), 
                                                         interval->getEndAge ()->getTime (),
                                                         interval->getOverpressureValue ());

      }

      delete constrainedOverpressureList;
   }

}

//------------------------------------------------------------//

CalculationMode FastcauldronSimulator::getCalculationMode () const {
   return m_calculationMode;
}

//------------------------------------------------------------//

Interface::PropertyValue * FastcauldronSimulator::createMapPropertyValue ( const string &               propertyValueName,
                                                                           const Interface::Snapshot *  snapshot,
                                                                           const Interface::Reservoir * reservoir,
                                                                           const Interface::Formation * formation,
                                                                           const Interface::Surface *   surface ) {

   Interface::PropertyValue* propertyValue;

   propertyValue = Interface::ProjectHandle::createMapPropertyValue ( propertyValueName, snapshot, reservoir, formation, surface );
   m_propertyValuesForOutput.push_back ( propertyValue );

   return propertyValue;
}

//------------------------------------------------------------//

Interface::PropertyValue * FastcauldronSimulator::createVolumePropertyValue ( const string &               propertyValueName,
                                                                              const Interface::Snapshot *  snapshot,
                                                                              const Interface::Reservoir * reservoir,
                                                                              const Interface::Formation * formation,
                                                                                    unsigned int           depth ) {

   Interface::PropertyValue* propertyValue;

   propertyValue = Interface::ProjectHandle::createVolumePropertyValue ( propertyValueName, snapshot, reservoir, formation, depth );
   m_propertyValuesForOutput.push_back ( propertyValue );

   return propertyValue;
}

//------------------------------------------------------------//

FastcauldronSimulator::PropertyPartitioningPredicate::PropertyPartitioningPredicate ( const Interface::MutablePropertyValueList& list ) : m_list ( list ) {}


bool FastcauldronSimulator::PropertyPartitioningPredicate::operator ()( const Interface::PropertyValue* property ) const {
   // If item is not on the 'for deletion' list then return true.
   return std::find ( m_list.begin (), m_list.end (), property ) == m_list.end ();
}


void FastcauldronSimulator::deleteSnapshotProperties () {

   Interface::MutablePropertyValueList::iterator startOfItemsForDeletion;
   Interface::MutablePropertyValueList::iterator propertyIter;

   startOfItemsForDeletion = std::partition ( m_propertyValues.begin (), m_propertyValues.end (), PropertyPartitioningPredicate ( m_propertyValuesForOutput ));
   m_propertyValues.erase ( startOfItemsForDeletion, m_propertyValues.end ());

   // Now that the property-values have been removed from the m_propertyValues list, their maps should be deleted and the object deleted.
   for ( propertyIter = m_propertyValuesForOutput.begin (); propertyIter != m_propertyValuesForOutput.end (); ++propertyIter ) {

      if ((*propertyIter)->hasGridMap ()) {
         (*propertyIter)->getGridMap ()->release ();
      }

      delete (*propertyIter);
   }

   m_propertyValuesForOutput.clear ();

}

//------------------------------------------------------------//

void FastcauldronSimulator::deleteSnapshotPropertyValueMaps () {

   Interface::MutablePropertyValueList::iterator propertyIter;

   // Now that the property-values have been removed from the m_propertyValues list, their maps should be deleted and the object deleted.
   for ( propertyIter = m_propertyValuesForOutput.begin (); propertyIter != m_propertyValuesForOutput.end (); ++propertyIter ) {

      if ((*propertyIter)->hasGridMap ()) {
         (*propertyIter)->getGridMap ()->release ();
      }

   }

   m_propertyValuesForOutput.clear ();

}

//------------------------------------------------------------//

void FastcauldronSimulator::printSnapshotProperties () const {

   Interface::MutableSnapshotList::const_iterator snapshotIter;
   database::Table::iterator timeTableIter;
   database::Table* snapshotTable = getTable ( "SnapshotIoTbl" );

   std::vector<database::Record*> recordsForDeletion;

   assert ( snapshotTable != 0 );

   for ( timeTableIter = snapshotTable->begin (); timeTableIter != snapshotTable->end (); ++timeTableIter ) {
      cout << " snapshot table: " << (unsigned long)(*timeTableIter) << "  "
           << database::getTime ( *timeTableIter ) << endl;
   }


   for ( snapshotIter = m_minorSnapshots.begin (); snapshotIter != m_minorSnapshots.end (); ++snapshotIter ) {
      cout << " minor snapshots: " << (*snapshotIter)->getTime () << "  " << *snapshotIter << endl;
   }

}

//------------------------------------------------------------//

bool FastcauldronSimulator::nodeIsDefined ( const int i, const int j ) const {
   return m_cauldron->nodeIsDefined ( i, j );
}

//------------------------------------------------------------//
bool FastcauldronSimulator::mergeOutputFiles ( ) {

   if( ! H5_Parallel_PropertyList::isOneFilePerProcessEnabled() || 
       getModellingMode () == Interface::MODE1D ) {

      return true;
   }

#ifndef _MSC_VER
   PetscPrintf ( PETSC_COMM_WORLD, "Merging output files ...\n" ); 

   PetscBool noFileCopy = PETSC_FALSE;
   PetscLogDouble StartMergingTime;

   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );

   PetscTime(&StartMergingTime);
   bool status = true;
   
   const std::string& directoryName = getOutputDir ();
   
   if(  m_calculationMode != HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE && m_calculationMode != COUPLED_HIGH_RES_DECOMPACTION_MODE && 
        m_calculationMode != NO_CALCULATION_MODE ) {

      database::Table::iterator timeTableIter;
      database::Table* snapshotTable = getTable ( "SnapshotIoTbl" );
      
      assert ( snapshotTable != 0 );
      
      for ( timeTableIter = snapshotTable->begin (); timeTableIter != snapshotTable->end (); ++timeTableIter ) {
         
         string snapshotFileName = database::getSnapshotFileName ( *timeTableIter );
         
         if ( !snapshotFileName.empty() ) {
            string filePathName = getProjectPath () + "/" + directoryName + "/" + snapshotFileName;

            Display_Merging_Progress( snapshotFileName, StartMergingTime );

            if( m_calculationMode == OVERPRESSURED_TEMPERATURE_MODE ) {
               if( ! database::getIsMinorSnapshot ( *timeTableIter ) ) {                  
                  if( !mergeFiles ( allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), APPEND ))) {
                     status = false;
                     PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not merge the file %s.\n", filePathName.c_str() );               
                  } 
               }
            } else {
               if( !mergeFiles ( allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), CREATE ))) {
                  status = false;
                  PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not merge the file %s.\n", filePathName.c_str() );               
               } 
            }
         }
      }
   }
   string fileName = getActivityName () + "_Results.HDF" ; 
   string filePathName = getProjectPath () + "/" + directoryName + "/" + fileName;
   
   Display_Merging_Progress( fileName, StartMergingTime );

   status = mergeFiles (  allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), ( noFileCopy ? CREATE : REUSE )));

   if( !noFileCopy && status ) {
      status = H5_Parallel_PropertyList::copyMergedFile( filePathName );
   }
   if( status ) {
      if( m_fastcauldronSimulator->getRank () == 0 ) {
         displayTime( "Total merging time: ", StartMergingTime );
      }
    } else {
      PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not merge the file %s.\n", filePathName.c_str() );               
   }
   return status;
#else
   return true;
#endif
}

//------------------------------------------------------------//

void FastcauldronSimulator::finalise ( const bool saveResults ) {

   if ( m_fastcauldronSimulator != 0 ) {

      // Sort the snapshot-table before outputting project file.
      database::Table* snapshotTable = m_fastcauldronSimulator->getTable ( "SnapshotIoTbl" );
      snapshotTable->sort ( SnapshotRecordLess );

      m_fastcauldronSimulator->finishActivity ( saveResults );

      if ( saveResults and m_fastcauldronSimulator->getRank () == 0 and m_fastcauldronSimulator->getCalculationMode () != NO_CALCULATION_MODE ) {
         m_fastcauldronSimulator->setSimulationDetails ( "fastcauldron",
                                                         getSimulationModeString ( m_fastcauldronSimulator->getCalculationMode ()),
                                                         m_fastcauldronSimulator->m_commandLine );

         if ( m_fastcauldronSimulator->getModellingMode () == Interface::MODE1D ) 
         {
            m_fastcauldronSimulator->m_cauldron->writeIsoLinesToDatabase();
         }

         m_fastcauldronSimulator->saveToFile ( m_fastcauldronSimulator->m_cauldron->getFastCauldronProjectFileName ());
      }

      delete m_fastcauldronSimulator;
   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::correctTimeFilterDefaults () {

   if ( getModellingMode () == Interface::MODE1D ) {
      correctTimeFilterDefaults3D ();
   } else {
      correctTimeFilterDefaults3D ();
   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::correctTimeFilterDefaults1D () {
}

//------------------------------------------------------------//

void FastcauldronSimulator::correctTimeFilterDefaults3D () {

   // it is possible that the fault-elements property needs to be updated as well
   // but this cannot be done here, since we do not know at this stage if the 
   // project has falts or not.

   // Some properties may not be in the time-filter table, so they will have to be created.
   bool containsAllochthonous = false;
   bool containsErosionFactor = false;
   bool containsFCTCorrection = false;
   bool containsThicknessError = false;
   bool containsChemicalCompaction = false;
   bool containsHorizontalPermeability = false;
   bool containsLithologyId = false;
   bool containsBiomarkers = false;
   bool basementOutputRequested = false;
   bool outputALC = false;
   int  i;

   // Initialised to stop compiler from complaining. The initialisation is legitimate
   // because the variable is only assigned-to if containsBiomarkers is true, when this 
   // variable will be updated to the biomarkers output option value.
   Interface::PropertyOutputOption biomarkersOption = Interface::NO_OUTPUT;

   Interface::OutputProperty* newProperty;
   Interface::MutableOutputPropertyList::iterator propertyIter;

   // Any property that is mentioned explicitly in the loop here, is expected not to appear in the property-selection gui.
   // But is required for output or for the calculation of other properties to be output.
   for ( propertyIter = m_timeOutputProperties.begin (); propertyIter != m_timeOutputProperties.end (); ++propertyIter ) {

      Interface::OutputProperty * property = * propertyIter;

      const std::string& name = property->getName ();

      bool isOp = name == "OverPressure";

      if ( isOp ) {
         int var;
         var = 1;
      }

      if ( getModellingMode () == Interface::MODE1D and property->getOption () == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
         basementOutputRequested = true;
      }

      if ( name == "Biomarkers" ) {
         containsBiomarkers = true;
         biomarkersOption = property->getOption ();
      }

      if ( getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE and
           name == "LithoStaticPressure" and 
           property->getOption () == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
         property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
      }

      if ( CBMGenerics::ComponentManager::getInstance ().GetSpeciesIdByName ( name ) != -1 ) {
         property->setOption ( Interface::SOURCE_ROCK_ONLY_OUTPUT );
      }

#if 0
      if ( name == "C1ExpelledCumulative" ) {
         property->setOption ( Interface::SOURCE_ROCK_ONLY_OUTPUT );
      }
#endif

      // Even though this one is marked as a optional property it is in fact required.
      if ( name == "HcGasExpelledCumulative" ) {
         property->setOption ( Interface::SOURCE_ROCK_ONLY_OUTPUT );
      }

      // Even though this one is marked as a optional property it is in fact required.
      if ( name == "OilExpelledCumulative" ) {
         property->setOption ( Interface::SOURCE_ROCK_ONLY_OUTPUT );
      }

      if ( name == "AllochthonousLithology" ) {
         containsAllochthonous = true;
         property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
      }
          
      if ( name == "Lithology" ) {
         containsLithologyId = true;
         property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
      }

      if ( name == "FCTCorrection" ) {
         containsFCTCorrection = true;
         property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
      }
          
      if ( name == "ThicknessError" ) {
         containsThicknessError = true;
         property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
      }

      if ( name == "ErosionFactor" ) {
         containsErosionFactor = true;
         property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
      }
          
      if ( name == "ChemicalCompaction" ) {
         containsChemicalCompaction = true;
         if ( getRunParameters ()->getChemicalCompaction () ) {
            property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
         } else {
            property->setOption ( Interface::NO_OUTPUT );
         }
      }

      if ( name == "FracturePressure" ) {

         if ( getRunParameters ()->getFractureType () == "None" ) {
            property->setOption ( Interface::NO_OUTPUT );
         }

      }


      if ( name == "HorizontalPermeability" ) {
         containsHorizontalPermeability = true;
      }

      if ( name == "ALCStepTopBasaltDepth" || name == "ALCStepMohoDepth"   ) {
         if( isALC() ) {
            outputALC = true;
            property->setOption ( Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
         } else {
            property->setOption ( Interface::NO_OUTPUT );
         }
      }

      if( property->getOption () != Interface::NO_OUTPUT && 
          ( name == "ALCMaxAsthenoMantleDepth" || name == "ALCStepMohoDepth" || name == "ALCSmTopBasaltDepth" ||
            name == "ALCStepContCrustThickness" || name == "ALCStepBasaltThickness" || name == "ALCSmBasaltThickness" )) {
         if( isALC() ) {
            property->setOption ( Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
         } else {
            property->setOption ( Interface::NO_OUTPUT );
         }
      }
      if( property->getOption () != Interface::NO_OUTPUT && 
          ( name == "ALCOrigLithMantleDepth" || name == "ALCSmContCrustThickness" ||
            name == "ALCSmTopBasaltDepth" || name == "ALCSmMohoDepth" )) {
         if( isALC() ) {
            property->setOption ( Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
         } else {
            property->setOption ( Interface::NO_OUTPUT );
         }
      }

      if ( name == "FaultElements" ) {

         if ( getBasinHasActiveFaults ()) {
            property->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
         } else {
            property->setOption ( Interface::NO_OUTPUT );
         } 

      }
          
      const PropertyList propertyListValue = getPropertyList ( name );
      Interface::PropertyOutputOption option = property->getOption ();

      if ( propertyListValue >= DIFFUSIVITYVEC and propertyListValue < ENDPROPERTYLIST and getModellingMode () == property->getMode ()) {
         m_propertyConstraints.constrain ( propertyListValue, option );
         property->setOption ( option );

      }

   }

   if ( basementOutputRequested ) { // and not depthOutputIncludesBasement
      /// Must now enable depth output for basement, if it is not already on.

      for ( propertyIter = m_timeOutputProperties.begin (); propertyIter != m_timeOutputProperties.end (); ++propertyIter ) {
         Interface::OutputProperty* property = *propertyIter;

         if ( property->getName () == "Depth" ) {
            // only need to set the depth if it is not set already.
            property->setOption ( Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
            break;
         }

      }

   }
   if( not outputALC and isALC() ) {
      Interface::OutputProperty* property = getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, "ALCStepTopBasaltDepth" );
      property->setOption ( Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
      m_timeOutputProperties.push_back ( property );

      property = getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, "ALCStepMohoDepth" );
      property->setOption ( Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
      m_timeOutputProperties.push_back (property );
   }

   // Properties that were not found in the original list must be added, since they 
   // are either output or used in other derived properties that are output.
   if ( not containsAllochthonous and getModellingMode () == Interface::MODE3D ) {
      m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT, "AllochthonousLithology" ));
   } 

   if ( not containsErosionFactor ) {
      m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT, "ErosionFactor" ));
   }

   if ( not containsFCTCorrection ) {
      m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT, "FCTCorrection" ));
   } 

   //for ( i = 0; i < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; ++i ) {
   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      newProperty = getFactory ()->produceOutputProperty (this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT,
                                                          CBMGenerics::ComponentManager::getInstance ().GetSpeciesName ( i ) + "Concentration" );
      // newProperty = getFactory ()->produceOutputProperty (this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT,
      //                                                     pvtFlash::ComponentIdNames [ i ] + "Concentration" );
      newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
      m_timeOutputProperties.push_back ( newProperty );
   }


   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "BrineSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "ImmobileSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );


  
   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "AverageBrineSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "AverageHcLiquidSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "AverageHcVapourSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "AverageImmobileSaturation" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   
   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidBrineCapillaryPressure" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );


   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourBrineCapillaryPressure" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "GOR" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "CGR" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "OilAPI" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "CondensateAPI" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "BrineDensity" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "BrineViscosity" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "TimeOfInvasion" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );
   
   
   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourDensity" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidDensity" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );


   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourViscosity" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidViscosity" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourVolume" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidVolume" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "ElementVolume" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "ElementPoreVolume" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );


   // Add the property name for the mass that is transported over the snapshot interval
   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "TransportedMass" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "BrineRelativePermeability" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidRelativePermeability" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourRelativePermeability" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );


   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourVelocityX" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourVelocityY" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourVelocityZ" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcVapourVelocityMagnitude" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );


   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidVelocityX" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidVelocityY" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidVelocityZ" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );

   newProperty = getFactory ()->produceOutputProperty ( this, getModellingMode (),
                                                        Interface::SEDIMENTS_ONLY_OUTPUT,
                                                        "HcLiquidVelocityMagnitude" );
   newProperty->setOption ( Interface::SEDIMENTS_ONLY_OUTPUT );
   m_timeOutputProperties.push_back ( newProperty );



   if ( not containsChemicalCompaction ) {
      if ( getRunParameters ()->getChemicalCompaction () ) {
         m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT, "ChemicalCompaction" ));
      } else {
         m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::NO_OUTPUT, "ChemicalCompaction" ));
      }
   } 

   if ( not containsThicknessError ) {
      m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT, "ThicknessError" ));
   } 

   if ( not containsLithologyId and getModellingMode () == Interface::MODE3D ) {
      m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), Interface::SEDIMENTS_ONLY_OUTPUT, "Lithology" ));
   }

   if ( not containsHorizontalPermeability ) {
      const Interface::OutputProperty* permeability = findTimeOutputProperty ( "PermeabilityVec" );
      Interface::PropertyOutputOption option = ( permeability == 0 ? Interface::NO_OUTPUT : permeability->getOption ());

      m_timeOutputProperties.push_back ( getFactory ()->produceOutputProperty ( this, getModellingMode (), option, "HorizontalPermeability" ));
   }

   using namespace CBMGenerics;
   ComponentManager & theComponentManager = ComponentManager::getInstance();
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   // The genex species output names do not appear in the project file filter-time-io table.
   // So create them here.
   for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      m_timeOutputProperties.push_back (getFactory ()->produceOutputProperty ( this,
                                                                               getModellingMode (),
                                                                               Interface::SOURCE_ROCK_ONLY_OUTPUT,
                                                                               theComponentManager.GetSpeciesOutputPropertyName( i )));

   }

   for (i = 0; i < GenexResultManager::NumberOfResults; ++i)
   {

      const string name = theResultManager.GetResultName ( i );

      const Interface::OutputProperty* prop = findTimeOutputProperty ( name );

      if ( prop != 0 ) {

         if ( prop->getOption () != Interface::NO_OUTPUT ) {
            theResultManager.SetResultToggleByResId ( i , true );
         }

      }

   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::setToConstantDensity () {

   Interface::MutableFluidTypeList::iterator fluidIter;

   if ( getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE or
        getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE or
        getCalculationMode () == HYDROSTATIC_TEMPERATURE_MODE  ) {

      for ( fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end (); ++fluidIter ) {
         FluidType* fluid = (FluidType*)(*fluidIter);
         fluid->setDensityToConstant ();
      }

   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::correctAllPropertyLists () {
   correctTimeFilterDefaults ();
   connectOutputProperties ();
  
}

//------------------------------------------------------------//

void FastcauldronSimulator::deleteMinorSnapshots () {

   Interface::MutableSnapshotList::iterator snapshotIter;
   database::Table::iterator timeTableIter;
   database::Table* snapshotTable = getTable ( "SnapshotIoTbl" );

   std::vector<database::Record*> recordsForDeletion;
   std::vector<database::Record*>::iterator recordsForDeletionIter;

   assert ( snapshotTable != 0 );

   for ( snapshotIter = m_minorSnapshots.begin (); snapshotIter != m_minorSnapshots.end (); ++snapshotIter ) {

      for ( timeTableIter = snapshotTable->begin (); timeTableIter != snapshotTable->end (); ++timeTableIter ) {

         if ((*snapshotIter)->getTime () == database::getTime ( *timeTableIter )) {
            recordsForDeletion.push_back ( *timeTableIter );
            break;
         }

      }

      if( ! H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) {
         if ( getRank () == 0 ) {
            const std::string fileName = getFullOutputDir () + "/" + (*snapshotIter)->getFileName ();
            int status = std::remove( fileName.c_str ());
            
            if (status == -1)
               cerr << "MeSsAgE WARNING  Unable to remove minor snapshot file, because '" 
                    << std::strerror(errno) << "'" << endl;
         }  
      }
   }

   for ( recordsForDeletionIter = recordsForDeletion.begin (); recordsForDeletionIter != recordsForDeletion.end (); ++recordsForDeletionIter ) {
      snapshotTable->deleteRecord ( *recordsForDeletionIter );
   }

   m_minorSnapshots.clear ();

}

//------------------------------------------------------------//

void FastcauldronSimulator::deleteMinorSnapshotsFromSnapshotTable () {

   database::Table::iterator timeTableIter;
   database::Table* snapshotTable = getTable ( "SnapshotIoTbl" );

   std::vector<database::Record*> recordsForDeletion;
   std::vector<database::Record*>::iterator recordsForDeletionIter;

   assert ( snapshotTable != 0 );

   for ( timeTableIter = snapshotTable->begin (); timeTableIter != snapshotTable->end (); ++timeTableIter ) {

      if ( database::getIsMinorSnapshot ( *timeTableIter )) {
         recordsForDeletion.push_back ( *timeTableIter );
      }

   }

   for ( recordsForDeletionIter = recordsForDeletion.begin (); recordsForDeletionIter != recordsForDeletion.end (); ++recordsForDeletionIter ) {
      snapshotTable->deleteRecord ( *recordsForDeletionIter );
   }

}

//------------------------------------------------------------//

const Interface::Snapshot* FastcauldronSimulator::findOrCreateSnapshot ( const double time ) {

   const double tolerance = 1.0e-6;

   database::Table::iterator ssIter;
   database::Table* ssTable = getTable ( "SnapshotIoTbl" );

   for ( ssIter = ssTable->begin (); ssIter != ssTable->end (); ++ssIter ) {

      if ( std::fabs ( database::getTime ( *ssIter ) - time ) < tolerance ) {
         return findSnapshot ( time, Interface::MAJOR | Interface::MINOR );
      }

   }

   // If we reach this point then the corresponding snapshot has not been found.

   database::Record* snapshotRecord = ssTable->createRecord ();

   setTime ( snapshotRecord, time );

   // Since it is only the minor snapshots that will be deleted,
   // major snapshots will always be a part of the snapshot-table.
   // So any created snapshot is a minor-snapshot.
   setIsMinorSnapshot  ( snapshotRecord, 1 );
   setTypeOfSnapshot   ( snapshotRecord, "System Generated");
   setSnapshotFileName ( snapshotRecord, "" );

   m_minorSnapshots.push_back (getFactory ()->produceSnapshot (this, snapshotRecord));
   m_snapshots.push_back ( m_minorSnapshots [ m_minorSnapshots.size () - 1 ]);

   // return the snapshot that was just created.
   return m_minorSnapshots [ m_minorSnapshots.size () - 1 ];

}

#if 0
const Interface::Snapshot* FastcauldronSimulator::findOrCreateSnapshot ( const double time ) {

   const double tolerance = 1.0e-6;

   const Interface::Snapshot* closestSnapshot = findSnapshot ( time, Interface::MAJOR | Interface::MINOR );

   if ( std::fabs ( closestSnapshot->getTime () - time ) < tolerance ) {
//       cout << " found snapshot: " << time << "   " << closestSnapshot->getTime () << endl;
      return closestSnapshot;
   } else {
//       cout << " creating snapshot " << time << endl;

      //add entry to SnapshotIoTbl
      database::Table *snapshotIoTbl = getTable ("SnapshotIoTbl");
      assert (snapshotIoTbl);
      database::Record* snapshotRecord = snapshotIoTbl->createRecord();

      setTime             ( snapshotRecord, time );

      // Major snapshots should always be a part of the snapshot-table.
      // So any created snapshot is a minor-snapshot.
      setIsMinorSnapshot  ( snapshotRecord, 1 );
      setTypeOfSnapshot   ( snapshotRecord, "System Generated");
      setSnapshotFileName ( snapshotRecord, "" );


      m_minorSnapshots.push_back (getFactory ()->produceSnapshot (this, snapshotRecord));
      m_snapshots.push_back ( m_minorSnapshots [ m_minorSnapshots.size () - 1 ]);

//       cout << " creating snapshot: " << time << "  " << (unsigned long)(m_minorSnapshots [ m_minorSnapshots.size () - 1 ]) << endl;

      // return the snapshot that was just created.
      return m_minorSnapshots [ m_minorSnapshots.size () - 1 ];
   }

}
#endif

//------------------------------------------------------------//

void FastcauldronSimulator::saveSourceRockProperties ( const Interface::Snapshot* snapshot,
                                                       const PropListVec&         genexProperties,
                                                       const PropListVec&         shaleGasProperties ) {

   // Compute all required properties.
   PropertyManager::getInstance ().computeSourceRockPropertyMaps ( m_cauldron, snapshot, genexProperties, shaleGasProperties );

   // Save properties to disk.
   continueActivity ();

   // Remove property calculators.
   PropertyManager::getInstance ().clear ();

   // Delete the output snapshot property-values.

#if 0
   deleteSnapshotProperties ();
#endif

   deleteSnapshotPropertyValueMaps ();

}

//------------------------------------------------------------//

void FastcauldronSimulator::saveMapProperties ( const PropListVec&                    requiredProperties,
                                                const Interface::Snapshot*            snapshot,
                                                const Interface::PropertyOutputOption maximumOutputOption ) {

   // Compute all required properties.
   PropertyManager::getInstance ().computePropertyMaps ( m_cauldron, requiredProperties, snapshot, maximumOutputOption );

   // Save properties to disk.
   continueActivity ();

   // Remove property calculators.
   PropertyManager::getInstance ().clear ();

   // Delete the output snapshot property-values.

#if 0
   deleteSnapshotProperties ();
#endif

   deleteSnapshotPropertyValueMaps ();

}

//------------------------------------------------------------//

void FastcauldronSimulator::saveVolumeProperties ( const PropListVec&                    requiredProperties,
                                                   const Interface::Snapshot*            snapshot,
                                                   const Interface::PropertyOutputOption maximumOutputOption ) {


   // Compute all required properties.
   PropertyManager::getInstance ().computePropertyVolumes ( m_cauldron, requiredProperties, snapshot, maximumOutputOption );

   // Save properties to disk.
   continueActivity ();

   // Remove property calculators.
   PropertyManager::getInstance ().clear ();

   // Delete the output snapshot property-values.
   deleteSnapshotPropertyValueMaps ();

#if 0
   deleteSnapshotProperties ();
#endif

}

//------------------------------------------------------------//

void FastcauldronSimulator::saveProperties ( const PropListVec&                    mapProperties,
                                             const PropListVec&                    volumeProperties,
                                             const Interface::Snapshot*            snapshot,
                                             const Interface::PropertyOutputOption maximumOutputOption ) {

   // Compute all required properties.
   PropertyManager::getInstance ().computeProperties ( m_cauldron, mapProperties, volumeProperties, snapshot, maximumOutputOption );

   // Save properties to disk.
   continueActivity ();

   // Remove property calculators.
   PropertyManager::getInstance ().clear ();

   // Delete the output snapshot property-values.
   deleteSnapshotPropertyValueMaps ();

#if 0
   deleteSnapshotProperties ();
#endif

}

//------------------------------------------------------------//

database::Record* FastcauldronSimulator::findTimeIoRecord ( database::Table*   timeIoTbl,
                                                            const std::string& propertyName,
                                                            const double       time,
                                                            const std::string& surfaceName, 
                                                            const std::string& formationName ) const {


   database::Table::iterator tblIter;

   for (tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++tblIter)
   {
      if ( database::getTime (*tblIter) == time ) {
         if( database::getPropertyName (*tblIter) == propertyName and
             database::getSurfaceName (*tblIter) == surfaceName and
             database::getFormationName (*tblIter) == formationName)
         {
            return *tblIter;
         }
      }

   }

   return 0;
}

//------------------------------------------------------------//

const Interface::OutputProperty* FastcauldronSimulator::findOutputProperty ( const Interface::Property* property ) const {

   const Interface::OutputProperty* result = 0;

   const std::string& outputPropertyName = PropertyManager::getInstance ().findOutputPropertyName ( property->getName ());

   if ( outputPropertyName != "" ) {
      result = findTimeOutputProperty ( outputPropertyName );
   } else {
      result = 0;
   }

   return result;
}

//------------------------------------------------------------//

void FastcauldronSimulator::setOutputPropertyOption ( const PropertyList                    property,
                                                      const Interface::PropertyOutputOption option ) {

   const std::string& propertyName = propertyListName ( property );

   Interface::MutableOutputPropertyList::iterator propertyIter;
   Interface::OutputProperty* outputProperty = 0;

   for ( propertyIter = m_timeOutputProperties.begin (); propertyIter != m_timeOutputProperties.end (); ++propertyIter ) {

      if ( (*propertyIter)->getName () == propertyName ) {
         outputProperty = *propertyIter;
         break;
      }

   }

   if ( outputProperty != 0 ) {
      outputProperty->setOption ( option );
   }
   
}

//------------------------------------------------------------//

void FastcauldronSimulator::connectOutputProperties () {

   Interface::MutablePropertyList::iterator propertyIter;

   for ( propertyIter = m_properties.begin (); propertyIter != m_properties.end (); ++propertyIter ) {
      Property* property = (Property*)(*propertyIter);
      property->setOutputProperty ( findOutputProperty ( *propertyIter ));
//       property->printOn ( std::cout );
   }

}

//------------------------------------------------------------//

LayerProps* FastcauldronSimulator::findLayer ( const std::string& layerName ) const {
   return m_cauldron->findLayer ( layerName );
}

//------------------------------------------------------------//

int FastcauldronSimulator::DACreate2D ( DM& theDA ) {

   int ierr;

   ierr = DMDACreate2d ( PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                         getInstance ().getActivityOutputGrid ()->numIGlobal (),
                         getInstance ().getActivityOutputGrid ()->numJGlobal (),
                         getInstance ().getActivityOutputGrid ()->numProcsI (),
                         getInstance ().getActivityOutputGrid ()->numProcsJ (),
                         1, 1,
                         getInstance ().getActivityOutputGrid ()->numsI (),
                         getInstance ().getActivityOutputGrid ()->numsJ (),
                         &theDA );
   
   return ierr;
}

int FastcauldronSimulator::DACreate3D ( const int numberOfZNodes,
                                        DM& theDA ) {

   int ierr;

   ierr = DMDACreate3d ( PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                         getInstance ().getActivityOutputGrid ()->numIGlobal (),
                         getInstance ().getActivityOutputGrid ()->numJGlobal (),
                         numberOfZNodes,
                         getInstance ().getActivityOutputGrid ()->numProcsI (),
                         getInstance ().getActivityOutputGrid ()->numProcsJ (),
                         1, 1, 1, 
                         getInstance ().getActivityOutputGrid ()->numsI (),
                         getInstance ().getActivityOutputGrid ()->numsJ (),
                         PETSC_NULL,
                         &theDA );

   return ierr;
}

//------------------------------------------------------------//

bool FastcauldronSimulator::saveCreatedVolumePropertyValues () {

   return Interface::ProjectHandle::saveCreatedVolumePropertyValues ();
}

//------------------------------------------------------------//

void FastcauldronSimulator::addLateralStressInterpolator ( ibs::Interpolator* interp ) {
   m_lateralStressInterpolator = interp;
}

//------------------------------------------------------------//

MultiComponentFlowHandler& FastcauldronSimulator::getMcfHandler () {
   return *m_mcfHandler;
}

//------------------------------------------------------------//

void FastcauldronSimulator::setRelativePermeabilityType ( const RelativePermeabilityType relPerm ) {
   m_relativePermeabilityType = relPerm;
}

//------------------------------------------------------------//

void FastcauldronSimulator::readCommandLineParametersEarlyStage( const int argc, char **argv ) {

   // Should move all command line parameters from appctx to fastcauldron-simulator.

   PetscBool fctScalingChanged;
   double    fctScaling;
   PetscBool hasPrintCommandLine; 
   PetscBool computeCapillaryPressure;

   PetscOptionsHasName ( PETSC_NULL, "-printcl", &hasPrintCommandLine );
   PetscOptionsGetReal  ( PETSC_NULL, "-glfctweight", &fctScaling, &fctScalingChanged );
   PetscOptionsHasName ( PETSC_NULL, "-fcpce", &computeCapillaryPressure );

   if ( fctScalingChanged ) {
      m_fctCorrectionScalingWeight = NumericFunctions::clipValueToRange ( fctScaling, 0.0, 1.0 );
   }

   m_printCommandLine = hasPrintCommandLine or m_cauldron->debug1 or m_cauldron->verbose;
   m_computeCapillaryPressure = computeCapillaryPressure == PETSC_TRUE;

   if ( getModellingMode () != Interface::MODE1D ) {
      H5_Parallel_PropertyList::setOneFilePerProcessOption ();
   }

   for ( int i = 1; i < argc; ++i ) {
      m_commandLine += std::string ( argv [ i ]) + ( i == argc - 1 ? "" : " " );
   }

   H5_Parallel_PropertyList::setOneNodeCollectiveBufferingOption();
}

//------------------------------------------------------------//
void FastcauldronSimulator::deleteTemporaryDirSnapshots()
{
   if( getCauldron ()->getCalculationMode() == OVERPRESSURED_TEMPERATURE_MODE && H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) {

      database::Table::iterator timeTableIter;
      database::Table* snapshotTable = getTable ( "SnapshotIoTbl" );
      
      assert ( snapshotTable != 0 );
      
      for ( timeTableIter = snapshotTable->begin (); timeTableIter != snapshotTable->end (); ++timeTableIter ) {
         
         string snapshotFileName = database::getSnapshotFileName ( *timeTableIter );
         if ( !snapshotFileName.empty() && ! database::getIsMinorSnapshot ( *timeTableIter ) ) {
            std::stringstream filePath;
            filePath << H5_Parallel_PropertyList::getTempDirName() << "/" << getProjectPath () << "/" <<  getOutputDir() << "/" << snapshotFileName << "_" << PetscGlobalRank;
            std::remove ( filePath.str().c_str ());            
         }
      }
   }
}

//------------------------------------------------------------//
void FastcauldronSimulator::readCommandLineParametersLateStage( const int argc, char ** argv)
{
   m_cauldron->setAdditionalCommandLineParameters ();
   readRelPermCommandLineParameters ();
   readCommandLineWells ();
   printCommandLine ( argc, argv );
}

//------------------------------------------------------------//

void FastcauldronSimulator::readCommandLineWells () {

   const int MaximumNumberOfPseudoWells = 200;

   PetscBool pseudoWellIndexInput = PETSC_FALSE;
   int pseudoWellIndices [ MaximumNumberOfPseudoWells ];
   int numberOfWellIndicesInput = MaximumNumberOfPseudoWells;

   PetscBool pseudoWellLocationInput = PETSC_FALSE;
   double pseudoWellLocations [ MaximumNumberOfPseudoWells ];
   int numberOfWellLocationsInput = MaximumNumberOfPseudoWells;

   int i;

   PetscOptionsGetIntArray ( PETSC_NULL, "-fcwellindex", pseudoWellIndices, &numberOfWellIndicesInput, &pseudoWellIndexInput );
   PetscOptionsGetRealArray ( PETSC_NULL, "-fcwelllocation", pseudoWellLocations, &numberOfWellLocationsInput, &pseudoWellLocationInput );

   if ( pseudoWellIndexInput ) {

      if ( numberOfWellIndicesInput % 2 != 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " ERROR the number of well indices is not divisible by 2 (a set of i,j positions)." );
      } else {
         int indexI;
         int indexJ;

         for ( i = 0; i < numberOfWellIndicesInput; i += 2 ) {
            indexI = pseudoWellIndices [ i ];
            indexJ = pseudoWellIndices [ i + 1 ];
            m_cauldron->setRelatedProject ( indexI, indexJ );
         }

      }
      
   }

   if ( pseudoWellLocationInput ) {

      if ( numberOfWellLocationsInput % 2 != 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " ERROR the number of well locations is not divisible by 2 (a set of x,y positions)." );
      } else {
         double locationX;
         double locationY;

         for ( i = 0; i < numberOfWellLocationsInput; i += 2 ) {
            locationX = pseudoWellLocations [ i ];
            locationY = pseudoWellLocations [ i + 1 ];
            m_cauldron->setRelatedProject ( locationX, locationY );
         }

      }
      
   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::printCommandLine ( const int argc, char **argv ) {

   if ( m_printCommandLine and getRank () == 0 ) {
      int i;

      PetscPrintf ( PETSC_COMM_WORLD, "\n\n The command line:\n");

      for ( i = 0; i < argc; ++i ) {
         PetscPrintf ( PETSC_COMM_WORLD, " %s", argv [ i ]);
      }

      PetscPrintf ( PETSC_COMM_WORLD, "\n\n");
   }


}

//------------------------------------------------------------//

void FastcauldronSimulator::setBrineViscosities ( const Interface::ViscosityModel model ) {

   Interface::MutableFluidTypeList::iterator fluidIter;

   for ( fluidIter = m_fluidTypes.begin (); fluidIter != m_fluidTypes.end (); ++fluidIter ) {
      (*fluidIter)->setViscosityModel ( model );
   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::readRelPermCommandLineParameters () {

   // PetscBool relPermMethodDescribed = PETSC_FALSE;
   // char relPermMethodName [ MAXLINESIZE ];

   PetscBool minimumHcSaturationChanged;
   double     minimumHcSaturation;

   PetscBool minimumWaterSaturationChanged;
   double     minimumWaterSaturation;

   PetscBool waterCurveExponentChanged;
   double     waterCurveExponent;

   PetscBool hcCurveExponentChanged;
   double     hcCurveExponent;

   PetscBool hcLiquidCurveExponentChanged;
   double     hcLiquidCurveExponent;

   PetscBool hcVapourCurveExponentChanged;
   double     hcVapourCurveExponent;

   PetscBool useTemisPackViscosities;

   // PetscOptionsGetString ( PETSC_NULL, "-relperm", relPermMethodName, MAXLINESIZE, &relPermMethodDescribed );
   PetscOptionsGetReal   ( PETSC_NULL, "-minhcsat", &minimumHcSaturation, &minimumHcSaturationChanged );
   PetscOptionsGetReal   ( PETSC_NULL, "-minwatersat", &minimumWaterSaturation, &minimumWaterSaturationChanged );
   PetscOptionsGetReal   ( PETSC_NULL, "-waterexpo", &waterCurveExponent, &waterCurveExponentChanged );
   PetscOptionsGetReal   ( PETSC_NULL, "-hcexpo", &hcCurveExponent, &hcCurveExponentChanged );

   PetscOptionsGetReal   ( PETSC_NULL, "-hcvapexpo", &hcVapourCurveExponent, &hcVapourCurveExponentChanged );
   PetscOptionsGetReal   ( PETSC_NULL, "-hcliqexpo", &hcLiquidCurveExponent, &hcLiquidCurveExponentChanged );

   PetscOptionsHasName   ( PETSC_NULL, "-temisviscosity", &useTemisPackViscosities );


   // if ( relPermMethodDescribed ) {
   //    RelativePermeabilityType relativePermeabilityFunction;

   //    relativePermeabilityFunction = RelativePermeabilityTypeValue ( relPermMethodName );

   //    if ( relativePermeabilityFunction == UNKNOWN_RELATIVE_PERMEABILITY_FUNCTION ) {
   //       relativePermeabilityFunction = DefaultRelativePermeabilityFunction;
   //    }

   //    setRelativePermeabilityType ( relativePermeabilityFunction );
   // }

   if ( minimumHcSaturationChanged ) {
      m_minimumHcSaturation = minimumHcSaturation;
   }

   if ( minimumWaterSaturationChanged ) {
      m_minimumWaterSaturation = minimumWaterSaturation;
   }

   if ( waterCurveExponentChanged ) {
      m_waterCurveExponent = waterCurveExponent;
   }

   if ( hcCurveExponentChanged ) {
      m_hcVapourCurveExponent = hcCurveExponent;
      m_hcLiquidCurveExponent = hcCurveExponent;
   }

   if ( hcVapourCurveExponentChanged ) {
      m_hcVapourCurveExponent = hcVapourCurveExponent;
   }

   if ( hcLiquidCurveExponentChanged ) {
      m_hcLiquidCurveExponent = hcLiquidCurveExponent;
   }

   if ( useTemisPackViscosities ) {
      setBrineViscosities ( DataAccess::Interface::TEMIS_PACK_VISCOSITY );
   }

}


//------------------------------------------------------------//


bool FastcauldronSimulator::checkMobileLayerThicknesses () const {

   size_t i;

   bool ret = true;
   for ( i = 0; i < m_cauldron->layers.size (); ++i ) {
      LayerProps* layer = m_cauldron->layers [ i ];

      if ( layer->isMobile () and layer->getMinimumThickness () < -0.01 ) {
	 if (getRank () == 0)
	 {
	    cerr << "MeSsAgE ERROR mobile layer " << layer->layername << " has (too large) negative thicknesses" << endl;
	 }
         ret = false;
      }
   }

   return ret;
}

//------------------------------------------------------------//

void FastcauldronSimulator::updateSourceRocksForGenex () {

   DataAccess::Interface::MutableSourceRockList::iterator srIter;

   for ( srIter = m_sourceRocks.begin (); srIter != m_sourceRocks.end (); ++srIter ) {
      GeoPhysics::GeoPhysicsSourceRock* sourceRock = dynamic_cast<GeoPhysics::GeoPhysicsSourceRock*>(*srIter);

      sourceRock->addHistoryToNodes ();
   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::saveGenexHistory () {

   DataAccess::Interface::MutableSourceRockList::iterator srIter;

   for ( srIter = m_sourceRocks.begin (); srIter != m_sourceRocks.end (); ++srIter ) {
      GeoPhysics::GeoPhysicsSourceRock* sourceRock = dynamic_cast<GeoPhysics::GeoPhysicsSourceRock*>(*srIter);

      sourceRock->saveSourceRockNodeAdsorptionHistory ();
   }

}

//------------------------------------------------------------//

void FastcauldronSimulator::updateSourceRocksForDarcy () {

   if ( not getMcfHandler ().solveFlowEquations ()) {
      // If not doing a Darcy run then no need to disable adsorption.
      return;
   }

   DataAccess::Interface::MutableSourceRockList::iterator srIter;

   for ( srIter = m_sourceRocks.begin (); srIter != m_sourceRocks.end (); ++srIter ) {
      GeoPhysics::GeoPhysicsSourceRock* sourceRock = dynamic_cast<GeoPhysics::GeoPhysicsSourceRock*>(*srIter);

      sourceRock->setCanIncludeAdsorption ( false );
   }

}
