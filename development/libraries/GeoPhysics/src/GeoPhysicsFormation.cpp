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

#include "Grid.h"
#include "GridMap.h"
#include "ProjectHandle.h"
#include "Snapshot.h"
#include "PropertyValue.h"
#include "Surface.h"
#include "MobileLayer.h"
#include "auxiliaryfaulttypes.h"
#include "Faulting.h"
#include "FaultCollection.h"
#include "FaultElementCalculator.h"
#include "RunParameters.h"

#include "GeoPhysicsProjectHandle.h"
#include "CompoundLithologyComposition.h"
#include "LithologyManager.h"

#include "NumericFunctions.h"
#include "FormattingException.h"

typedef formattingexception::GeneralException GeoPhysicsFormationException;


using namespace DataAccess;

using namespace std;

//------------------------------------------------------------//

GeoPhysics::GeoPhysicsFormation::GeoPhysicsFormation ( DataAccess::Interface::ProjectHandle& projectHandle,
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

GeoPhysics::GeoPhysicsFormation::~GeoPhysicsFormation () {
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::GeoPhysicsFormation::getLithologyFromStratTable ( bool& undefinedMapValue,
                                                                                             bool useMaps,
                                                                                             unsigned int i,
                                                                                             unsigned int j,
                                                                                             const Interface::GridMap* lithoMap1,
                                                                                             const Interface::GridMap* lithoMap2,
                                                                                             const Interface::GridMap* lithoMap3,
                                                                                             const string& lithoName1,
                                                                                             const string& lithoName2,
                                                                                             const string& lithoName3 ) const
{
  const double LithologyTolerance = 1.0e-4;

  double lithologyPercentage1 = 0.0;
  if ( lithoMap1 )
  {
     lithologyPercentage1 = useMaps ? lithoMap1->getValue ( i, j ) : lithoMap1->getConstantValue();
     if ( lithologyPercentage1 == Interface::DefaultUndefinedMapValue || lithologyPercentage1 == Interface::DefaultUndefinedScalarValue )
     {
       undefinedMapValue = true;
       return nullptr;
     }
  }

  double lithologyPercentage2 = 0.0;
  if ( lithoMap2 )
  {
     lithologyPercentage2 = useMaps ? lithoMap2->getValue ( i, j ) : lithoMap2->getConstantValue();
     if ( lithologyPercentage2 == Interface::DefaultUndefinedMapValue || lithologyPercentage2 == Interface::DefaultUndefinedScalarValue )
     {
       undefinedMapValue = true;
       return nullptr;
     }
  }

  double lithologyPercentage3 = 0.0;
  if ( lithoMap3 )
  {
     lithologyPercentage3 = useMaps ? lithoMap3->getValue ( i, j ) : lithoMap3->getConstantValue();
     if ( lithologyPercentage3 == Interface::DefaultUndefinedMapValue || lithologyPercentage3 == Interface::DefaultUndefinedScalarValue )
     {
       undefinedMapValue = true;
       return nullptr;
     }
  }

  double minimumPercentage = NumericFunctions::Minimum3 ( lithologyPercentage1, lithologyPercentage2, lithologyPercentage3 );
  double maximumPercentage = NumericFunctions::Maximum3 ( lithologyPercentage1, lithologyPercentage2, lithologyPercentage3 );
  double percentageSum = lithologyPercentage1 + lithologyPercentage2 + lithologyPercentage3;

  if ( minimumPercentage < -LithologyTolerance ||
       maximumPercentage > 100.0 + LithologyTolerance ||
       ! NumericFunctions::isEqual<double> ( percentageSum, 100.0, LithologyTolerance ))
  {
     std::ostringstream errorBuffer;
     if (useMaps)
     {
       errorBuffer << " Percentage Maps incorrect: ( " << i << ", " << j << " ) "
                   << lithologyPercentage1 << "  " << lithologyPercentage2 << "  " << lithologyPercentage3 << "  " << std::endl;
     }
     else
     {
       errorBuffer << " Percentage values incorrect: " << std::endl;
     }
     errorBuffer << "          min (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) < " << -LithologyTolerance << "; or " << endl
                 << "          max (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) < " << 100 + LithologyTolerance << "; or " << endl
                 << "          sum (  " << lithologyPercentage1 << ", " << lithologyPercentage2 << ", " << lithologyPercentage3 << " ) = "
                 << lithologyPercentage1 + lithologyPercentage2 + lithologyPercentage3 << " /= " << 100 + LithologyTolerance << ". " << endl;

     const std::string errorMessage = errorBuffer.str ();
     cout << "Basin_Error: "  << errorMessage << endl;
     return nullptr;
  }
  else
  {
     CompoundLithologyComposition lc ( lithoName1,           lithoName2,           lithoName3,
                                       lithologyPercentage1, lithologyPercentage2, lithologyPercentage3,
                                       getMixModelStr (), getLayeringIndex());

     return dynamic_cast<GeoPhysics::ProjectHandle&>(getProjectHandle()).getLithologyManager ().getCompoundLithology ( lc );
  }
}

bool GeoPhysics::GeoPhysicsFormation::setLithologiesFromStratTable ()
{
   bool createdLithologies = true;

   m_compoundLithologies.allocate ( getProjectHandle().getActivityOutputGrid ());

   const Interface::GridMap* lithoMap1 = getLithoType1PercentageMap ();
   const Interface::GridMap* lithoMap2 = getLithoType2PercentageMap ();
   const Interface::GridMap* lithoMap3 = getLithoType3PercentageMap ();

   // Get lithology names.
   const string lithoName1 = ( getLithoType1 () ) ? getLithoType1 ()->getName () : "";
   const string lithoName2 = ( getLithoType2 () ) ? getLithoType2 ()->getName () : "";
   const string lithoName3 = ( getLithoType3 () ) ? getLithoType3 ()->getName () : "";

   // Load the lithology maps.
   if ( lithoMap1 ) lithoMap1->retrieveGhostedData ();
   if ( lithoMap2 ) lithoMap2->retrieveGhostedData ();
   if ( lithoMap3 ) lithoMap3->retrieveGhostedData ();

   // Are any of the lithologies defined by a varying lithology map?
   if (( lithoMap1 && ! lithoMap1->isConstant ()) ||
       ( lithoMap2 && ! lithoMap2->isConstant ()) ||
       ( lithoMap3 && ! lithoMap3->isConstant ()))
   {
      for ( unsigned int i = m_compoundLithologies.first ( 0 ); i <= m_compoundLithologies.last ( 0 ); ++i )
      {
         for ( unsigned int j = m_compoundLithologies.first ( 1 ); j <= m_compoundLithologies.last ( 1 ); ++j )
         {
            bool undefinedMapValue = false;
            CompoundLithology* pMixedLitho = getLithologyFromStratTable( undefinedMapValue, true, i, j, lithoMap1, lithoMap2, lithoMap3, lithoName1, lithoName2, lithoName3);

            if ( pMixedLitho || undefinedMapValue )
            {
               m_compoundLithologies.addStratigraphyTableLithology ( i, j, pMixedLitho );
            } else {
               createdLithologies = false;
               break;
            }
         }

         if ( ! createdLithologies ) {
            // If created-lithologies is false then exit the outer loop as well.
            break;
         }
      }
   }
   else
   {
     bool undefinedMapValue = false;
     CompoundLithology* pMixedLitho = getLithologyFromStratTable( undefinedMapValue, false, 0, 0, lithoMap1, lithoMap2, lithoMap3, lithoName1, lithoName2, lithoName3);

     createdLithologies = (pMixedLitho);
     if ( createdLithologies )
     {
        m_compoundLithologies.fillWithLithology ( pMixedLitho );
     }
   }

   if ( ! createdLithologies ) {
      cout << "Basin_Error: Could not create lithologies for layer: " << getName () << endl;
   }


   if ( lithoMap1 )
   {
     lithoMap1->restoreData ( false, true );
     lithoMap1->release();
   }

   if ( lithoMap2 )
   {
     lithoMap2->restoreData ( false, true );
     lithoMap2->release();
   }

   if ( lithoMap3 )
   {
     lithoMap3->restoreData ( false, true );
     lithoMap3->release();
   }

   return createdLithologies;
}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsFormation::setAllochthonousLayer ( AllochthonousLithologyInterpolator* interpolator ) {

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

void GeoPhysics::GeoPhysicsFormation::setFaultLithologies ( bool& layerHasFaults,
                                                  bool& error ) {

   using Interface::X_COORD;
   using Interface::Y_COORD;

   GeoPhysics::ProjectHandle& project = dynamic_cast<GeoPhysics::ProjectHandle&>( getProjectHandle() );

   std::unique_ptr<Interface::FaultCollectionList> faultCollections( Interface::Formation::getFaultCollections () );

   const Interface::Grid* activityGrid = dynamic_cast<const Interface::Grid*>(project.getActivityOutputGrid ());

   Interface::FaultElementCalculator FEC;
   DataAccess::Interface::ElementSet faultElements;
   CompoundLithology* faultLithology;

   FEC.setGrid ( activityGrid );

   // If any fault lithology is assigned to an element then this is set to true.
   layerHasFaults = false;

   error = false;

   for ( const Interface::FaultCollection* faultCollection : *faultCollections )
   {
      std::unique_ptr<Interface::FaultList> faults( faultCollection->getFaults () );
      for ( const Interface::Fault* fault : *faults )
      {
         faultElements.clear ();

         // Find all the elements that the fault intersects.
         // This returns just the (I, J) positions of the elements in the mesh.
         FEC.computeFaultElements ( fault->getFaultLine (), faultElements );

         Interface::Fault::OverpressureFaultEventIterator eventIter;

         for ( eventIter = fault->beginOverpressureEvents (); eventIter != fault->endOverpressureEvents (); ++eventIter ) {
            const Interface::OverpressureFaultEvent& event = *eventIter;

            if ( event.getUsedInOverpressureCalculation ()) {
               const double startAge = event.getAge();

               // Set here because even though the fault may not lie in any element of this process,
               // the formation nevertheless has faults.
               layerHasFaults = true;
               const std::string& lithologyName = event.getFaultLithologyName ();

               Interface::ElementSet::const_iterator elementIter;

               for ( const Interface::Element& element : faultElements )
               {
                  // Check to see if the element is a valid element on this subdomain.
                  if ( m_compoundLithologies.validIndex ( element ( X_COORD ), element ( Y_COORD ))) {
                     faultLithology = project.getLithologyManager ().getCompoundFaultLithology ( lithologyName,
                                                                                                 getCompoundLithology ( element ( X_COORD ), element ( Y_COORD ))->getComposition ());

                     if ( faultLithology != nullptr ) {
                        //
                        // Then define a change of lithology at the age specified.
                        //
                        m_compoundLithologies.addLithology ( element ( X_COORD ), element ( Y_COORD ), startAge, faultLithology );
                     } else {
                        //
                        // ERROR:
                        //

                        getProjectHandle().getMessageHandler ().printLine ( "Basin_Error: Fault::setBasinFault Unable to find or create the fault lithology ");
                        getProjectHandle().getMessageHandler ().print ( "Basin_Error: Fault::setBasinFault so for age " );
                        getProjectHandle().getMessageHandler ().print ( startAge );
                        getProjectHandle().getMessageHandler ().print ( " not fault lithology will be set" );
                        getProjectHandle().getMessageHandler ().newLine ();

                        error = true;
                     }

                  }

               }

            }

         }

      }
   }

   m_containsFault = layerHasFaults;
}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsFormation::switchLithologies ( const double age ) {

  bool switchHasOccurred = m_compoundLithologies.setCurrentLithologies ( age );

#if 0
  cout << " GeoPhysics::GeoPhysicsFormation::switchLithologies " << getName () << "  "
       << ( switchHasOccurred ? " TRUE " : " FALSE " ) << endl;
#endif

}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsFormation::determineMinMaxThickness () {

   if ( kind () == Interface::SEDIMENT_FORMATION ) {
      m_minimumDepositedThickness =  1.0e10;
      m_maximumDepositedThickness = -1.0e10;

      double gridMapMinimum;
      double gridMapMaximum;

      const Interface::Snapshot* presentDay = getProjectHandle().findSnapshot ( 0.0 );
      const Interface::GridMap* gridMap = dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ());

      gridMap->retrieveData ();
      gridMap->getMinMaxValue ( gridMapMinimum, gridMapMaximum );
      gridMap->restoreData ( false );

      m_minimumDepositedThickness = NumericFunctions::Minimum ( m_minimumDepositedThickness, gridMapMinimum );
      m_maximumDepositedThickness = NumericFunctions::Maximum ( m_maximumDepositedThickness, gridMapMaximum );

      // at this point only valid MobileLayerList was collected from MobLayThicknIoTbl
      // still never hurts to check
      if (this->isMobileLayer())
      {
          Interface::MobileLayerList::const_iterator mobIt;
          Interface::MobileLayerList* mobileLayersList = getMobileLayers(); 
          for (mobIt = mobileLayersList->begin(); mobIt != mobileLayersList->end(); ++mobIt) {
              auto age = (*mobIt)->getSnapshot();
              // The map at present day will be handled by the input-thickness map.
              if (age != presentDay) {
                  gridMap = dynamic_cast<const Interface::GridMap*>((*mobIt)->getMap(Interface::MobileLayerThicknessMap));
                  if (gridMap) {
                      gridMap->retrieveData();
                      gridMap->getMinMaxValue(gridMapMinimum, gridMapMaximum);

                      m_minimumDepositedThickness = NumericFunctions::Minimum(m_minimumDepositedThickness, gridMapMinimum);
                      m_maximumDepositedThickness = NumericFunctions::Maximum(m_maximumDepositedThickness, gridMapMaximum);

                      gridMap->restoreData(false);
                      gridMap->release();
                  }
                  else
                      throw GeoPhysicsFormationException() << "Invalid thickness entry encountered for this layer, "
                      << this->getName() << ", at " << age->asString() << " Ma. check MobLayThicknIoTbl!";
              }

          }

          delete mobileLayersList;
      }
   }

}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsFormation::setDepthRefinementFactor ( const int zRefinementFactor ) {
   m_zRefinementFactor = zRefinementFactor;
}

//------------------------------------------------------------//

unsigned int GeoPhysics::GeoPhysicsFormation::setMaximumNumberOfElements ( const bool readSizeFromVolumeData ) {

   if ( readSizeFromVolumeData ) {

      // Any volume property could be used here that is always output, Ves is always output.
      const Interface::Property* vesProperty = getProjectHandle().findProperty ( "Ves" );

      Interface::PropertyValueList* vesValueList = getProjectHandle().getPropertyValues ( Interface::FORMATION,
                                                                                        vesProperty,
                                                                                        getProjectHandle().findSnapshot ( 0.0, Interface::MAJOR ),
                                                                                        0,
                                                                                        this,
                                                                                        0,
                                                                                        Interface::VOLUME );

      assert ( vesValueList->size () == 1 );

      const Interface::PropertyValue* ves = *vesValueList->begin ();

      if ( ves != nullptr and ves->getGridMap () != nullptr ) {
         m_maximumNumberOfElements = ves->getGridMap ()->getDepth () - 1;
      } else {
         m_maximumNumberOfElements = 1;
      }

      delete vesValueList;

   } else {

      const double sedimentElementHeight = getProjectHandle().getRunParameters ()->getBrickHeightSediment ();

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

void GeoPhysics::GeoPhysicsFormation::retrieveAllThicknessMaps () {

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

void GeoPhysics::GeoPhysicsFormation::restoreAllThicknessMaps () {

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

