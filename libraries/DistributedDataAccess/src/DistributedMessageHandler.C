#include "Interface/DistributedMessageHandler.h"
#include "petsc.h"

DataAccess::Interface::DistributedMessageHandler::~DistributedMessageHandler () {
}

void DataAccess::Interface::DistributedMessageHandler::print ( const std::string& str ) const {
   PetscPrintf ( PETSC_COMM_WORLD, str.c_str ());
}

void DataAccess::Interface::DistributedMessageHandler::print ( const int val ) const {
   PetscPrintf ( PETSC_COMM_WORLD, "%i", val );
}

void DataAccess::Interface::DistributedMessageHandler::print ( const double val ) const {
   PetscPrintf ( PETSC_COMM_WORLD, "%f", val );
}


void DataAccess::Interface::DistributedMessageHandler::synchronisedPrint ( const std::string& str ) const {
   PetscSynchronizedPrintf ( PETSC_COMM_WORLD, str.c_str ());
}


void DataAccess::Interface::DistributedMessageHandler::synchronisedPrint ( const int val ) const {
   PetscSynchronizedPrintf ( PETSC_COMM_WORLD, "%i", val );
}


void DataAccess::Interface::DistributedMessageHandler::synchronisedPrint ( const double val ) const {
   PetscSynchronizedPrintf ( PETSC_COMM_WORLD, "%f", val );
}



void DataAccess::Interface::DistributedMessageHandler::printLine ( const std::string& str ) const {
   PetscPrintf ( PETSC_COMM_WORLD, "%s\n", str.c_str ());
}


void DataAccess::Interface::DistributedMessageHandler::synchronisedPrintLine ( const std::string& str ) const {
   PetscSynchronizedPrintf ( PETSC_COMM_WORLD, "%s\n", str.c_str ());
}



void DataAccess::Interface::DistributedMessageHandler::newLine () const {
   PetscPrintf ( PETSC_COMM_WORLD, "\n" );
}


void DataAccess::Interface::DistributedMessageHandler::synchronisedNewLine () const {
   PetscSynchronizedPrintf ( PETSC_COMM_WORLD, "\n" );
}

         


void DataAccess::Interface::DistributedMessageHandler::flush () const {
   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
}

