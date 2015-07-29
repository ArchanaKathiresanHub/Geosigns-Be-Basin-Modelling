#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <petsc.h>

#include "PropertiesCalculator.h"
#include "Interface/SimulationDetails.h"

static bool splitString( char * string, char separator, char * & firstPart, char * & secondPart );
static bool parseStrings( StringVector & strings, char * stringsString );
static bool parseAges( DoubleVector & ages, char * agesString );


static bool snapshotSorter( const Snapshot * snapshot1, const Snapshot * snapshot2 );
static bool snapshotIsEqual( const Snapshot * snapshot1, const Snapshot * snapshot2 );

//------------------------------------------------------------//

PropertiesCalculator::PropertiesCalculator( int aRank ) {

   m_rank = aRank;

   m_debug = false;
   m_basement = false; 
   m_all2Dproperties = false;
   m_all3Dproperties = false;
   m_listProperties = false;
   m_listSnapshots = false;
   m_listStratigraphy = false;
   m_snapshotsType = MAJOR;

   m_projectFileName = "";
   m_simulationMode = "";
   m_activityName = "";

   m_projectHandle = 0; 
   m_propertyManager = 0;

}

//------------------------------------------------------------//

PropertiesCalculator::~PropertiesCalculator() {

   if(  m_propertyManager != 0 ) delete m_propertyManager;

   if( m_projectHandle != 0 ) delete m_projectHandle;

   m_propertyManager = 0;
   m_projectHandle   = 0;
}

//------------------------------------------------------------//

void  PropertiesCalculator::finalise ( bool isComplete ) {

   m_projectHandle->setSimulationDetails ( "fastproperties", "Default", "" );
   m_projectHandle->finishActivity ( isComplete );

   if( isComplete && m_rank == 0 ) {
      m_projectHandle->saveToFile(m_projectFileName);

   }
   delete m_propertyManager;
   m_propertyManager = 0;

   delete m_projectHandle;
   m_projectHandle = 0;
}

//------------------------------------------------------------//

bool PropertiesCalculator::CreateFrom ( DataAccess::Interface::ObjectFactory* factory ){
   
    if ( m_projectHandle == 0 ) {
      m_projectHandle = ( GeoPhysics::ProjectHandle* )( OpenCauldronProject( m_projectFileName, "r", factory ) );

      if(  m_projectHandle != 0 ) {
         m_propertyManager = new DerivedPropertyManager ( m_projectHandle, m_debug );
      }
   }
   if(  m_projectHandle == 0 ||  m_propertyManager == 0 ) {
      return false;
   }
   return true;

}
//------------------------------------------------------------//

bool PropertiesCalculator::startActivity() {
   
   if( !setFastcauldronActivityName() ) {
      return false;
   }
   
   const Interface::Grid * grid = m_projectHandle->getLowResolutionOutputGrid();

   bool started = m_projectHandle->startActivity ( m_activityName, grid, false, true, m_activityName != "Fastproperties" );

   // If this table is not present the assume that the last
   // fastcauldron mode was not pressure mode.
   // This table may not be present because we are running c2e on an old 
   // project, before this table was added.
   bool coupledCalculation = false; 
 
   coupledCalculation = m_simulationMode == "Overpressure" or
      m_simulationMode == "LooselyCoupledTemperature" or
      m_simulationMode == "CoupledHighResDecompaction" or
      m_simulationMode == "CoupledPressureAndTemperature" or
      m_simulationMode == "CoupledDarcy";

   if( started ) {
      m_projectHandle->initialise ( coupledCalculation );
   }

   if( started ) {
      started = m_projectHandle->setFormationLithologies ( true, true );
   }
   if( started) {
      started = m_projectHandle->initialiseLayerThicknessHistory ( coupledCalculation );
   }

   return started;
}

//------------------------------------------------------------//
bool PropertiesCalculator::showLists() {
 
   return m_listProperties || m_listSnapshots || m_listStratigraphy;
 
}
//------------------------------------------------------------//
void PropertiesCalculator::calculateProperties( FormationVector& formationItems, PropertyList properties, SnapshotList & snapshots )  {

   SnapshotList::iterator snapshotIter;
   PropertyList::iterator propertyIter;
   FormationVector::iterator formationIter;

   SnapshotFormationOutputPropertyValueMap allOutputPropertyValues;

   bool zeroSnapshotAdded = false;
   if( snapshots.empty() ) {
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot( 0 );
      
      snapshots.push_back( zeroSnapshot ); 
      zeroSnapshotAdded = true;
   }

   const string outputDirName = m_projectHandle->getFullOutputDir () + "/";
   struct stat fileStatus;
   int fileError;
    
   for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;

      if ( snapshot->getFileName () != "" ) {
         const string fileName = outputDirName + snapshot->getFileName ();
         fileError = stat ( fileName.c_str(), &fileStatus );
         
         ((Snapshot *)snapshot)->setAppendFile ( not fileError );
      }

      for ( formationIter = formationItems.begin(); formationIter != formationItems.end(); ++formationIter )
      {
         const Formation * formation = *formationIter;

         for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter )
         {

            const Property * property = *propertyIter;
            OutputPropertyValuePtr outputProperty = allocateOutputProperty ( * m_propertyManager, property, snapshot, formation );

            if ( outputProperty != 0 ) {
               if( m_debug && m_rank == 0 ) {
                  cout << "Snapshot: " << snapshot->getTime() << " allocate " << property->getName() << " " << formation->getName() << endl;
               }
               allOutputPropertyValues [ snapshot ][ formation ][ property ] = outputProperty;
            }

         }
      }
   }

   if( zeroSnapshotAdded ) {
      snapshots.pop_back(); // to remove the explicitly added snapshot age 0
   }

   for ( formationIter = formationItems.begin();  formationIter != formationItems.end(); ++formationIter )
   {
      const Formation * formation = *formationIter;

      for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
      {
         const Snapshot * snapshot = *snapshotIter;

         outputSnapshotFormationData( snapshot, formation, properties, allOutputPropertyValues );
         m_projectHandle->continueActivity();
      }
   }
}

//------------------------------------------------------------//

bool PropertiesCalculator::acquireSnapshots( SnapshotList & snapshots )
{
   if ( m_ages.size() == 0 )
   {
      SnapshotList * allSnapshots = m_projectHandle->getSnapshots( m_snapshotsType );
      snapshots = *allSnapshots;
      return true;
   }
   else
   {
      int index;
      double firstAge = -1;
      double secondAge = -1;
      for ( index = 0; index < m_ages.size(); ++index )
      {
         if ( m_ages[ index ] >= 0 )
         {
            if ( firstAge < 0 )
               firstAge = m_ages[ index ];
            else
               secondAge = m_ages[ index ];
         }
         else
         {
            if ( secondAge < 0 )
            {
               if ( firstAge >= 0 )
               {
                  const Snapshot * snapshot = m_projectHandle->findSnapshot( firstAge, m_snapshotsType );
                  if ( snapshot ) snapshots.push_back( snapshot );
                  if ( m_debug && m_rank == 0 && snapshot ) cerr << "adding single snapshot " << snapshot->getTime() << endl;
               }
            }
            else
            {
               if ( firstAge >= 0 )
               {
                  if ( firstAge > secondAge )
                  {
                     Swap( firstAge, secondAge );
                  }

                  SnapshotList * allSnapshots = m_projectHandle->getSnapshots( m_snapshotsType );
                  SnapshotList::iterator snapshotIter;
                  for ( snapshotIter = allSnapshots->begin(); snapshotIter != allSnapshots->end(); ++snapshotIter )
                  {
                     const Snapshot * snapshot = *snapshotIter;
                     if ( snapshot->getTime() >= firstAge && snapshot->getTime() <= secondAge )
                     {
                        if ( snapshot ) snapshots.push_back( snapshot );
                        if ( m_debug && snapshot && m_rank == 0 ) cerr << "adding range snapshot " << snapshot->getTime() << endl;
                     }
                  }
               }
            }
            firstAge = secondAge = -1;
         }
      }
   }
   sort( snapshots.begin(), snapshots.end(), snapshotSorter );

   if ( m_debug && m_rank == 0 )
   {
      cerr << "Snapshots ordered" << endl;
      SnapshotList::iterator snapshotIter;
      for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
      {
         cerr << ( *snapshotIter )->getTime() << endl;
      }
   }

   SnapshotList::iterator firstObsolete = unique( snapshots.begin(), snapshots.end(), snapshotIsEqual );
   snapshots.erase( firstObsolete, snapshots.end() );

   if ( m_debug && m_rank == 0 )
   {
      cerr << "Snapshots uniquefied" << endl;
      SnapshotList::iterator snapshotIter;
      for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
      {
         cerr << ( *snapshotIter )->getTime() << endl;
      }
   }

   return true;
}
//------------------------------------------------------------//

bool PropertiesCalculator::acquireProperties( PropertyList & properties )
{
   StringVector::iterator stringIter;

   for ( stringIter = m_propertyNames.begin(); stringIter != m_propertyNames.end(); ++stringIter )
   {

      const Property * property = m_projectHandle->findProperty( *stringIter );
      bool isComputable = false;

      if ( property == 0 && m_rank == 0 )
      {
         cerr << "Could not find property named '" << *stringIter << "'" << endl;
         continue;
      }

      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
          m_propertyManager->formationPropertyIsComputable ( property ))
      {
         isComputable = true;
      }
      else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
                m_propertyManager->formationMapPropertyIsComputable ( property ))
      {
         isComputable = true;
      } else if ( property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY and 
                m_propertyManager->surfacePropertyIsComputable ( property ))
      {
         isComputable = true;
      }

      if ( isComputable ) {
         properties.push_back( property );
      } else {
         if( m_rank == 0 ) {
            cerr << "Could not find calculator for property named '" << *stringIter << "'" << endl;
         }
      }

   }

   return true;
}
//------------------------------------------------------------//

bool PropertiesCalculator::acquireFormations( FormationVector & formationsItems )
{
   if ( m_formationNames.size() != 0 )
   {
      StringVector::iterator stringIter;
      for ( stringIter = m_formationNames.begin(); stringIter != m_formationNames.end(); ++stringIter )
      {
         const Formation * formation = m_projectHandle->findFormation( *stringIter );
         if ( !formation && m_rank == 0 )
         {
            cerr << "Could not find formation named '" << *stringIter << "'" << endl;
            continue;
         }

         formationsItems.push_back( formation );
      }
   }
  if ( formationsItems.empty() )
   {
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot( 0 );
      FormationList * formations = m_projectHandle->getFormations( zeroSnapshot, m_basement );
      FormationList::iterator formationIter;
      for ( formationIter = formations->begin(); formationIter != formations->end(); ++formationIter )
      {
         const Formation * formation = *formationIter;
         formationsItems.push_back( formation );

         if ( m_debug ) {
            cout << " adding formation "  << formation->getName () << endl;
         }

      }
   }


   return true;
}
//------------------------------------------------------------//
const GridMap * PropertiesCalculator::getPropertyGridMap ( const string & propertyName,
                                                           const Interface::Snapshot * snapshot,
                                                           const Formation * formation ) 
{
   const GridMap * propertyHasMap = 0;
   const Property* property = m_projectHandle->findProperty (propertyName);

   if ( property != 0 ) {  
      int selectionFlags = Interface::FORMATION | Interface::FORMATIONSURFACE;
      bool volumeProperties = true;
      PropertyValueList * propertyValues = m_projectHandle->getPropertyValues ( selectionFlags,
                                                                                property,
                                                                                snapshot, 
                                                                                0, 
                                                                                formation, 
                                                                                0,
                                                                                Interface::VOLUME ); 
      
      if ( propertyValues->size () == 0 ) {
         delete propertyValues;
         volumeProperties = false;

         propertyValues = m_projectHandle->getPropertyValues ( selectionFlags,
                                                               property,
                                                               snapshot, 
                                                               0, 
                                                               formation, 
                                                               0,
                                                               Interface::MAP );     
         if ( propertyValues->size () == 0 ) {
            delete propertyValues;
            
            return 0;
         }
      }

      
      if ( propertyValues->size () != 1 && m_debug &&  m_rank == 0 ) {
         cout << propertyValues->size () << ( volumeProperties ? " volume " : " map " ) << "properties values are available for  " << propertyName 
              << " at " << snapshot->getTime() << " for formation " << formation->getName() << endl;
      }
         
      propertyHasMap = (*propertyValues)[0]->hasGridMap();

      if( propertyHasMap == 0 ) {
         propertyHasMap = (*propertyValues)[0]->getGridMap();
      }
      
      delete propertyValues;
   }
   return propertyHasMap;
}
//------------------------------------------------------------//

bool PropertiesCalculator::toBeSaved( const string & propertyName, const Interface::Snapshot * snapshot, const Formation * formation ) 
{
   bool isSaved = false;
   const Property* property = m_projectHandle->findProperty (propertyName);

   if ( property != 0 ) {  
      int selectionFlags = Interface::FORMATION | Interface::FORMATIONSURFACE | Interface::SURFACE;
      bool volumeProperties = true;
      PropertyValueList * propertyValues = m_projectHandle->getPropertyValues ( selectionFlags,
                                                                                property,
                                                                                snapshot, 
                                                                                0, 
                                                                                formation, 
                                                                                0,
                                                                                Interface::VOLUME ); 
      
      if ( propertyValues->size () == 0 ) {
         delete propertyValues;
         volumeProperties = false;

         propertyValues = m_projectHandle->getPropertyValues ( selectionFlags,
                                                               property,
                                                               snapshot, 
                                                               0, 
                                                               formation, 
                                                               0,
                                                               Interface::MAP );     
         if ( propertyValues->size () == 0 ) {
            delete propertyValues;
            
            return true;
         }
      }

      
      if (propertyValues->size () != 1 && m_rank == 0 ) {
         cout << propertyValues->size () << ( volumeProperties ? " volume " : " map " ) << "properties values are available for  " << propertyName 
              << " at " << snapshot->getTime() << " for formation " << formation->getName() << endl;
      }
         
      isSaved = ( (*propertyValues)[0]->hasGridMap() != 0 ) || (*propertyValues)[0]->hasRecord();
      
      delete propertyValues;
   }
   return not isSaved;
}

bool PropertiesCalculator::toBeSaved( const string & propertyName, const Interface::Snapshot * snapshot, const Surface * surface ) 
{
   bool isSaved = false;
   const Property* property = m_projectHandle->findProperty (propertyName);

   if ( property != 0 ) {  
      int selectionFlags = Interface::SURFACE;
      bool volumeProperties = false;
      PropertyValueList * propertyValues = m_projectHandle->getPropertyValues ( selectionFlags,
                                                                                property,
                                                                                snapshot, 
                                                                                0, 
                                                                                0, 
                                                                                surface,
                                                                                Interface::MAP ); 

      if ( propertyValues->size () == 0 ) {
         delete propertyValues;
         return true;
      }
      
      if (propertyValues->size () != 1 && m_rank == 0 ) {
         cout << propertyValues->size () << ( volumeProperties ? " volume " : " map " ) << "properties values are available for  " << propertyName 
              << " at " << snapshot->getTime() << " for surface " << surface->getName() << endl;
      }
         
      isSaved = ( (*propertyValues)[0]->hasGridMap() != 0 ) || (*propertyValues)[0]->hasRecord();
      
      delete propertyValues;
   }
   return not isSaved;
}

//------------------------------------------------------------//
bool PropertiesCalculator::createSnapshotResultPropertyValue ( OutputPropertyValuePtr propertyValue,
                                                               const Snapshot* snapshot, const Formation * formation ) {
   

   if( not propertyValue->hasMap () ) {
      return true;
   }

   double p_depth = propertyValue->getDepth();
   
   PropertyValue *thePropertyValue = 0;
   
   if( p_depth > 1 ) {
      if( toBeSaved ( propertyValue->getName(), snapshot, formation )) {
         thePropertyValue = m_projectHandle->createVolumePropertyValue ( propertyValue->getName(), snapshot, 0, formation, p_depth );
      } else {
         // the property is already in output file
         if( false && m_debug && m_rank == 0 ) {
            cout << "Volume " <<propertyValue->getName() << " is in the output file" << endl;
         }
      }
   } else {

      const DataModel::AbstractSurface* surface = propertyValue->getSurface ();
      const DataAccess::Interface::Surface*   daSurface   = dynamic_cast<const DataAccess::Interface::Surface *>(surface);
      const DataAccess::Interface::Formation* daFormation = dynamic_cast<const DataAccess::Interface::Formation *>(formation);

      if ( toBeSaved ( propertyValue->getName(), snapshot, daSurface )) {

         if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) {
            thePropertyValue = m_projectHandle->createMapPropertyValue ( propertyValue->getName(), snapshot, 0, 0, daSurface );
         } else if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY ) {
            thePropertyValue = m_projectHandle->createMapPropertyValue ( propertyValue->getName(), snapshot, 0, daFormation, 0 );
         } else if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
            thePropertyValue = m_projectHandle->createMapPropertyValue ( propertyValue->getName(), snapshot, 0, daFormation, daSurface );
         } else if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY ) {
            thePropertyValue = m_projectHandle->createMapPropertyValue ( propertyValue->getName(), snapshot, 0, 0, daSurface );
         }

      } else {
         //  the property is already in output file
        if( false && m_debug && m_rank == 0 ) {
           cout << "Map " << propertyValue->getName() << " is in the output file" << endl;
         }
      }
   }     
 
   if( thePropertyValue != 0 ) {
      if ( m_debug && m_rank == 0 ) {
         cout << "   " << propertyValue->getName() << endl;;
      }
      
      GridMap * theMap = thePropertyValue->getGridMap();
      if( theMap != 0 ) {
         theMap->retrieveData();
         
         for ( unsigned int i = theMap->firstI (); i <= theMap->lastI (); ++i ) {
            for ( unsigned int j = theMap->firstJ (); j <= theMap->lastJ (); ++j ) {
               for ( unsigned int k = 0; k < theMap->getDepth (); ++k ) {
                  theMap->setValue (i, j, k, propertyValue->getValue( i, j, k ));
               }
            }
         }
         theMap->restoreData (true);
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------//
void PropertiesCalculator::outputSnapshotFormationData( const Snapshot * snapshot, const Formation * formation, PropertyList & properties,
                                                        SnapshotFormationOutputPropertyValueMap & allOutputPropertyValues )
{
   
   PropertyList::iterator propertyIter;

   if ( m_debug && m_rank == 0 ) {
      cout << "Calculating formation " << formation->getName() << " at " << snapshot->getTime() << ":" << endl;
   }

   for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter )
   {
      const Property * property = *propertyIter;
      OutputPropertyValuePtr propertyValue = allOutputPropertyValues[ snapshot ][ formation ][ property ];
      
      if ( propertyValue != 0 )
      {
         createSnapshotResultPropertyValue ( propertyValue, snapshot, formation );
      }
      else
      {
         //  outputStream << " No property available" << endl;;
      }
   }
   if ( m_debug && m_rank == 0 ) {
      cout << endl;
   }
}

//------------------------------------------------------------//
void PropertiesCalculator::acquireAll2Dproperties() {

   if ( m_all2Dproperties  )
   {
      PropertyList * allProperties = m_projectHandle->getProperties( true );
      
      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];
         
         bool addIt = false;
         
         if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
              m_propertyManager->formationMapPropertyIsComputable ( property )) {
            m_propertyNames.push_back( property->getName() );
         }
         if ( property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY and 
              m_propertyManager->surfacePropertyIsComputable ( property )) {
            m_propertyNames.push_back( property->getName() );
         }
         
      }
      
      delete allProperties;
   }
}
//------------------------------------------------------------//

OutputPropertyValuePtr PropertiesCalculator::allocateOutputProperty ( DerivedProperties::AbstractPropertyManager& propertyManager, 
                                                                       const DataModel::AbstractProperty* property, 
                                                                       const DataModel::AbstractSnapshot* snapshot,
                                                                       const Interface::Formation* formation ) {
   
   OutputPropertyValuePtr outputProperty;

   if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
         property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
       m_propertyManager->formationPropertyIsComputable ( property, snapshot, formation ))
   {
      outputProperty = OutputPropertyValuePtr ( new FormationOutputPropertyValue ( * m_propertyManager, property, snapshot, formation ));
   }

   if ( outputProperty == 0 ) {

      // check if the formation-map property is computable
      if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
           m_propertyManager->formationMapPropertyIsComputable ( property, snapshot, formation ))
      {
          outputProperty = OutputPropertyValuePtr ( new FormationMapOutputPropertyValue ( * m_propertyManager, property, snapshot, formation ));

      } else if ( property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) {

         const Interface::Surface* topSurface = 0;
         const Interface::Surface* bottomSurface = 0;

         if ( formation->getTopSurface () != 0  ) {
            topSurface = formation->getTopSurface ();
         } else if ( formation->getBottomSurface () != 0 ) {
            bottomSurface = formation->getBottomSurface ();
         }
         if( topSurface != 0 && m_propertyManager->surfacePropertyIsComputable ( property, snapshot, topSurface )) 
         {
            outputProperty = OutputPropertyValuePtr ( new SurfaceOutputPropertyValue ( * m_propertyManager, property, snapshot, topSurface ));
         } else if( bottomSurface != 0 && m_propertyManager->surfacePropertyIsComputable ( property, snapshot, bottomSurface )) 
         {
            outputProperty = OutputPropertyValuePtr ( new SurfaceOutputPropertyValue ( * m_propertyManager, property, snapshot, bottomSurface ));
         }
      }
      
   }

   return outputProperty;
}
//------------------------------------------------------------//

void PropertiesCalculator::acquireAll3Dproperties() {

   if ( m_all3Dproperties ) {

      PropertyList * allProperties = m_projectHandle->getProperties( true );

      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];

         if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
             m_propertyManager->formationPropertyIsComputable ( property ))
         {
            m_propertyNames.push_back( property->getName() );
            //            cout << property->getName() << endl;
         }

      }
      delete allProperties;
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::printListSnapshots ()  {

   if ( m_listSnapshots && m_rank == 0 ) { 

      cout << endl;
      SnapshotList * mySnapshots = m_projectHandle->getSnapshots( m_snapshotsType );
      SnapshotList::iterator snapshotIter;
      
      cout.precision ( 8 );
      cout << "Available snapshots are: ";
      
      for ( snapshotIter = mySnapshots->begin(); snapshotIter != mySnapshots->end(); ++snapshotIter )
      {
         if ( snapshotIter != mySnapshots->begin() ) cout << ",";
         int oldPrecision = cout.precision();
         cout << setprecision(9);
         cout << ( *snapshotIter )->getTime();
         cout <<setprecision(oldPrecision);
      }
      cout << endl;
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::printListStratigraphy () {
   
  if ( m_listStratigraphy && m_rank == 0 )
   {
      cout << endl;
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot( 0 );

      FormationList * myFormations = m_projectHandle->getFormations( zeroSnapshot, true );
      FormationList::iterator formationIter;
      cout << "Stratigraphy: ";
      cout << endl;
      cout << endl;
      bool firstFormation = true;
      bool arrivedAtBasement = false;
      for ( formationIter = myFormations->begin(); formationIter != myFormations->end(); ++formationIter )
      {
         const Formation * formation = *formationIter;
         if ( formation->kind() == BASEMENT_FORMATION && arrivedAtBasement == false )
         {
            arrivedAtBasement = true;
            cout << "Next formations and surfaces belong to the basement and only produce output when used with '-basement'" << endl;
         }
         if ( firstFormation )
         {
            cout << "\t" << formation->getTopSurface()->getName() << " (" << formation->getTopSurface()->getSnapshot()->getTime() << " Ma)" << endl;
            firstFormation = false;
         }

         cout << "\t\t" << formation->getName() << endl;
         const Surface * bottomSurface = formation->getBottomSurface();
         if ( bottomSurface )
         {
            cout << "\t" << bottomSurface->getName();
            if ( bottomSurface->getSnapshot() )
            {
               cout << " (" << bottomSurface->getSnapshot()->getTime() << " Ma)";
            }
            cout << endl;
         }
      }
      cout << endl;
   }


}

//------------------------------------------------------------//

void PropertiesCalculator::printOutputableProperties () {


   if( m_listProperties ) {
      PropertyList * allProperties = m_projectHandle->getProperties ( true );
      
      PetscPrintf( PETSC_COMM_WORLD, "Available 3D output properties are: " );
      
      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];
         
         if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
             m_propertyManager->formationPropertyIsComputable ( property )) {
            PetscPrintf( PETSC_COMM_WORLD, "%s ",  property->getName ().c_str() );          
         }
         
      }
      
      PetscPrintf( PETSC_COMM_WORLD, "\n\n" );
      PetscPrintf( PETSC_COMM_WORLD, "Available 2D output properties are: ");
      
      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];
         
         if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
              m_propertyManager->formationMapPropertyIsComputable ( property )) {
            PetscPrintf( PETSC_COMM_WORLD, "%s ", property->getName ().c_str() );
         }
         
         if ( property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY and 
              m_propertyManager->surfacePropertyIsComputable ( property )) {
            PetscPrintf( PETSC_COMM_WORLD, "%s ", property->getName ().c_str() );
         }
         
      }
      
      PetscPrintf( PETSC_COMM_WORLD, "\n\n" );
      
      delete allProperties;
   }
}

//------------------------------------------------------------//
bool PropertiesCalculator::setFastcauldronActivityName() {

   if( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) == 0 ||
       m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "NoCalculaction" ) {
      return false;
   }

   m_simulationMode = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode ();

   if( m_simulationMode == "HydrostaticDecompaction" ||
       m_simulationMode == "HydrostaticTemperature" ||
       m_simulationMode == "Overpressure" ) {
      m_activityName = m_simulationMode;
   } else if( m_simulationMode == "CoupledPressureAndTemperature" ) {
      m_activityName = "PressureAndTemperature";
   } else if( m_simulationMode == "LooselyCoupledTemperature" ) {
      m_activityName = "OverpressuredTemperature";
   } else if( m_simulationMode == "CoupledHighResDecompaction" or m_simulationMode == "HydrostaticHighResDecompaction") {
      m_activityName = "HighResDecompaction";
   } else {
      m_activityName = "Fastproperties";
   }
   return true;
}
//------------------------------------------------------------//

bool PropertiesCalculator::parseCommandLine( int argc, char ** argv ) {
   
   int arg;
   for ( arg = 1; arg < argc; arg++ )
   {
      if ( strncmp( argv[ arg ], "-properties", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-properties' is missing" );

            return false;
         }
         if ( !parseStrings( m_propertyNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-properties'" );

            return false;
         }
      }
      else if ( strncmp( argv[ arg ], "-ages", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-ages' is missing" );

            return false;
         }
         if ( !parseAges( m_ages, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-ages'" );

            return false;
         }
      }
      else if ( strncmp( argv[ arg ], "-formations", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-formations' is missing" );

            return false;
         }
         if ( !parseStrings( m_formationNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-formations'" );

            return false;
         }
      }
      else if ( strncmp( argv[ arg ], "-basement", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_basement = true;
      }
      else if ( strncmp( argv[ arg ], "-minor", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_snapshotsType = MAJOR | MINOR;
      }
      else if ( strncmp( argv[ arg ], "-all-2D-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_all2Dproperties = true;
      }
      else if ( strncmp( argv[ arg ], "-all-3D-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_all3Dproperties = true;
      }
      else if ( strncmp( argv[ arg ], "-list-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_listProperties = true;
      }
      else if ( strncmp( argv[ arg ], "-list-snapshots", Max( 8, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_listSnapshots = true;
      }
      else if ( strncmp( argv[ arg ], "-list-stratigraphy", Max( 8, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_listStratigraphy = true;
      }
      else if ( strncmp( argv[ arg ], "-project", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-project' is missing" );
            return false;
         }
         m_projectFileName = argv[ ++arg ];
      }
      else if ( strncmp( argv[ arg ], "-debug", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_debug = true;
      }
      else if ( strncmp( argv[ arg ], "-help", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );

         return false;
      }
      else if ( strncmp( argv[ arg ], "-?", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );

         return false;
      }
      else if ( strncmp( argv[ arg ], "-usage", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );

         return false;
      }
      else if ( strncmp( argv[ arg ], "-", 1 ) != 0 )
      {
         m_projectFileName = argv[ arg ];
      }
      else
      {
         cerr << endl << "Unknown or ambiguous option: " << argv[ arg ] << endl;
         showUsage( argv[ 0 ] );

         return false;
      }
   }
   if ( m_projectFileName == "" )
   {
      showUsage( argv[ 0 ], "No project file specified" );

      return false;
   }
   return true;
}

//------------------------------------------------------------//

void PropertiesCalculator::showUsage( const char* command, const char* message )
{
   if( m_rank == 0 ) {
      cout << endl;
      
      if ( message != 0 )
      {
         cout << command << ": " << message << endl;
      }
      
      cout << "Usage (case sensitive!!): " << command << endl << endl
           << "\t[-properties name1,name2...]                       properties to produce output for" << endl
           << "\t[-ages age1[-age2],...]                            select snapshot ages using single values and/or ranges" << endl << endl
           << "\t[-formations formation1,formation2...]             produce output for the given formations" << endl
           << "\t                                                   the four options above can include Crust or Mantle" << endl << endl
           << "\t[-basement]                                        produce output for the basement as well," << endl
           << "\t                                                   only needed if none of the three options above have been specified" << endl << endl
           << "\t[-project] projectname                             name of 3D Cauldron project file to produce output for" << endl
           << "\t[-save filename]                                   name of file to save output (*.csv format) table to, otherwise save to stdout" << endl
           << endl
           << "\t[-all-3D-properties]                               produce output for all 3D properties" << endl
           << "\t[-all-2D-properties]                               produce output for all 2D properties" << endl
           << "\t[-list-properties]                                 print a list of available properties and exit" << endl
           << "\t[-list-snapshots]                                  print a list of available snapshots and exit" << endl
           << "\t[-list-stratigraphy]                               print a list of available surfaces and formations and exit" << endl << endl
           << "\t[-help]                                            print this message and exit" << endl << endl;
      cout << "If names in an argument list contain spaces, put the list between double or single quotes, e.g:"
           << "\t-formations \"Dissolved Salt,Al Khalata\"" << endl;
      cout << "Bracketed options are optional and options may be abbreviated" << endl << endl;
   }
}

//------------------------------------------------------------//

/// destructive!!!
bool splitString( char * string, char separator, char * & firstPart, char * & secondPart )
{
   if ( !string || strlen( string ) == 0 ) return false;
   secondPart = strchr( string, separator );
   if ( secondPart != 0 )
   {
      *secondPart = '\0';
      ++secondPart;
      if ( *secondPart == '\0' ) secondPart = 0;
   }

   firstPart = string;
   if ( strlen( firstPart ) == 0 ) return false;
   return true;
}

bool parseStrings( StringVector & strings, char * stringsString )
{
   char * strPtr = stringsString;
   char * section;
   while ( splitString( strPtr, ',', section, strPtr ) )
   {
      strings.push_back( string( section ) );
   }
   return true;
}


bool parseAges( DoubleVector & ages, char * agesString )
{
   char * strPtr = agesString;
   char * commasection;
   char * section;
   while ( splitString( strPtr, ',', commasection, strPtr ) )
   {
      while ( splitString( commasection, '-', section, commasection ) )
      {
         double number = atof( section );
         ages.push_back( number );
      }
      ages.push_back( -1 ); // separator
   }
   ages.push_back( -1 ); // separator
   return true;
}

bool snapshotIsEqual( const Snapshot * snapshot1, const Snapshot * snapshot2 )
{
   return snapshot1->getTime() == snapshot2->getTime();
}

bool snapshotSorter( const Snapshot * snapshot1, const Snapshot * snapshot2 )
{
   return snapshot1->getTime() > snapshot2->getTime();
}

void displayTime ( const double timeToDisplay, const char * msgToDisplay ) {

   int hours   = (int)(  timeToDisplay / 3600.0 );
   int minutes = (int)(( timeToDisplay - (hours * 3600.0) ) / 60.0 );
   int seconds = (int)(  timeToDisplay - hours * 3600.0 - minutes * 60.0 );
   
   PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );
   PetscPrintf ( PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds );
   PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );

}
