//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataMinerImpl.C
/// @brief This file keeps implementation of API which provides access to simulation results

#include "DataMinerImpl.h"

namespace mbapi
{
   DataMinerImpl::DataMinerImpl()
   {
   }



}

Mining::DomainPropertyFactory* factory = new DataAccess::Mining::DomainPropertyFactory;

   Interface::ProjectHandle::UseFactory ( factory );

   Mining::ProjectHandle* projectHandle = (Mining::ProjectHandle*)(OpenCauldronProject (inputProjectFileName, "r"));

   projectHandle->startActivity ( "datadriller", projectHandle->getLowResolutionOutputGrid ());
   projectHandle->initialise ( true, false );

   projectHandle->setFormationLithologies (false, false);

   CauldronDomain domain ( projectHandle );

   DomainPropertyCollection* domainProperties = projectHandle->getDomainPropertyCollection ();

   database::Table * table = projectHandle->getTable ("DataMiningIoTbl");

   const Interface::Grid * grid = projectHandle->getLowResolutionOutputGrid ();


