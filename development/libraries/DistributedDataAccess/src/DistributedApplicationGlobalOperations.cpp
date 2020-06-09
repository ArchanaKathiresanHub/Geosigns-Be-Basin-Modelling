//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DistributedApplicationGlobalOperations.h"

#include "MpiFunctions.h"
#include "petsc.h"

double DataAccess::Interface::DistributedApplicationGlobalOperations::maximum ( const double val ) const {
   return MpiFunctions::Maximum<double>( PETSC_COMM_WORLD, val );
}

double DataAccess::Interface::DistributedApplicationGlobalOperations::minimum ( const double val ) const {
   return MpiFunctions::Minimum<double>( PETSC_COMM_WORLD, val );
}

double DataAccess::Interface::DistributedApplicationGlobalOperations::sum ( const double val ) const {
   return MpiFunctions::Sum<double>( PETSC_COMM_WORLD, val );
}