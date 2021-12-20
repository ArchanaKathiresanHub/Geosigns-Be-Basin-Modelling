//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DataMiningObjectFactory.h"

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
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

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
   , "GammaRay"
   , "HorizontalPermeability"
   , "HydroStaticPressure"
   , "Lithology"
   , "LithoStaticPressure"
   , "MaxVes"
   , "MaxVesHighRes"
   , "Overburden"
   , "OverPressure"
   , "Permeability"
   , "Porosity"
   , "Pressure"
   , "Reflectivity"
   , "SonicSlowness"
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

   DataAccess::Interface::ProjectHandle* ObjectFactory::produceProjectHandle( database::ProjectFileHandlerPtr pfh,
                                                                                      const std::string  & name ) const
   {
      ProjectHandle * projectHandle = new ProjectHandle ( pfh, name, this );
      return projectHandle;
   }

   DomainPropertyCollection * ObjectFactory::produceDomainPropertyCollection ( Interface::ProjectHandle* projectHandle ) const
   {
      return new DomainPropertyCollection( projectHandle );
   }

   void ObjectFactory::initialiseObjectFactory( Interface::ProjectHandle* handle )
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

      for ( unsigned int i = 0; i < ComponentManager::NUMBER_OF_SPECIES; ++i )
      {
         property = m_projectHandle->findProperty( theComponentManager.getSpeciesOutputPropertyName( i ) );
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
      for ( unsigned int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
      {
         property = m_projectHandle->findProperty( ComponentManager::getInstance().getSpeciesName( i ) + "Concentration" );
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


   ObjectFactory::~ObjectFactory()
   {
      for (  PropertyToDomainPropertyAllocator::iterator allocIter = m_allocators.begin(); allocIter != m_allocators.end(); ++allocIter )
      {
         delete allocIter->second;
      }
   }


   DomainProperty* ObjectFactory::allocate ( const DomainPropertyCollection*            collection,
                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                     const Interface::Snapshot*                 snapshot,
                                                     const Interface::Property*                 property ) const
   {
      PropertyToDomainPropertyAllocator::const_iterator allocIter = m_allocators.find( property );
      // should we check that the project-handle is the same,
      // i.e. collection->getProjectHandle () == m_projectHandle->

      if ( allocIter != m_allocators.end() )
      {
         return allocIter->second->allocate ( collection, propertyManager, snapshot, property );
      }
      else
      {
         return 0;
      }

   }


   bool ObjectFactory::containsAllocator( const Interface::Property* property ) const
   {
      PropertyToDomainPropertyAllocator::const_iterator allocIter = m_allocators.find ( property );

      return allocIter != m_allocators.end ();
   }


   void ObjectFactory::addAllocator ( const Interface::Property * property,
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


   DomainPropertyAllocator * ObjectFactory::produceFormationPropertyAllocator( const Interface::ProjectHandle * ,
                                                                               const Interface::Property      * property ) const
   {
      DomainPropertyAllocator * allocator;

      // How better to handle this?
      if (      property->getName() == "Porosity"               ) { allocator = new PorosityCalculatorAllocator;              }
      else if ( property->getName() == "Permeability"           ) { allocator = new PermeabilityCalculatorAllocator( true );  }
      else if ( property->getName() == "HorizontalPermeability" ) { allocator = new PermeabilityCalculatorAllocator( false ); }
      else                                                        { allocator = new DomainFormationPropertyAllocator;         }

      return allocator;
   }


   DomainPropertyAllocator * ObjectFactory::produceFormationMapPropertyAllocator( const Interface::ProjectHandle * ,
                                                                                  const Interface::Property      *  ) const
   {
      return new DomainFormationMapPropertyAllocator;
   }

   DomainPropertyAllocator* ObjectFactory::produceSurfacePropertyAllocator( const Interface::ProjectHandle * ,
                                                                            const Interface::Property      * ) const
   {
      return new DomainSurfacePropertyAllocator;
   }

   DomainPropertyAllocator * ObjectFactory::produceFormationConstantPropertyAllocator( const Interface::ProjectHandle * ,
                                                                                       const Interface::Property      *  ) const
   {
      return new ConstantDomainFormationPropertyAllocator;
   }

   DomainPropertyAllocator* ObjectFactory::produceReservoirPropertyAllocator( const Interface::ProjectHandle * ,
                                                                              const Interface::Property      * property ) const
   {
      assert ( property->getType () == Interface::RESERVOIRPROPERTY );
      return new DomainReservoirPropertyAllocator;
   }

   CauldronDomain* ObjectFactory::produceCauldronDomain( Interface::ProjectHandle* projectHandle ) const
   {
      return new CauldronDomain ( projectHandle );
   }
}}
