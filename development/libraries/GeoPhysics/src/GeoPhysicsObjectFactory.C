//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GeoPhysicsObjectFactory.h"

#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsFormation.h"

#include "GeoPhysicsCrustFormation.h"
#include "GeoPhysicsMantleFormation.h"
#include "GeoPhysicsSourceRock.h"

#include "Interface/CrustFormation.h"
#include "Interface/MantleFormation.h"

#include "database.h"
#include "ProjectFileHandler.h"
#include "cauldronschemafuncs.h"

#include "BasementLithology.h"

using namespace DataAccess;

Interface::ProjectHandle * GeoPhysics::ObjectFactory::produceProjectHandle ( database::ProjectFileHandlerPtr pfh,
                                                                             const string & name,
                                                                             const string & accessMode ) const
{
   return new GeoPhysics::ProjectHandle ( pfh, name, accessMode, this );
}

Interface::FluidType* GeoPhysics::ObjectFactory::produceFluidType ( Interface::ProjectHandle* projectHandle,
                                                                    database::Record*         record ) const
{
   return new GeoPhysics::FluidType ( projectHandle, record );
}

DataAccess::Interface::Formation* GeoPhysics::ObjectFactory::produceFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                                database::Record*                     record ) const
{
   return new GeoPhysics::Formation ( projectHandle, record );
}

DataAccess::Interface::CrustFormation* GeoPhysics::ObjectFactory::produceCrustFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                                          database::Record*                     record ) const
{


   GeoPhysics::GeoPhysicsCrustFormation * result = new GeoPhysics::GeoPhysicsCrustFormation ( projectHandle, record );

   return result;
}

DataAccess::Interface::MantleFormation* GeoPhysics::ObjectFactory::produceMantleFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                                            database::Record*                     record ) const
{
   GeoPhysics::GeoPhysicsMantleFormation * result = new GeoPhysics::GeoPhysicsMantleFormation ( projectHandle, record );

   return result;
}

DataAccess::Interface::SourceRock* GeoPhysics::ObjectFactory::produceSourceRock ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                                  database::Record*                     record ) const
{
   GeoPhysics::GeoPhysicsSourceRock * result = new GeoPhysics::GeoPhysicsSourceRock ( projectHandle, record );

   return result;
}

Interface::LithoType* GeoPhysics::ObjectFactory::produceLithoType( DataAccess::Interface::ProjectHandle * projectHandle,
                                                                    database::Record *         record ) const
{
   const string lithoname = database::getLithotype (record);
   if ((projectHandle->isALC()) &&
      (lithoname == projectHandle->getCrustLithoName() || lithoname == projectHandle->getMantleLithoName() || lithoname == DataAccess::Interface::ALCBasalt))
   {
      return new BasementLithology( projectHandle, record );
   }
   else
   {
      return new SimpleLithology ( projectHandle, record );
   }
}

Interface::LithoType* GeoPhysics::ObjectFactory::produceLithoType ( const SimpleLithology* litho,
                                                                    const std::string&     newName ) const
{
   SimpleLithology * copy = new SimpleLithology ( *litho );
   copy->setName(newName);
   return copy;
}

Interface::LithoType* GeoPhysics::ObjectFactory::produceLithoType ( const SimpleLithology*      litho,
                                                                    const std::string&          newName,
                                                                    const double                permeabilityAnisotropy,
                                                                    const std::vector<double> & porosityPercentageSamples,
                                                                    const std::vector<double> & permeabilitySamples ) const
{
   SimpleLithology * copy = new SimpleLithology(*litho);
   copy->setPermeability( newName, permeabilityAnisotropy, porosityPercentageSamples, permeabilitySamples);
   return copy;
}

GeoPhysics::CompoundLithology* GeoPhysics::ObjectFactory::produceCompoundLithology ( GeoPhysics::ProjectHandle* projectHandle ) const
{
   return new CompoundLithology ( projectHandle );
}

GeoPhysics::FracturePressureCalculator* GeoPhysics::ObjectFactory::produceFracturePressureCalculator ( GeoPhysics::ProjectHandle* projectHandle ) const
{
   return new FracturePressureCalculator ( projectHandle );
}
