#include "StatisticsHandler.h"

#include <sstream>

#include "NumericFunctions.h"
#include "FastcauldronSimulator.h"
#include "utils.h"
#include "System.h"

double StatisticsHandler::s_virtualMemoryUsage = 0.0;
double StatisticsHandler::s_residentMemoryUsage = 0.0;

void StatisticsHandler::initialise () {
   s_virtualMemoryUsage = 0.0;
   s_residentMemoryUsage = 0.0;
}

void StatisticsHandler::update () {

#ifdef _MSC_VER // TODO_SK
   s_virtualMemoryUsage = 0;  
   s_residentMemoryUsage = 0;
#else
   StatM stat;

   getStatM ( stat );
   s_virtualMemoryUsage = NumericFunctions::Maximum ( s_virtualMemoryUsage, double ( stat.size ) * double ( getpagesize ()));
   s_residentMemoryUsage = NumericFunctions::Maximum ( s_residentMemoryUsage, double ( stat.resident ) * double ( getpagesize ()));
#endif

}

void StatisticsHandler::print () {


  std::stringstream buffer;

  buffer << "  <memory_usage>" << endl;
  buffer << "    <rank> " << FastcauldronSimulator::getInstance ().getRank () << " </rank>" << endl;
  buffer << "    <virtual> " << s_virtualMemoryUsage << " </virtual>" << endl;
  buffer << "    <resident> " << s_residentMemoryUsage << " </resident>" << endl;
  buffer << "  </memory_usage>" << endl;

  PetscPrintf ( PETSC_COMM_WORLD, "<statistics>\n");
  PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );

  PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
  PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );

  PetscPrintf ( PETSC_COMM_WORLD, "</statistics>\n");
  PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );

}
