#include <petsc.h>

using namespace std;

#include "PropertiesCalculator.h"

int main( int argc, char ** argv )
{

   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);
   int rank;

   MPI_Comm_rank ( PETSC_COMM_WORLD, &rank );

   SnapshotList snapshots;
   PropertyList properties;
   FormationVector formationItems;

   GeoPhysics::ObjectFactory* factory = new GeoPhysics::ObjectFactory;
   DataAccess::Interface::ProjectHandle::UseFactory (factory);

   PropertiesCalculator propCalculator;
       
   if( !propCalculator.parseCommandLine( argc, argv )) {

      PetscFinalize();
      return -1;
   }

   if( ! propCalculator.CreateFrom( rank ) ) {

      propCalculator.showUsage( argv[ 0 ], "Could not open specified project file" );
      PetscFinalize();
      return -1;
   }
 
   propCalculator.printOutputableProperties ();
   propCalculator.acquireAll2Dproperties();
   propCalculator.acquireAll3Dproperties();

   propCalculator.printListSnapshots();
   propCalculator.printListStratigraphy(); 

   if ( propCalculator.showLists() )
   {
      propCalculator.finalise ( false );
      return 0;
   }

   if ( !propCalculator.startActivity () ) {
      propCalculator.finalise ( false );
      return 1;
   };
   
   propCalculator.acquireSnapshots( snapshots );
   propCalculator.acquireProperties( properties );
   propCalculator.acquireFormations( formationItems );

   propCalculator.calculateProperties( formationItems, properties, snapshots );
   
   propCalculator.finalise ( true );

   return 0;
}

