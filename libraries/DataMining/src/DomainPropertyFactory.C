#include "DomainPropertyFactory.h"

#include "DomainSurfaceProperty.h"
#include "DomainFormationProperty.h"
#include "ConstantDomainFormationProperty.h"
#include "DomainReservoirProperty.h"

#include "GenexResultManager.h"
#include "ComponentManager.h"

#include "DataMiningProjectHandle.h"
#include "DomainPropertyCollection.h"

#include "PorosityCalculator.h"
#include "PermeabilityCalculator.h"

DataAccess::Interface::ProjectHandle* DataAccess::Mining::DomainPropertyFactory::produceProjectHandle (database::Database * database, 
                                                                                                            const std::string &  name,
                                                                                                            const std::string &  accessMode) {

   DataAccess::Mining::ProjectHandle* projectHandle = new DataAccess::Mining::ProjectHandle ( database, name, accessMode );

   initialiseDomainPropertyFactory ( projectHandle );

   return projectHandle;
}

DataAccess::Mining::DomainPropertyCollection* DataAccess::Mining::DomainPropertyFactory::produceDomainPropertyCollection ( Interface::ProjectHandle* projectHandle ) {
   return new DomainPropertyCollection ( projectHandle );
}


void DataAccess::Mining::DomainPropertyFactory::initialiseDomainPropertyFactory ( Interface::ProjectHandle* handle ) {

   if ( handle == 0 ) {
      std::cerr << " The project-handle has not been set." << std::endl;
      std::exit ( 1 );
   }

   m_projectHandle = handle;


   unsigned int i;
   const Interface::Property* property;


   //Genex5 Properties
   using namespace CBMGenerics;
   ComponentManager & theComponentManager = ComponentManager::getInstance();
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      property = m_projectHandle->findProperty ( theComponentManager.GetSpeciesOutputPropertyName( i ));

      if ( not containsAllocator ( property )) {
         m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );
      }

   }

   for(i = 0; i < GenexResultManager::NumberOfResults; ++i)
   {
      property = m_projectHandle->findProperty ( theResultManager.GetResultName( i ));

      if ( not containsAllocator ( property )) {
         m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );
      }

   }

   property = m_projectHandle->findProperty ( "AllochthonousLithology" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "BulkDensity" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Depth" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "DepthHighRes" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Diffusivity" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "ErosionFactor" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "FaultElements" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "FCTCorrection" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "FluidVelocity" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "FluidVelocityX" );

   if ( not containsAllocator ( property )) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
   }

   property = m_projectHandle->findProperty ( "FluidVelocityY" );

   if ( not containsAllocator ( property )) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
   }

   property = m_projectHandle->findProperty ( "FluidVelocityZ" );

   if ( not containsAllocator ( property )) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
   }

   property = m_projectHandle->findProperty ( "HeatFlow" );

   if ( not containsAllocator ( property )) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
   }

   property = m_projectHandle->findProperty ( "HeatFlowX" );

   if ( not containsAllocator ( property )) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
   }

   property = m_projectHandle->findProperty ( "HeatFlowY" );

   if ( not containsAllocator ( property )) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
   }

   property = m_projectHandle->findProperty ( "HeatFlowZ" );

   if ( not containsAllocator ( property )) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
   }


#if 0
   INTERFACE::PropertyList* formationProperties = m_projectHandle->getProperties ( false, FORMATION | FORMATIONSURFACE, 0, 0, 0, 0 ); // , VOLUME
   INTERFACE::PropertyList::const_iterator formationPropIter;

   for ( formationPropIter = formationProperties->begin (); formationPropIter != formationProperties->end (); ++formationPropIter ) {
      m_allocators [ property ] = produceFormationPropertyAllocator ( handle, *formationPropIter );
   }
#endif

   property = m_projectHandle->findProperty ( "HopaneIsomerisation" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "HorizontalPermeability" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "HydroStaticPressure" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "IlliteFraction" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Lithology" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "LithoStaticPressure" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "MassFlux" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "MassFluxX" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "MassFluxY" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "MassFluxZ" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "MaxVesHighRes" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "MaxVes" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Overburden" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "OverPressure" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "PermeabilityH" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Permeability" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Porosity" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Pressure" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Reflectivity" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "SonicVelocity" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "SteraneAromatisation" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "SteraneIsomerisation" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Temperature" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "ThCond" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "ThicknessError" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "ThicknessHighRes" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Thickness" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Velocity" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "VesHighRes" );
   m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Ves" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Vre" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "Vr" );
   m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );


   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      property = m_projectHandle->findProperty ( pvtFlash::ComponentIdNames [ i ] + "Concentration" );
      m_allocators [ property ] = produceFormationConstantPropertyAllocator ( handle, property );
   }

   property = m_projectHandle->findProperty ( "WaterSaturation" );
   m_allocators [ property ] = produceFormationConstantPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "OilSaturation" );
   m_allocators [ property ] = produceFormationConstantPropertyAllocator ( handle, property );

   property = m_projectHandle->findProperty ( "GasSaturation" );
   m_allocators [ property ] = produceFormationConstantPropertyAllocator ( handle, property );


   // for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   // {
   //    property = m_projectHandle->findProperty ( theComponentManager.GetSpeciesOutputPropertyName( i ));
   //    m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );
   // }

   // for(i = 0; i < GenexResultManager::NumberOfResults; ++i)
   // {
   //    property = m_projectHandle->findProperty ( theResultManager.GetResultName( i ));
   //    m_allocators [ property ] = produceSurfacePropertyAllocator ( handle, property );
   // }


   Interface::PropertyList* reservoirProperties = m_projectHandle->getProperties ( false, RESERVOIR );
   Interface::PropertyList::const_iterator resPropIter;


   for ( resPropIter = reservoirProperties->begin (); resPropIter != reservoirProperties->end (); ++resPropIter ) {
      property = *resPropIter;
      m_allocators [ property ] = produceReservoirPropertyAllocator ( handle, property );
   }

   delete reservoirProperties;
}

DataAccess::Mining::DomainPropertyFactory::~DomainPropertyFactory () {

   PropertyToDomainPropertyAllocator::iterator allocIter;

   for ( allocIter = m_allocators.begin (); allocIter != m_allocators.end (); ++allocIter ) {
      delete allocIter->second;
   }

}

DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainPropertyFactory::allocate ( const DomainPropertyCollection*  collection,
                                                                                          const Interface::Snapshot* snapshot,
                                                                                          const Interface::Property* property ) const {

   PropertyToDomainPropertyAllocator::const_iterator allocIter = m_allocators.find ( property );
   // should we check that the project-handle is the same,
   // i.e. collection->getProjectHandle () == m_projectHandle.

   if ( allocIter != m_allocators.end ()) {
      return allocIter->second->allocate ( collection, snapshot, property );
   } else {
      return 0;
   }

}

bool DataAccess::Mining::DomainPropertyFactory::containsAllocator ( const Interface::Property* property ) const {

   PropertyToDomainPropertyAllocator::const_iterator allocIter = m_allocators.find ( property );

   return allocIter != m_allocators.end ();
}

void DataAccess::Mining::DomainPropertyFactory::addAllocator ( const Interface::Property* property,
                                                               DomainPropertyAllocator*   allocator ) {

   PropertyToDomainPropertyAllocator::iterator allocIter = m_allocators.find ( property );

   if ( allocIter == m_allocators.end ()) {
      m_allocators [ property ] = allocator;
   } else {
      DomainPropertyAllocator* oldAllocator = allocIter->second;
      m_allocators [ property ] = allocator;
      delete oldAllocator;
   }

}

DataAccess::Mining::DomainPropertyAllocator* DataAccess::Mining::DomainPropertyFactory::produceFormationPropertyAllocator ( const Interface::ProjectHandle* projectHandle,
                                                                                                                            const Interface::Property*     property ) {
//    assert ( property->getType () == Interface::FORMATIONPROPERTY );
   DataAccess::Mining::DomainPropertyAllocator* allocator;

   // How better to handle this?
   if ( property->getName () == "Porosity" ) {
      allocator = new PorosityCalculatorAllocator;
   } else if ( property->getName () == "Permeability" ) {
      allocator = new PermeabilityCalculatorAllocator ( true );
   } else if ( property->getName () == "HorizontalPermeability" ) {
      allocator = new PermeabilityCalculatorAllocator ( false );
   } else {
      allocator = new DomainFormationPropertyAllocator;
   }

   return allocator;
   // return new DomainFormationPropertyAllocator;
}

DataAccess::Mining::DomainPropertyAllocator* DataAccess::Mining::DomainPropertyFactory::produceSurfacePropertyAllocator ( const Interface::ProjectHandle* projectHandle,
                                                                                                                          const Interface::Property*     property ) {
//    assert ( property->getType () == Interface::SURFACEPROPERTY );

   return new DomainSurfacePropertyAllocator;
}

DataAccess::Mining::DomainPropertyAllocator* DataAccess::Mining::DomainPropertyFactory::produceFormationConstantPropertyAllocator ( const Interface::ProjectHandle* projectHandle,
                                                                                                                                    const Interface::Property*     property ) {
   return new ConstantDomainFormationPropertyAllocator;
}

DataAccess::Mining::DomainPropertyAllocator* DataAccess::Mining::DomainPropertyFactory::produceReservoirPropertyAllocator ( const Interface::ProjectHandle* projectHandle,
                                                                                                                            const Interface::Property*     property ) {
   assert ( property->getType () == Interface::RESERVOIRPROPERTY );
   return new DomainReservoirPropertyAllocator;
}

DataAccess::Mining::CauldronDomain* DataAccess::Mining::DomainPropertyFactory::produceCauldronDomain ( Interface::ProjectHandle* projectHandle ) {
   return new CauldronDomain ( projectHandle );
}
