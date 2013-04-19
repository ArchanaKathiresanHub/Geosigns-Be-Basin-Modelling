#include "StatisticsHandler.h"

#include <sstream>
#include <unistd.h>

#include "NumericFunctions.h"
#include "FastcauldronSimulator.h"
#include "utils.h"

double StatisticsHandler::s_virtualMemoryUsage = 0.0;
double StatisticsHandler::s_residentMemoryUsage = 0.0;

void StatisticsHandler::initialise () {
   s_virtualMemoryUsage = 0.0;
   s_residentMemoryUsage = 0.0;
}

void StatisticsHandler::update () {

   StatM stat;

   getStatM ( stat );
   s_virtualMemoryUsage = NumericFunctions::Maximum ( s_virtualMemoryUsage, double ( stat.size ) * double ( getpagesize ()));
   s_residentMemoryUsage = NumericFunctions::Maximum ( s_residentMemoryUsage, double ( stat.resident ) * double ( getpagesize ()));

}

void StatisticsHandler::print () {


  std::stringstream buffer;

  buffer << "  <memory_usage>" << endl;
  buffer << "    <rank> " << FastcauldronSimulator::getInstance ().getRank () << " </rank>" << endl;
  buffer << "    <virtual> " << s_virtualMemoryUsage << " </virtual>" << endl;
  buffer << "    <resident> " << s_residentMemoryUsage << " </resident>" << endl;
  buffer << "  </memory_usage>" << endl;

  PetscPrintf ( PETSC_COMM_WORLD, "<statistics>\n");
  PetscSynchronizedFlush ( PETSC_COMM_WORLD );

  PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
  PetscSynchronizedFlush ( PETSC_COMM_WORLD );

  PetscPrintf ( PETSC_COMM_WORLD, "</statistics>\n");
  PetscSynchronizedFlush ( PETSC_COMM_WORLD );

}



void StatisticsHandler::getStatM ( StatM& statm ) {

   unsigned long dummy;
   const char* statm_path = "/proc/self/statm";

   FILE *f = fopen(statm_path,"r");

   if(!f){
      perror(statm_path);
   }

   if(7 != fscanf(f,"%ld %ld %ld %ld %ld %ld %ld",
                  &statm.size,&statm.resident,&statm.share,&statm.text,&statm.lib,&statm.data,&statm.dt))
   {
   }
   fclose(f);
}
