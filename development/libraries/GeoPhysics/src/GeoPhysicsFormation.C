// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GeoPhysicsFormation.h"

#include <cmath>
#include <sstream>

#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/PropertyValue.h"
#include "Interface/Surface.h"
#include "Interface/MobileLayer.h"
#include "Interface/auxiliaryfaulttypes.h"
#include "Interface/Faulting.h"
#include "Interface/FaultCollection.h"
#include "Interface/FaultElementCalculator.h"
#include "Interface/RunParameters.h"

#include "GeoPhysicsProjectHandle.h"
#include "CompoundLithologyComposition.h"
#include "LithologyManager.h"

#include "NumericFunctions.h"

using namespace DataAccess;

//------------------------------------------------------------//

GeoPhysics::Formation::Formation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                   database::Record*                          record ) : 
   DataAccess::Interface::Formation ( projectHandle, record )
{

   m_minimumDepositedThickness = Interface::DefaultUndefinedMapValue;
   m_maximumDepositedThickness = Interface::DefaultUndefinedMapValue;
   m_containsFault = false;

   if ( kind () == DataAccess::Interface::SEDIMENT_FORMATION ) {
      m_zRefinementFactor = getElementRefinement ();
   } else {
      m_zRefinementFactor = 1;
   }

}

//------------------------------------------------------------//

GeoPhysics::Formation::~Formation () {
}

//------------------------------------------------------------//

bool GeoPhysics::Formation::setLithologiesFromStratTable () {

   const double LithologyTolerance = 1.0e-4;

   m_compoundLithologies.allocate ( m_projectHandle->getActivityOutputGrid ());

   const Interface::GridMap* lithoMap1 = dynamic_cast<const Interface::GridMap*>(getLithoType1PercentageMap ());
   const Interface::GridMap* lithoMap2 = dynamic_cast<const Interface::GridMap*>(getLithoType2PercentageMap ());
   const Interface::GridMap* lithoMap3 = dynamic_cast<const Interface::GridMap*>(getLithoType3PercentageMap ());

   string lithoName1;
   string lithoName2;
   string lithoName3;

   double lithologyPercentage1;
   double lithologyPercentage2;
   double lithologyPercentage3;

   double minimumPercentage;
   double maximumPercentage;
   double percentageSum;

   bool createLithoFromMaps = false;
   bool noDefinedLithologyValue;
   bool createdLithologies = true;

   std::string errorMessage;

   CompoundLithology*  pMixedLitho;

   // Load the lithology maps.
   if ( lithoMap1 != 0 ) {
      lithoMap1->retrieveGhostedData ();
   }

   if ( lithoMap2 != 0 ) {
      lithoMap2->retrieveGhostedData ();
   }

   if ( lithoMap3 != 0 ) {
      lithoMap3->retrieveGhostedData ();
   }

   // Are any of the lithologies defined by a varying lithology map?
   if (( lithoMap1 != 0 and not lithoMap1->isConstant ()) or
       ( lithoMap2 != 0 and not lithoMap2->isConstant ()) or
       ( lithoMap3 != 0 and not lithoMap3->isConstant ())) {
      createLithoFromMaps = true;
   } else {
      createLithoFromMaps = false;
   }

   // Get lithology names.
   if ( getLithoType1 () != 0 ) {
      lithoName1 = getLithoType1 ()->getName ();
   } else {
      lithoName1 = "";
   }

   if ( getLithoType2 () != 0 ) {
      lithoName2 = getLithoType2 ()->getName ();
   } else {
      lithoName2 = "";
   }

   if ( getLithoType3 () != 0 ) {
      lithoName3 = getLithoType3 ()->getName ();
   } else {
      lithoName3 = "";
   }

   if (createLithoFromMaps) {
      unsigned int i;
      unsigned int j;

      for ( i = m_compoundLithologies.first ( 0 ); i <= m_compoundLithologies.last ( 0 ); ++i ) {

         for ( j = m_compoundLithologies.first ( 1 ); j <= m_compoundLithologies.last ( 1 ); ++j ) {
            noDefinedLithologyValue = false;

            if ( lithoMap1 != 0 ) {
               lithologyPercentage1 = lithoMap1->getValue ( i, j );

               if ( lithologyPercentage1 == Interface::DefaultUndefinedMapValue ) {
                  lithologyPercentage1 = 0.0;
                  noDefinedLithologyValue = true;
               }

            } else {
               lithologyPercentage1 = 0.0;
            }

            if ( lithoMap2 != 0 ) {
               lithologyPercentage2 = lithoMap2->getValue ( i, j );

               if ( lithologyPercentage2 == Interface::DefaultUndefinedMapValue ) {
                  lithologyPercentage2 = 0.0;
                  noDefinedLithologyValue = true;
               }

            } else {
               lithologyPercentage2 = 0.0;
            }

            if ( lithoMap3 != 0 ) {
               lithologyPercentage3 = lithoMap3->getValue ( i, j );

               if ( lithologyPercentage3 == Interface::DefaultUndefinedMapValue ) {
                  lithologyPercentage3 = 0.0;
                  noDefinedLithologyValue = true;
               }

            } else {
               lithologyPercentage3 = 0.0;
            }

            if ( noDefinedLithologyValue ) {
               m_compoundLithologies.addStratigraphyTableLithology ( i, j, 0 );
            } else {

               minimumPercentage = NumericFunctions::Minimum3 ( lithologyPercentage1, lithologyPercentage2, lithologyPercentage3 );
               maximumPercentage = NumericFunctions::Maximum3 ( lithologyPercentage1, lithologyPercentage2, lithologyPercentage3 );
               percentageSum = lithologyPercentage1 + lithologyPercentage2 + lithologyPercentage3;

               if ( minimumPercentage < -LithologyTolerance or 
                    maximumPercentage > 100.0 + LithologyTolerance or
                    not NumericFunctions::isEqual<double> ( percentageSum, 100.0, LithologyTolerance )) {

                  createdLithologies = false;

                  std::ostringstream errorBuffer;
                  errorBuffer << " Percentage Maps incorrect: ( " << i << ", " << j << " ) " 
                              << (lithoMap1 ? lithoMap1->getValue ( i, j ) : 0) << "  "
                              << (lithoMap2 ? lithoMap2->getValue ( i, j ) : 0) << "  "
                              << (lithoMap3 ? lithoMap3->getValue ( i, j ) : 0) << "  " 
                              << std::endl
                              << "          min (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) < " << -LithologyTolerance << "; or " << endl
                              << "          max (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) < " << 100 + LithologyTolerance << "; or " << endl
                              << "          sum (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) = " 
                              << lithologyPercentage1 + lithologyPercentage2 + lithologyPercentage3 << " /= " << 100 + LithologyTolerance << ". " << endl;
                
                  errorMessage = errorBuffer.str ();

                  cout << " Error message: "  << errorMessage << endl;

                  break;

               } else {

                  CompoundLithologyComposition lc ( lithoName1,           lithoName2,           lithoName3,
                                                    lithologyPercentage1, lithologyPercentage2, lithologyPercentage3,
                                                    getMixModelStr (), getLayeringIndex());

                  pMixedLitho = ((GeoPhysics::ProjectHandle*)(m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );

                  if ( pMixedLitho != 0 ) {
                     m_compoundLithologies.addStratigraphyTableLithology ( i, j, pMixedLitho );
                  } else {
                     createdLithologies = false;
                     break;
                  }

               }

            }

         }

         if ( not createdLithologies ) {
            // If created-lithologies is false then exit the outer loop as well.
            break;
         }

      }

//       if ( not successfulExecution ( createdLithologies )) {
//         return false;
//       }

   } else {
      noDefinedLithologyValue = false;

      if ( lithoMap1 != 0 ) {
         lithologyPercentage1 = lithoMap1->getConstantValue ();

         if ( lithologyPercentage1 == Interface::DefaultUndefinedMapValue or lithologyPercentage1 == Interface::DefaultUndefinedScalarValue ) {
            lithologyPercentage1 = 0.0;
            noDefinedLithologyValue = true;
         }

      } else {
         lithologyPercentage1 = 0.0;
      }

      if ( lithoMap2 != 0 ) {
         lithologyPercentage2 = lithoMap2->getConstantValue ();

         if ( lithologyPercentage2 == Interface::DefaultUndefinedMapValue or lithologyPercentage2 == Interface::DefaultUndefinedScalarValue ) {
            lithologyPercentage2 = 0.0;
            noDefinedLithologyValue = true;
         }

      } else {
         lithologyPercentage2 = 0.0;
      }

      if ( lithoMap3 != 0 ) {
         lithologyPercentage3 = lithoMap3->getConstantValue ();

         if ( lithologyPercentage3 == Interface::DefaultUndefinedMapValue or lithologyPercentage3 == Interface::DefaultUndefinedScalarValue ) {
            lithologyPercentage3 = 0.0;
            noDefinedLithologyValue = true;
         }

      } else {
         lithologyPercentage3 = 0.0;
      }

      if ( m_projectHandle->getModellingMode () == Interface::MODE1D and
           NumericFunctions::isEqual ( lithologyPercentage1 + lithologyPercentage2 + lithologyPercentage3, 0.0, LithologyTolerance )) {

         CompoundLithologyComposition lc ( OneDHiatusLithologyName, "", "",
                                           100.0, 0.0, 0.0,
                                           getMixModelStr (), getLayeringIndex());

         pMixedLitho = ((GeoPhysics::ProjectHandle*)(m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );
         createdLithologies = pMixedLitho != 0;
      } else if ( NumericFunctions::Minimum3 ( lithologyPercentage1, lithologyPercentage2, lithologyPercentage3 ) < -LithologyTolerance or 
                  NumericFunctions::Maximum3 ( lithologyPercentage1, lithologyPercentage2, lithologyPercentage3 ) > 100.0 + LithologyTolerance or
                  not NumericFunctions::isEqual ( lithologyPercentage1 + lithologyPercentage2 + lithologyPercentage3, 100.0, LithologyTolerance )) {

         pMixedLitho = 0;
         createdLithologies = false;
         std::ostringstream errorBuffer;
         errorBuffer << " Percentage values incorrect: " << std::endl
                     << "          min (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) < " << -LithologyTolerance << "; or " << endl
                     << "          max (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) < " << 100 + LithologyTolerance << "; or " << endl
                     << "          sum (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) = " 
                     << lithologyPercentage1 + lithologyPercentage2 + lithologyPercentage3 << " /= " << 100 + LithologyTolerance << ". " << endl;
                
         errorMessage = errorBuffer.str ();

         cout << " Error message: "  << errorMessage << endl;

      } else {
         CompoundLithologyComposition lc ( lithoName1,           lithoName2,           lithoName3,
                                           lithologyPercentage1, lithologyPercentage2, lithologyPercentage3,
                                           getMixModelStr (), getLayeringIndex() );

         pMixedLitho = ((GeoPhysics::ProjectHandle*)(m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );
         createdLithologies = pMixedLitho != 0;
      }

      if ( createdLithologies ) {
         m_compoundLithologies.fillWithLithology ( pMixedLitho );
      }

   }

   if ( not createdLithologies ) {
      cout << "MeSsAgE ERROR Could not create lithologies for layer: " << getName () << endl;
   }


   if ( lithoMap1 != 0 ) {
      lithoMap1->restoreData ( false, true );
   }

   if ( lithoMap2 != 0 ) {
      lithoMap2->restoreData ( false, true );
   }

   if ( lithoMap3 != 0 ) {
      lithoMap3->restoreData ( false, true );
   }
   
   if ( lithoMap1 ) lithoMap1->release();
   if ( lithoMap2 ) lithoMap2->release();
   if ( lithoMap3 ) lithoMap3->release();

   return createdLithologies;
}

//------------------------------------------------------------//

void GeoPhysics::Formation::setAllochthonousLayer ( AllochthonousLithologyInterpolator* interpolator ) {

  unsigned int i;
  unsigned int j;
  size_t t;

  double age;

  const AllochthonousLithologyInterpolator::MorphingTimeSteps& morphingSteps = interpolator->getMorphingTimeSteps ();

  for ( t = 0; t < morphingSteps.size (); ++t ) {
    age = morphingSteps [ t ];

    interpolator->setInterpolator ( age );

    for ( i = m_compoundLithologies.first ( 0 ); i <= m_compoundLithologies.last ( 0 ) ; i++ ) {

      for ( j = m_compoundLithologies.first ( 1 ); j <= m_compoundLithologies.last ( 1 ) ; j++ ) {
        m_compoundLithologies.addLithology ( i, j, age, (*interpolator)( i, j, age ));
      }

    }
    
  }

  m_compoundLithologies.setAllochthonousInterpolator ( interpolator );
}

//------------------------------------------------------------//

void GeoPhysics::Formation::setFaultLithologies ( bool& layerHasFaults,
                                                  bool& error ) {

   using Interface::X_COORD;
   using Interface::Y_COORD;

   GeoPhysics::ProjectHandle* project = dynamic_cast<GeoPhysics::ProjectHandle*>( m_projectHandle );

   Interface::FaultCollectionList* faultCollections = Interface::Formation::getFaultCollections ();
   Interface::FaultCollectionList::const_iterator faultCollectionIter;

   const Interface::Grid* activityGrid = dynamic_cast<const Interface::Grid*>(m_projectHandle->getActivityOutputGrid ());

   Interface::FaultElementCalculator FEC;
   DataAccess::Interface::ElementSet faultElements;
   CompoundLithology* faultLithology;

   FEC.setGrid ( activityGrid );

   // If any fault lithology is assigned to an element then this is set to true.
   layerHasFaults = false;

   error = false;

   for ( faultCollectionIter = faultCollections->begin (); faultCollectionIter != faultCollections->end (); ++faultCollectionIter ) {
      const Interface::FaultCollection* faultCollection = *faultCollectionIter;

      const Interface::FaultList* faults = faultCollection->getFaults ();
      Interface::FaultList::const_iterator faultIter;

      for ( faultIter = faults->begin (); faultIter != faults->end (); ++faultIter ) {
         const Interface::Fault* fault = *faultIter;
         faultElements.clear ();

         // Find all the elements that the fault intersects.
         // This returns just the (I, J) positions of the elements in the mesh.
         FEC.computeFaultElements ( fault->getFaultLine (), faultElements );

         Interface::Fault::OverpressureFaultEventIterator eventIter;

         for ( eventIter = fault->beginOverpressureEvents (); eventIter != fault->endOverpressureEvents (); ++eventIter ) {
            const Interface::OverpressureFaultEvent& event = *eventIter;

            if ( event.getUsedInOverpressureCalculation ()) {
               const double startAge = event.getSnapshot ()->getTime ();

               // Set here because even though the fault may not lie in any element of this process, 
               // the formation nevertheless has faults.
               layerHasFaults = true;
               const std::string& lithologyName = event.getFaultLithologyName ();

               Interface::ElementSet::const_iterator elementIter;

               for ( elementIter = faultElements.begin (); elementIter != faultElements.end (); ++elementIter ) {
                  const Interface::Element& element = *elementIter;

                  // Check to see if the element is a valid element on this subdomain.
                  if ( m_compoundLithologies.validIndex ( element ( X_COORD ), element ( Y_COORD ))) {
                     faultLithology = project->getLithologyManager ().getCompoundFaultLithology ( lithologyName, 
                                                                                                  getCompoundLithology ( element ( X_COORD ), element ( Y_COORD ))->getComposition ());

                     if ( faultLithology != 0 ) {
                        //
                        // Then define a change of lithology at the age specified.
                        //
                        m_compoundLithologies.addLithology ( element ( X_COORD ), element ( Y_COORD ), startAge, faultLithology );
                     } else {
                        //
                        // ERROR:
                        //

                        m_projectHandle->getMessageHandler ().printLine ( "MeSsAgE ERROR Fault::setBasinFault Unable to find or create the fault lithology ");
                        m_projectHandle->getMessageHandler ().print ( "MeSsAgE ERROR Fault::setBasinFault so for age " );
                        m_projectHandle->getMessageHandler ().print ( startAge );
                        m_projectHandle->getMessageHandler ().print ( " not fault lithology will be set" );
                        m_projectHandle->getMessageHandler ().newLine ();

                        error = true;
                     }

                  }

               }

            }

         }

      }

   }

   delete faultCollections;
   m_containsFault = layerHasFaults;
}

//------------------------------------------------------------//

void GeoPhysics::Formation::switchLithologies ( const double age ) {

  bool switchHasOccurred = m_compoundLithologies.setCurrentLithologies ( age );

#if 0
  cout << " GeoPhysics::Formation::switchLithologies " << getName () << "  " 
       << ( switchHasOccurred ? " TRUE " : " FALSE " ) << endl;
#endif

}

//------------------------------------------------------------//

void GeoPhysics::Formation::determineMinMaxThickness () {

   if ( kind () == Interface::SEDIMENT_FORMATION ) {
      m_minimumDepositedThickness =  1.0e10;
      m_maximumDepositedThickness = -1.0e10;

      Interface::MobileLayerList::const_iterator mobIt;
      Interface::MobileLayerList* mobileLayers = getMobileLayers ();

      double gridMapMinimum;
      double gridMapMaximum;

      const Interface::Snapshot* presentDay = m_projectHandle->findSnapshot ( 0.0 );
      const Interface::GridMap* gridMap = dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ());

      gridMap->retrieveData ();
      gridMap->getMinMaxValue ( gridMapMinimum, gridMapMaximum );
      gridMap->restoreData ( false );

      m_minimumDepositedThickness = NumericFunctions::Minimum ( m_minimumDepositedThickness, gridMapMinimum );
      m_maximumDepositedThickness = NumericFunctions::Maximum ( m_maximumDepositedThickness, gridMapMaximum );

      for ( mobIt = mobileLayers->begin (); mobIt != mobileLayers->end (); ++mobIt ) {

         // The map at present day will be handled by the input-thickness map.
         if ((*mobIt)->getSnapshot () != presentDay ) {
            gridMap = dynamic_cast<const Interface::GridMap*>((*mobIt)->getMap ( Interface::MobileLayerThicknessMap ));

            gridMap->retrieveData ();
            gridMap->getMinMaxValue ( gridMapMinimum, gridMapMaximum );

            m_minimumDepositedThickness = NumericFunctions::Minimum ( m_minimumDepositedThickness, gridMapMinimum );
            m_maximumDepositedThickness = NumericFunctions::Maximum ( m_maximumDepositedThickness, gridMapMaximum );

            gridMap->restoreData ( false );
            gridMap->release();
         }

      }

      delete mobileLayers;
   }

}

//------------------------------------------------------------//

void GeoPhysics::Formation::setDepthRefinementFactor ( const int zRefinementFactor ) {
   m_zRefinementFactor = zRefinementFactor;
}

//------------------------------------------------------------//

unsigned int GeoPhysics::Formation::setMaximumNumberOfElements ( const bool readSizeFromVolumeData ) {

   if ( readSizeFromVolumeData ) {

      // Any volume property could be used here that is always output, Ves is always output.
      const Interface::Property* vesProperty = m_projectHandle->findProperty ( "Ves" );

      Interface::PropertyValueList* vesValueList = m_projectHandle->getPropertyValues ( Interface::FORMATION,
                                                                                        vesProperty,
                                                                                        m_projectHandle->findSnapshot ( 0.0, Interface::MAJOR ),
                                                                                        0,
                                                                                        this,
                                                                                        0,
                                                                                        Interface::VOLUME );

      assert ( vesValueList->size () == 1 );

      const Interface::PropertyValue* ves = *vesValueList->begin ();

      if ( ves != 0 and ves->getGridMap () != 0 ) {
         m_maximumNumberOfElements = ves->getGridMap ()->getDepth () - 1;
      } else {
         m_maximumNumberOfElements = 1;
      }

      delete vesValueList;

   } else {

      const double sedimentElementHeight = m_projectHandle->getRunParameters ()->getBrickHeightSediment ();

      double layerMaximumThickness = getMaximumThickness ();

      if ( isMobileLayer ()) {
         // I do not know what this is for!
         /* OFM - introduced to reproduce Cauldron behaviour */
         layerMaximumThickness = NumericFunctions::Maximum ( 1000.0, layerMaximumThickness );
      }

      if ( layerMaximumThickness < 0.0 ) {
         layerMaximumThickness = 0.0;
      }

      m_maximumNumberOfElements = (unsigned int)( m_zRefinementFactor * std::ceil ( layerMaximumThickness / sedimentElementHeight ));
      m_maximumNumberOfElements = NumericFunctions::Maximum<unsigned int> ( 1, m_maximumNumberOfElements );
   }

   return m_maximumNumberOfElements;
}

//------------------------------------------------------------//

void GeoPhysics::Formation::retrieveAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->retrieveGhostedData ();

   if ( isMobileLayer ()) {
      Interface::MobileLayerList* mobileLayers = getMobileLayers ();
      Interface::MobileLayerList::iterator mobileLayerIter;

      for ( mobileLayerIter = mobileLayers->begin (); mobileLayerIter != mobileLayers->end (); ++mobileLayerIter ) {
         dynamic_cast<const Interface::GridMap*>((*mobileLayerIter)->getMap (Interface::MobileLayerThicknessMap))->retrieveGhostedData ();
      }

      delete mobileLayers;
   }

}

//------------------------------------------------------------//

void GeoPhysics::Formation::restoreAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->restoreData ( false, true );

   if ( isMobileLayer ()) {
      Interface::MobileLayerList* mobileLayers = getMobileLayers ();
      Interface::MobileLayerList::iterator mobileLayerIter;

      for ( mobileLayerIter = mobileLayers->begin (); mobileLayerIter != mobileLayers->end (); ++mobileLayerIter ) {
         dynamic_cast<const Interface::GridMap*>((*mobileLayerIter)->getMap (Interface::MobileLayerThicknessMap))->restoreData ( false, true );
      }

      delete mobileLayers;
   }

}

