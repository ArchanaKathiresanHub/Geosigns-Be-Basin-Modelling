#include <petsc.h>

using namespace std;

#include "PropertiesCalculator.h"

void displayTime ( const double timeToDisplay, const char * msgToDisplay );

int main( int argc, char ** argv )
{

   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);
   int rank;

   MPI_Comm_rank ( PETSC_COMM_WORLD, &rank );

   SnapshotList snapshots;
   PropertyList properties;
   FormationVector formationItems;

   PetscLogDouble sim_Start_Time;
   PetscTime( &sim_Start_Time );   

   GeoPhysics::ObjectFactory* factory = new GeoPhysics::ObjectFactory;
   PropertiesCalculator propCalculator ( rank );
       
   if( !propCalculator.parseCommandLine( argc, argv )) {

      PetscFinalize();
      return 1;
   }

   if( ! propCalculator.CreateFrom(factory) ) {

      propCalculator.showUsage( argv[ 0 ], "Could not open specified project file" );

      PetscFinalize();

      return 1;
   }
 
   propCalculator.printOutputableProperties ();
   propCalculator.acquireAll2Dproperties();
   propCalculator.acquireAll3Dproperties();

   propCalculator.printListSnapshots();
   propCalculator.printListStratigraphy(); 

   if ( propCalculator.showLists() ) {
      propCalculator.finalise ( false );

      PetscFinalize ();

      return 0;
   }

   if ( !propCalculator.startActivity () ) {
      propCalculator.finalise ( false );
 
      PetscFinalize ();

      return 1;
   };
   
   propCalculator.acquireSnapshots( snapshots );
   propCalculator.acquireProperties( properties );
   propCalculator.acquireFormations( formationItems );

   propCalculator.calculateProperties( formationItems, properties, snapshots );
   
   propCalculator.finalise ( true );

   PetscLogDouble sim_End_Time;
   PetscTime( &sim_End_Time );   

   displayTime( sim_End_Time - sim_Start_Time, "End of calculation" );

   PetscFinalize ();
   return 0;
}

