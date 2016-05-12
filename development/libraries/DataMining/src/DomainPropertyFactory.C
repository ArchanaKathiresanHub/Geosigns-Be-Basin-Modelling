//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "DomainPropertyFactory.h"

#include "PropertyAttribute.h"

#include "DomainSurfaceProperty.h"
#include "DomainFormationProperty.h"
#include "DomainFormationMapProperty.h"
#include "ConstantDomainFormationProperty.h"
#include "DomainReservoirProperty.h"

#include "GenexResultManager.h"
#include "ComponentManager.h"

#include "DataMiningProjectHandle.h"
#include "DomainPropertyCollection.h"

#include "PorosityCalculator.h"
#include "PermeabilityCalculator.h"

using namespace CBMGenerics;

static const char * s_SurfacePropList[] =
{
     "AllochthonousLithology"
   , "ErosionFactor"
   , "FaultElements"
   , "FCTCorrection"
   , "ThicknessError"
   , "ThicknessHighRes"
   , "Thickness"
};

static const char * s_FormationPropCheckAllocList[] =
{
     "FluidVelocityX"
   , "FluidVelocityY"
   , "FluidVelocityZ"
   , "HeatFlow"
   , "HeatFlowX"
   , "HeatFlowY"
   , "HeatFlowZ"
};

static const char * s_FormationPropList[] =
{
     "BulkDensity"
   , "ChemicalCompaction"
   , "Depth"
   , "DepthHighRes"
   , "Diffusivity"
   , "FluidVelocity"
   , "HopaneIsomerisation"
   , "HorizontalPermeability"
   , "HydroStaticPressure"
   , "IlliteFraction"
   , "Lithology"
   , "LithoStaticPressure"
   , "MaxVes"
   , "MaxVesHighRes"
   , "Overburden"
   , "OverPressure"
   , "PermeabilityH"
   , "Permeability"
   , "Porosity"
   , "Pressure"
   , "Reflectivity"
   , "SonicSlowness"
   , "SteraneAromatisation"
   , "SteraneIsomerisation"
   , "Temperature"
   , "ThCond"
   , "Velocity"
   , "TwoWayTime"
   , "Ves"
   , "VesHighRes"
   , "Vre"
   , "Vr"
};

namespace DataAccess { namespace Mining
{

   DataAccess::Interface::ProjectHandle* DomainPropertyFactory::produceProjectHandle( database::Database * database, 
                                                                                      const std::string  & name,
                                                                                      const std::string  & accessMode )
   {
      ProjectHandle * projectHandle = new ProjectHandle( database, name, accessMode, this );
      initialiseDomainPropertyFactory( projectHandle );
      return projectHandle;
   }

   DomainPropertyCollection * DomainPropertyFactory::produceDomainPropertyCollection ( Interface::ProjectHandle* projectHandle )
   {
      return new DomainPropertyCollection( projectHandle );
   }

   void DomainPropertyFactory::initialiseDomainPropertyFactory( Interface::ProjectHandle* handle )
   {
      if ( handle == 0 )
      {
         std::cerr << " The project-handle has not been set." << std::endl;
         std::exit ( 1 );
      }
      m_projectHandle = handle;

      const Interface::Property * property;

      //Genex5 Properties
      ComponentManager   & theComponentManager = ComponentManager::getInstance();
      GenexResultManager & theResultManager    = GenexResultManager::getInstance();

      for ( unsigned int i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i )
      {
         property = m_projectHandle->findProperty( theComponentManager.GetSpeciesOutputPropertyName( i ) );
         if ( not containsAllocator( property ) )
         {
            m_allocators [ property ] = produceSurfacePropertyAllocator( handle, property );
         }
      }

      for( unsigned int i = 0; i < GenexResultManager::NumberOfResults; ++i )
      {
         property = m_projectHandle->findProperty( theResultManager.GetResultName( i ));

         if ( not containsAllocator( property ) )
         {
            m_allocators [ property ] = produceFormationMapPropertyAllocator( handle, property );
         }
      }

      // add surface properties
      for ( unsigned int i = 0; i < sizeof( s_SurfacePropList )/sizeof( const char *); ++i )
      {
         property = m_projectHandle->findProperty( s_SurfacePropList[i] );

         if ( property != 0 ) {

            if ( property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) {
               m_allocators [ property ] = produceSurfacePropertyAllocator( handle, property );
            } else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY ) {
               m_allocators [ property ] = produceFormationMapPropertyAllocator( handle, property );
            } else {
               std::cerr << " The property " << property->getName () << " is neither a FORMATION_2D_PROPERTY nor a SURFACE_2D_PROPERTY." << std::endl;
            }

         } else {
            std::cerr << " The property " << s_SurfacePropList[i] << " cannot be found." << std::endl;
         }

      }

      // add formation properties
      for ( unsigned int i = 0; i < sizeof( s_FormationPropList ) / sizeof( const char* ); ++i )
      {
         property = m_projectHandle->findProperty( s_FormationPropList[i] );
         m_allocators [ property ] = produceFormationPropertyAllocator( handle, property );
      }

      // add formation properties but check first, was allocator already added
      for ( unsigned int i = 0; i < sizeof( s_FormationPropCheckAllocList ) / sizeof( const char* ); ++i )
      {
         property = m_projectHandle->findProperty( s_FormationPropCheckAllocList[i] );
         if ( not containsAllocator ( property ) )
         {
            m_allocators [ property ] = produceFormationPropertyAllocator ( handle, property );
         }
      }

      // add concentrations for HC species
      for ( unsigned int i = 0; i < pvtFlash::NUM_COMPONENTS; ++i )
      {
         property = m_projectHandle->findProperty( pvtFlash::ComponentIdNames[ i ] + "Concentration" );
         m_allocators [ property ] = produceFormationConstantPropertyAllocator ( handle, property );
      }

      property = m_projectHandle->findProperty ( "WaterSaturation" );
      m_allocators [ property ] = produceFormationConstantPropertyAllocator( handle, property );

      property = m_projectHandle->findProperty ( "OilSaturation" );
      m_allocators [ property ] = produceFormationConstantPropertyAllocator( handle, property );

      property = m_projectHandle->findProperty ( "GasSaturation" );
      m_allocators [ property ] = produceFormationConstantPropertyAllocator( handle, property );

      Interface::PropertyList* reservoirProperties = m_projectHandle->getProperties( false, RESERVOIR );
      Interface::PropertyList::const_iterator resPropIter;

      // add reservoir porperties
      for ( resPropIter = reservoirProperties->begin(); resPropIter != reservoirProperties->end(); ++resPropIter )
      {
         property = *resPropIter;
         m_allocators [ property ] = produceReservoirPropertyAllocator ( handle, property );
      }

      delete reservoirProperties;
   }

   
   DomainPropertyFactory::~DomainPropertyFactory()
   {
      for (  PropertyToDomainPropertyAllocator::iterator allocIter = m_allocators.begin(); allocIter != m_allocators.end(); ++allocIter )
      {
         delete allocIter->second;
      }
   }

   
   DomainProperty* DomainPropertyFactory::allocate ( const DomainPropertyCollection*            collection,
                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                     const Interface::Snapshot*                 snapshot,
                                                     const Interface::Property*                 property ) const
   {
      PropertyToDomainPropertyAllocator::const_iterator allocIter = m_allocators.find( property );
      // should we check that the project-handle is the same,
      // i.e. collection->getProjectHandle () == m_projectHandle.
      
      if ( allocIter != m_allocators.end() )
      {
         return allocIter->second->allocate ( collection, propertyManager, snapshot, property );
      }
      else
      {
         return 0;
      }

   }


   bool DomainPropertyFactory::containsAllocator( const Interface::Property* property ) const 
   {
      PropertyToDomainPropertyAllocator::const_iterator allocIter = m_allocators.find ( property );

      return allocIter != m_allocators.end ();
   }


   void DomainPropertyFactory::addAllocator ( const Interface::Property * property,
                                              DomainPropertyAllocator   * allocator )
   {
      PropertyToDomainPropertyAllocator::iterator allocIter = m_allocators.find ( property );

      if ( allocIter == m_allocators.end() )
      {
         m_allocators [ property ] = allocator;
      }
      else
      {
         DomainPropertyAllocator* oldAllocator = allocIter->second;
         m_allocators [ property ] = allocator;
         delete oldAllocator;
      }
   }


   DomainPropertyAllocator * DomainPropertyFactory::produceFormationPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                                       const Interface::Property      * property )
   {
      DomainPropertyAllocator * allocator;

      // How better to handle this?
      if (      property->getName() == "Porosity"               ) { allocator = new PorosityCalculatorAllocator;              }
      else if ( property->getName() == "Permeability"           ) { allocator = new PermeabilityCalculatorAllocator( true );  }
      else if ( property->getName() == "HorizontalPermeability" ) { allocator = new PermeabilityCalculatorAllocator( false ); }
      else                                                        { allocator = new DomainFormationPropertyAllocator;         }

      return allocator;
   }


   DomainPropertyAllocator * DomainPropertyFactory::produceFormationMapPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                                          const Interface::Property      * property )
   {
      return new DomainFormationMapPropertyAllocator;
   }

   DomainPropertyAllocator* DomainPropertyFactory::produceSurfacePropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                                     const Interface::Property      * property )
   {
   //    assert ( property->getType () == Interface::SURFACEPROPERTY );
      return new DomainSurfacePropertyAllocator;
   }

   DomainPropertyAllocator * DomainPropertyFactory::produceFormationConstantPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                                               const Interface::Property      * property )
   {
      return new ConstantDomainFormationPropertyAllocator;
   }

   DomainPropertyAllocator* DomainPropertyFactory::produceReservoirPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                                       const Interface::Property      * property )
   {
      assert ( property->getType () == Interface::RESERVOIRPROPERTY );
      return new DomainReservoirPropertyAllocator;
   }

   CauldronDomain* DomainPropertyFactory::produceCauldronDomain( Interface::ProjectHandle* projectHandle )
   {
      return new CauldronDomain ( projectHandle );
   }
}}

