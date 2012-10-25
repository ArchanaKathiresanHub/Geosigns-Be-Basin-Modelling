#include "Interface/DistributedApplicationGlobalOperations.h"

#include "MpiFunctions.h"
#include "petsc.h"

DataAccess::Interface::DistributedApplicationGlobalOperations::~DistributedApplicationGlobalOperations () {
}

double DataAccess::Interface::DistributedApplicationGlobalOperations::maximum ( const double val ) const {
   return MpiFunctions::Maximum<double>( PETSC_COMM_WORLD, val );
}

double DataAccess::Interface::DistributedApplicationGlobalOperations::minimum ( const double val ) const {
   return MpiFunctions::Minimum<double>( PETSC_COMM_WORLD, val );
}

double DataAccess::Interface::DistributedApplicationGlobalOperations::sum ( const double val ) const {
   return MpiFunctions::Sum<double>( PETSC_COMM_WORLD, val );
}
