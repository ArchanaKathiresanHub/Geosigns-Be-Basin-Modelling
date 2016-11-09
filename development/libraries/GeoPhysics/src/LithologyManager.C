//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "LithologyManager.h"
#include "PiecewiseInterpolator.h"

#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"

using namespace DataAccess;

//------------------------------------------------------------//

GeoPhysics::LithologyManager::LithologyManager ( ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
}

//------------------------------------------------------------//

GeoPhysics::LithologyManager::~LithologyManager () {

  CompoundLithologyMapping::iterator compoundIter;

  for ( compoundIter = compoundLithologies.begin (); compoundIter != compoundLithologies.end (); ++compoundIter ) {
     delete (*compoundIter).second;
  }

}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::LithologyManager::getCompoundLithology ( const CompoundLithologyComposition& composition ) {

  CompoundLithology*    compoundLithology;
  SimpleLithology* simpleLithology;
  int              I;

  CompoundLithologyIterator compoundIter = compoundLithologies.find ( composition );

  // If the compound lithology could not be found on the 
  // list then it must be created and added to the list.
  // Basalt, as a part of mantle or Crust, can have differnent type of a thermal model 
  if ( compoundIter == compoundLithologies.end() || ( composition.lithologyName( 3 ) == DataAccess::Interface::ALCBasalt )) {
    bool   mixedSuccessfully;

    compoundLithology = dynamic_cast<GeoPhysics::ObjectFactory*>(m_projectHandle->getFactory ())->produceCompoundLithology ( m_projectHandle );
    compoundLithology->setMixModel( composition.mixingModel(), composition.layeringIndex());

    for ( I = 1; I <= MaximumNumberOfLithologies; I++ ) {

      if ( composition.lithologyFraction ( I ) > 0.0 ) {
        simpleLithology = getSimpleLithology ( composition.lithologyName ( I ));

        if ( simpleLithology != 0 ) {
           simpleLithology->setThermalModel( composition.thermalModel() );;
        
           compoundLithology->addLithology ( simpleLithology, composition.lithologyFraction ( I ));
           
        } else {

           m_projectHandle->getMessageHandler ().print ( "MeSsAgE ERROR GeoPhysics::LithologyManager::getCompoundLithology  could not find simple lithology " );
           m_projectHandle->getMessageHandler ().print ( composition.lithologyName ( I ));
           m_projectHandle->getMessageHandler ().newLine ();

           delete compoundLithology;
           return 0;
        }

      }

    }

    compoundLithology->makeFault ( false );
    mixedSuccessfully = compoundLithology->reCalcProperties();

    if ( mixedSuccessfully ) {
      compoundLithologies [ composition ] = compoundLithology;
    } else {
       m_projectHandle->getMessageHandler ().printLine ( "MeSsAgE ERROR GeoPhysics::LithologyManager::getCompoundLithology  the mixing of the compound lithology was not successful" );
       delete compoundLithology;
       compoundLithology = 0;
    }

  } else {
    compoundLithology = (*compoundIter).second;
  }

  return compoundLithology;
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::LithologyManager::getCompoundLithology ( const std::string& simpleLithologyName ) {

  CompoundLithologyComposition composition;

  composition.setComposition ( simpleLithologyName, "", "", 100.0, 0.0, 0.0, "Homogeneous", -9999 );
  return getCompoundLithology ( composition );

}

//------------------------------------------------------------//

int GeoPhysics::LithologyManager::numberOfSimpleLithologies () const {
  return simpleLithologies.size ();
}

//------------------------------------------------------------//

void GeoPhysics::LithologyManager::printSimpleLithologies () const {

  size_t I = 0;

  for ( I = 0; I < simpleLithologies.size(); I++ ) {
    simpleLithologies [ I ]->print ();
  } 
  
}

//------------------------------------------------------------//

GeoPhysics::SimpleLithology* GeoPhysics::LithologyManager::getSimpleFaultLithology ( const std::string&                        simpleLithologyName,
                                                                                     const Interface::PressureFaultStatus verticalStatus,
                                                                                     const Interface::PressureFaultStatus lateralStatus ) {

  SimpleLithology* simpleLithology;
  SimpleLithology* simpleFaultLithology;

  std::string simpleFaultLithologyName;

  if ( simpleLithologyName != "" ) {
    simpleFaultLithologyName = generateSimpleFaultLithologyName ( simpleLithologyName, verticalStatus, lateralStatus );

    simpleFaultLithology = getSimpleLithology ( simpleFaultLithologyName );

    // If the fault lithology does not already exsit, then one must be created with the correct properties.
    if ( simpleFaultLithology == 0 ) {

      // Put these in correct place when everything is defined. WHERE IS THE CORRECT PLACE???
      static const double permeabilityAnisotropy = 1000.0;
      static const double passingPermeabilities [ 2 ] = { -1.0, -3.0 };

      // The values here should be derived from the passingPerm values * log10 ( permAniso ), log10 ( 1000 ) = 3
      static const double sealingPermeabilities [ 2 ] = { -4.0, -6.0 };

      std::vector<double> defaultPermeabilities(2);
      std::vector<double> defaultPorosities(2);

      defaultPorosities[0] = 70.0 ; 
      defaultPorosities[1] = 3.0  ; 

      double anisotropy;

      if ( verticalStatus == Interface::PASS_WATER ) {
        defaultPermeabilities [ 0 ] = passingPermeabilities [ 0 ];
        defaultPermeabilities [ 1 ] = passingPermeabilities [ 1 ];

        if ( lateralStatus == Interface::PASS_WATER ) {
          anisotropy = 1.0;
        } else {
          anisotropy = 1.0 / permeabilityAnisotropy;
        }

      } else {
        defaultPermeabilities [ 0 ] = sealingPermeabilities [ 0 ];
        defaultPermeabilities [ 1 ] = sealingPermeabilities [ 1 ];

        if ( lateralStatus == Interface::PASS_WATER ) {
          anisotropy = permeabilityAnisotropy;
        } else {
          anisotropy = 1.0;
        }

      }

      simpleLithology = getSimpleLithology ( simpleLithologyName );

      assert ( simpleLithology != 0 );

      // Create the new fault lithology (simple lithology).
      simpleFaultLithology = (SimpleLithology*)(((GeoPhysics::ObjectFactory*)(m_projectHandle->getFactory ()))->produceLithoType ( simpleLithology, simpleFaultLithologyName, anisotropy, defaultPorosities, defaultPermeabilities));

      // Now add it to the list of currently defined simple lithologies.
      simpleLithologies.push_back ( simpleFaultLithology );
    }

  } else {
    simpleFaultLithology = 0;
  }

  return simpleFaultLithology;
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::LithologyManager::getCompoundFaultLithology ( const CompoundLithologyComposition&       composition,
                                                                                         const  DataAccess::Interface::PressureFaultStatus verticalStatus,
                                                                                         const  DataAccess::Interface::PressureFaultStatus lateralStatus ) {

  CompoundLithology* faultLithology;
  SimpleLithology*   simpleFaultLithology;
  CompoundLithologyComposition  faultComposition;
  int               I;


  // Create the lithology composition for the fault lithology from the given lithology composition.
  generateCompoundFaultLithologyComposition ( composition, verticalStatus, lateralStatus, faultComposition );
  
  // Find the fault lithology in the set of compound lithologies.
  CompoundLithologyIterator compoundIter = compoundLithologies.find ( faultComposition );

  // If the fault lithology composition was not found on the list 
  // of lithologies then it must be created.
  if ( compoundIter == compoundLithologies.end ()) {
    bool mixedSuccessfully;

    faultLithology = dynamic_cast<GeoPhysics::ObjectFactory*>(m_projectHandle->getFactory ())->produceCompoundLithology ( m_projectHandle );
    faultLithology->setMixModel( faultComposition.mixingModel(), faultComposition.layeringIndex());

    for ( I = 1; I <= MaximumNumberOfLithologies; I++ ) {

      if ( faultComposition.lithologyFraction ( I ) > 0.0 ) {

        // NOTICE: Here we use the composition.lithologyName, since the name in the faultComposition may not exist at this point.
        simpleFaultLithology = getSimpleFaultLithology ( composition.lithologyName ( I ), verticalStatus, lateralStatus );

        if ( simpleFaultLithology != 0 ) {
          faultLithology->addLithology ( simpleFaultLithology, faultComposition.lithologyFraction ( I ));
        } else {

           m_projectHandle->getMessageHandler ().print ( "MeSsAgE ERROR GeoPhysics::LithologyManager::getCompoundFaultLithology  could not find simple lithology " );
           m_projectHandle->getMessageHandler ().print ( composition.lithologyName ( I ));
           m_projectHandle->getMessageHandler ().newLine ();

          delete faultLithology;
          return 0;
        }

      }

    }

    faultLithology->makeFault ( true );
    mixedSuccessfully = faultLithology->reCalcProperties();

    if ( mixedSuccessfully ) {
      // If the lithology was mixed successfully then add it to the set of compound lithologies.
      compoundLithologies [ faultComposition ] = faultLithology;
    } else {

      // If the compound lithology was not mixed correctly then
      // return a null pointer, since this lithology cannot be used.
      m_projectHandle->getMessageHandler ().printLine ( "MeSsAgE ERROR GeoPhysics::LithologyManager::getCompoundFaultLithology  the mixing of the compound lithology was not successful. " );

      // ERROR: Could not create the compounnd lithology with the fault composition.
      delete faultLithology;
      faultLithology = 0;
    }

  } else {
    faultLithology = (*compoundIter).second;
  }

  return faultLithology;
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::LithologyManager::getCompoundFaultLithology ( const std::string&                  simpleLithologyName,
                                                                                         const CompoundLithologyComposition& composition ) {

   CompoundLithology* compoundFaultLithology;

   if ( simpleLithologyName == FaultDefaultSealingLithologyName ) { 
      compoundFaultLithology = getCompoundFaultLithology ( composition, Interface::SEAL_WATER, Interface::SEAL_WATER );
   } else if ( simpleLithologyName == FaultDefaultPassingLithologyName ) { 
      compoundFaultLithology = getCompoundFaultLithology ( composition, Interface::PASS_WATER, Interface::PASS_WATER );
   } else {
      std::string      faultLithologyName = generateSimpleFaultLithologyName ( simpleLithologyName );
      SimpleLithology* simpleLithology = getSimpleLithology ( simpleLithologyName );

      CompoundLithologyComposition faultComposition;
      SimpleLithology* simpleFaultLithology;

      if ( simpleLithology != 0 ) {

         simpleFaultLithology = (SimpleLithology*)(((GeoPhysics::ObjectFactory*)(m_projectHandle->getFactory ()))->produceLithoType ( simpleLithology, faultLithologyName ));
         simpleLithologies.push_back ( simpleFaultLithology );

         faultComposition.setComposition ( faultLithologyName, "", "", 100.0, 0.0, 0.0, "Homogeneous", -9999 );
         compoundFaultLithology = getCompoundLithology ( faultComposition );
      } else {
         compoundFaultLithology = 0;
      }

   }

   if ( compoundFaultLithology != 0 ) {
      compoundFaultLithology->makeFault ( true );
   }

   return compoundFaultLithology;
}

//------------------------------------------------------------//

GeoPhysics::SimpleLithology* GeoPhysics::LithologyManager::getSimpleLithology ( const std::string& simpleLithologyName ) const {

  size_t I;

  for ( I = 0; I < simpleLithologies.size(); I++ ) {

    if ( simpleLithologies [ I ]->getName () == simpleLithologyName ) {
      return simpleLithologies [ I ];
    }

  }

  return 0;
}

//------------------------------------------------------------//

std::string GeoPhysics::LithologyManager::generateSimpleFaultLithologyName ( const std::string&                   simpleLithologyName,
                                                                             const DataAccess::Interface::PressureFaultStatus verticalStatus,
                                                                             const DataAccess::Interface::PressureFaultStatus lateralStatus ) const {

  std::string simpleFaultLithologyName;

  if ( simpleLithologyName == "" ) {
    simpleFaultLithologyName = "";
  } else {
    simpleFaultLithologyName = simpleLithologyName + "_FAULT";

    if ( verticalStatus == Interface::SEAL_WATER ) {
      simpleFaultLithologyName += "_SEAL";
    } else {
      simpleFaultLithologyName += "_PASS";
    }

    if ( lateralStatus == Interface::SEAL_WATER ) {
      simpleFaultLithologyName += "_SEAL";
    } else {
      simpleFaultLithologyName += "_PASS";
    }

  }

  return simpleFaultLithologyName;
}

//------------------------------------------------------------//

std::string GeoPhysics::LithologyManager::generateSimpleFaultLithologyName ( const std::string& simpleLithologyName ) const {

  std::string simpleFaultLithologyName;

  if ( simpleLithologyName == "" ) {
    simpleFaultLithologyName = "";
  } else {
    // The name is extended to a length that cannot be input in the GUI.
    // This is to prevent name clashes with user defined lithologies.
    simpleFaultLithologyName = simpleLithologyName + "_SIMPLE_FAULT_LITHOLOGY";
  }

  return simpleFaultLithologyName;
}

//------------------------------------------------------------//

void GeoPhysics::LithologyManager::generateCompoundFaultLithologyComposition ( const CompoundLithologyComposition&  composition,
                                                                               const Interface::PressureFaultStatus verticalStatus,
                                                                               const Interface::PressureFaultStatus lateralStatus,
                                                                                     CompoundLithologyComposition&  faultComposition ) const {
  
  faultComposition.setComposition ( generateSimpleFaultLithologyName ( composition.lithologyName ( 1 ), verticalStatus, lateralStatus ),
                                    generateSimpleFaultLithologyName ( composition.lithologyName ( 2 ), verticalStatus, lateralStatus ),
                                    generateSimpleFaultLithologyName ( composition.lithologyName ( 3 ), verticalStatus, lateralStatus ),
                                    composition.lithologyFraction ( 1 ),
                                    composition.lithologyFraction ( 2 ),
                                    composition.lithologyFraction ( 3 ),
                                    composition.mixingModel (),
                                    composition.layeringIndex());


}

//------------------------------------------------------------//

void GeoPhysics::LithologyManager::addSimpleLithology ( const SimpleLithologyPtr newSimpleLithology ) {
  simpleLithologies.push_back ( newSimpleLithology );
}

//------------------------------------------------------------//

GeoPhysics::LithologyManager::CompoundLithologyIterator GeoPhysics::LithologyManager::begin () const {
  return compoundLithologies.begin ();
}

//------------------------------------------------------------//

GeoPhysics::LithologyManager::CompoundLithologyIterator GeoPhysics::LithologyManager::end () const {
  return compoundLithologies.end ();
}

//------------------------------------------------------------//
