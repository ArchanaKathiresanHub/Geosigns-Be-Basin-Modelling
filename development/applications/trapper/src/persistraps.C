/// Class PersisTraps follows trap movements over time by
/// comparing 2d maps of traps from different times and extablishing
/// which traps bleed into which as time progresses

#include "persistraps.h"


#include "h5_file_types.h"

#include "globalnumbers.h"
using namespace Null_Values;

//
// CTOR / DTOR
//

PersisTraps::PersisTraps (void)
   :  m_persisTrapOutput (new PersisTrapFileStdIo ()), 
   m_trapIdCount (-1), 
   m_currTrapList (NULL), 
   m_prevTrapList (NULL)
{}

PersisTraps::~PersisTraps (void)
{
   // delete dynamic memory
   reset ();
   
   delete m_persisTrapOutput;
}

//
//  PUBLIC METHODS
//
void PersisTraps::createPersisTrapOutputFile (const char* filename)
{
   m_persisTrapOutput->createFile (filename);
}

/// PersisTraps::generatePersisTrapOutput is the main function that controls the PersisTraps functionality
/// It opens the project file, loads the TrapID grid maps and then calls the function to compare the grids over time
void PersisTraps::generatePersisTrapOutput (const string& projectFilename, const string& dirName)
{ 
   resetErrorDescription ();
   
   m_directoryName = dirName;
   
   if( ! openTables (projectFilename) ) 
   {
      m_errDesc += "Please check Cauldron Migration has been run.\n";
      return;
   }
   
   // read all resrock trap id grid files
   loadGridsFromFile ();
   
   // loop grids per reservoir
   const_ResGridIt gridIt, endGrids (m_allGrids.end());
   for ( gridIt = m_allGrids.begin (); gridIt != endGrids; ++gridIt )
   {  
      m_currReservoir = gridIt->first.c_str();
      extractGridTraps (gridIt->second);
   }
}

void PersisTraps::savePersisTrapOutputFile (void)
{
   loadTrapHistory ();
   m_persisTrapOutput->writeTrapHistoryToFile (m_trapHistory);
}

void PersisTraps::clearPersisTraps ()
{
   reset ();
}

/// PersisTraps::getPersisId returns a persistent trap Id based on its reservoir name, age and transient Id
int PersisTraps::getPersisId (const string& resName, const double age, const int transId) const
{
   int persisId = -1;
   
   // get history for specific reservoir
   const_TrapInfoHistoryIT trapInfoIt = m_trapInfoHistory.find (resName);
   if ( trapInfoIt != m_trapInfoHistory.end() )
   {
      //  history for specific age
      const TrapInfoMap &trapMap = trapInfoIt->second;
      const_TrapInfoMapIT trapListIt = trapMap.find (age);
      if ( trapListIt != trapMap.end() )
      {
         // get persistent id
         const TrapList* trap = (trapListIt->second);
         const TrapInfo *info = trap->getTrapWithTransId (transId);
         if ( info )
         {
            persisId = info->persisId;         
         }
      }
   }
   
   return persisId;
}

/// PersisTraps::getTransId returns a transient trap Id for a specific age, based on its reservoir name and persistentId
int PersisTraps::getTransId (const string& resName, const double age, const int persisId) const
{
   int transId = -1;
   
   // get history for specific reservoir
   const_TrapInfoHistoryIT trapInfoIt = m_trapInfoHistory.find (resName);
   if ( trapInfoIt != m_trapInfoHistory.end() )
   {
      //  history for specific age
      const TrapInfoMap &trapMap = trapInfoIt->second;
      const_TrapInfoMapIT trapListIt = trapMap.find (age);
      if ( trapListIt != trapMap.end() )
      {
         // get transient id
         const TrapList* trap = (trapListIt->second);
         const TrapInfo *info = trap->getTrapWithPersisId (persisId);
         if ( info )
         {
            transId = info->transId;         
         }
      }
   }
   
   return transId;
}

//
// PRIVATE METHODS
//

/// PersisTraps::loadGridsFromFile reads all TrapID grid files into memory
/// and saves them in an map sorted by reservoir name and age
void PersisTraps::loadGridsFromFile (void)
{
   // read grid2d's from hdf5 files
   
   // create grid file list
   DirectoryInfo::DirVector gridList;
   if ( ! createGridList (gridList) )
   {
      m_errDesc += "Result grids not found. Cauldron results must be in HDF5 format for Trapper.\n";      
      return;
   }
   
   // loop grid file list and extract grids into grid list
   H5_ReadOnly_File infile;
   GridType *tempGrid; 
   DirectoryInfo::DirVectorIt gridIt, endGrids (gridList.end());
   
   string reservoirName;
   double age;

   string fileName = m_directoryName + "/HighResMigration_Results.HDF";
   if ( ! infile.open (fileName.c_str()) )
   {
     m_errDesc += "Could not open HighResMigration_Results.HDF.\n";
     return;
   }

   for ( gridIt = gridList.begin(); gridIt != endGrids; ++gridIt )
   {

     // extract grid data from file
     tempGrid = createGrid ();
      if ( tempGrid ) 
      {
         tempGrid->readGrid (&infile, *gridIt);
         
         // get the reservoir name and age from the grid and filename
         reservoirName = extractReservoirName (*tempGrid);
         age = extractSnapshot (*gridIt);              

         // create corresponding trap list
         if ( createTrapList (reservoirName, age) )
         {
            // add the grid to the grid list
            m_allGrids[reservoirName][age] = tempGrid;
         }
         else
         {
            cout << endl << "Error creating Trap List" << endl;
         }           
      }
      else
      {
         cout << endl << "PersisTraps::loadGridsFromFile" 
               << endl << "Error creating new Grid" << endl;
      }
      
   }
   infile.close ();
}

/// PersisTraps::extractGridTraps calls PersisTraps::readFromGridRange for
/// the first grid and then for all the rest of the grids. This is in two separate calls
// as a functor object is passed into the call and is different for the first grid.
void PersisTraps::extractGridTraps (const GridList &grids)
{
   if ( grids.size() < 1 )
   {
      return;
   }
   
   // create persitent id generator objects,
   // one for the first grid and one for all grids after that
   GeneratePersistentTrapId idGenerator (this);
   GeneratePersistentTrapId_FirstTime idGenerator_firsttime (this);
   
   // set up iterators for looping the grids
   const_GridIt firstGrid = grids.begin();
   const_GridIt gridIt = firstGrid; 
   ++gridIt;
   const_GridIt endGrids = grids.end();
   
   // for the first grid, use idGenerator_firsttime to generate
   // a unique id
   readFromGridRange (firstGrid, gridIt, idGenerator_firsttime);
   
   // for all grids after the first, use idGenerator which will compare
   // each grid to the previous grid
   readFromGridRange (gridIt, endGrids, idGenerator); 
}

/// PersisTraps::readFromGridRange loops through a specified ranges of grids and reads 
/// the necessary data from it to establish trap history
void PersisTraps::readFromGridRange (const_GridIt startGrid, const_GridIt endGrid, 
                                     GeneratePersistentTrapId &idGenerator)
{
   while ( startGrid != endGrid )
   {
      setupCurrentData (startGrid->first);
 
      readGrid (*(startGrid->second), idGenerator);
      
      savePreviousData (startGrid->second);
      
      ++startGrid;
   } 
}

/// PersisTraps::readGrid loops throught the grid points on a grid and if it 
/// finds a trap at any point, it saves its info, giving it a persistent and transiend id
void PersisTraps::readGrid ( GridType &grid, GeneratePersistentTrapId &idGen )
{
   const int MIN_I = minI (grid);
   const int MAX_I = maxI (grid);
   const int MIN_J = minJ (grid);
   const int MAX_J = maxJ (grid);
   
   TrapId trapId;
   TrapInfo trapInfo;
   
   // create set to hold examined trap ids
   set<TrapId> trapIdList;
   
   // examine each i,j point on the grid
   int i, j;
   for ( i = MIN_I; i < MAX_I; ++i )
   {
      for ( j = MIN_J; j < MAX_J; ++j )  
      {
         // get trap id
         trapId = gridVal (grid, i, j);
         
         // skip if not a trap
         if ( nullTrap (trapId) ) continue;
         
         // try adding trap Id to set
         // if success, then id not already examined)
         if ( (trapIdList.insert (trapId)).second )      
         {
            // create new trap with trap id
            createTrapInfo (trapInfo, grid, trapId, i, j);
            
            // generate the traps persistent id
            idGen.generateId (&trapInfo); 
            
            // save the trap to the trap list 
            m_currTrapList->add (trapInfo);
         }
      }
   }
}

// function for test purposes only
void PersisTraps::loadTrapHistory (void)
{
   // loop trap info history and move into reservoir trap history 
   // for writing to file   
   
   const_TrapInfoHistoryIT resIt, endRes = m_trapInfoHistory.end();
   const_TrapInfoMapIT snapshotIt, endSnapshots;
   
   for ( resIt = m_trapInfoHistory.begin(); resIt != endRes; ++resIt )
   {
      endSnapshots = (resIt->second).end();
      for ( snapshotIt = (resIt->second).begin(); snapshotIt != endSnapshots; ++snapshotIt )
      {
         const TrapInfoList *traps = (snapshotIt->second)->getTrapList ();
         if ( traps->empty () ) continue;
         
         TrapOutputMap &currentMap = m_trapHistory[resIt->first.c_str()][snapshotIt->first];
         const_TrapInfoListIT trapIt, endTraps = traps->end();
         
         for ( trapIt = traps->begin(); trapIt != endTraps; ++trapIt )
         {
            currentMap[(*trapIt).persisId] = (*trapIt).transId;
         }
      }
   }
}

/// PersisTraps::createTrapInfo gathers trap info from a particular grid coordinate
/// and saves the info in a TrapInfo object
void PersisTraps::createTrapInfo (TrapInfo &newTrap, const GridType &grid, 
                                  TrapId transId, int i, int j)
{
   newTrap.reinit ();
   newTrap.transId = transId;   
   findTrapCoords (grid, &newTrap, i, j);
   calculateTrapLocation (newTrap, grid); 
}

bool PersisTraps::newTrapBetter (const TrapInfo &oldTrap, const TrapInfo &newTrap)
{
   // new trap better if greater size than old trap 
   // because size is determined by volume
   return newTrap.coordList.size () > oldTrap.coordList.size ();
}

/// PersisTraps::findTrapCoords is called when  trap coordinate is found. This function then searches the sourrounding
/// coordinates to find the edges of the trap
void PersisTraps::findTrapCoords (const GridType &grid, TrapInfo *trap, int i, int j)
{
   const int max_I = maxI (grid);
   const int max_J = maxJ (grid);   
   TrapId trapId;
   
   int x = i;
   int y = j;
   
   while ( x != max_I && y != max_J )
   {
      // save grid coordinates of trap along column
      do
      {
         (trap->coordList).push_back (GridCoords (x,y));
         if ( ++y == max_J ) break;
         trapId = gridVal (grid, x, y);
      } while ( trapId == trap->transId );     
      
      // go to next column if there is one
      if ( ++x == max_I ) break;
      
      // iterate through column until find same trap again 
      // or reach end of column
      y = 0;
      while ( (trapId = gridVal (grid, x, y)) != trap->transId )
      {
         if ( ++y == max_J ) break;
      }   
   }
}

/// PersisTraps::createTrapList creates a list of traps for a particular reservoir at a particular age
bool PersisTraps::createTrapList (const string& reservoirName, const double age)
{
   pair <double, TrapList*> newPair (age, new TrapList (m_trapIoTbl->size()));
   return (m_trapInfoHistory [reservoirName].insert (newPair)).second;
}

/// PersisTraps::setupCurrentData updates current pointers
void PersisTraps::setupCurrentData (double snapshot)
{
   m_currSnapshot = snapshot;
   m_currTrapList = m_trapInfoHistory [m_currReservoir][m_currSnapshot];
}

/// PersisTraps::savePreviousData updates pervious pointers
void PersisTraps::savePreviousData (GridType *grid)
{
   m_prevGrid = grid;
   m_prevTrapList = m_currTrapList;
}

/// PersisTraps::findPersistentTrapId first checks if a newly found trap on a grid overlaps with 
/// a trap on a grid from a previous timestep. If so, then further checks are made to see if the trap from 
/// the previous timestep can be said to have evoloved into this new trap. It is possible that the new trap
/// already has been assigned a previous trap, from which it evolved. It is also possible that the pervious trap
/// has already been asigned to another more recent trap. In either of these cases, a best match rule must be applied
void PersisTraps::findPersistentTrapId (TrapInfo *newTrap)
{
   TrapId trapId;
   set <TrapId> trapIdsLookedAt;
   TrapInfo *newPersisTrap, *oldPersisTrap;
   
   // loop the coordinates of the newly found trap
   const_CoordIt coordIt, endCoords ((newTrap->coordList).end());
   for ( coordIt = (newTrap->coordList).begin(); coordIt != endCoords; ++coordIt )
   {
      // examine the trap's coordinates on the previous snapshot's grid
      trapId = gridVal (*m_prevGrid, (*coordIt).i, (*coordIt).j);
      
      // if not trap id then ignore
      if ( nullTrap (trapId) ) continue;
      
      // if trap id on previous grid already compared to new trap then ignore
      if ( ! (trapIdsLookedAt.insert (trapId)).second ) continue;
      
      // get the persistent trap in the previous grid, that 
      // overlaps the new trap
      newPersisTrap = m_prevTrapList->getTrapWithTransId (trapId);
      
      // before mapping the persistent trap from the previous grid
      // to the newly found trap in the current grid
      // check if another persistent trap has already been 
      // mapped to this new trap
      if ( newTrap->persisId != Cauldron_Null_Trap  )
      {
         oldPersisTrap = m_prevTrapList->getTrapWithPersisId (newTrap->persisId);
         
         if ( oldPersisTrap == NULL || 
              newTrapBetter (*oldPersisTrap, *newPersisTrap) )
         {
            // the new persistent id is better matched to this trap than the old persistent id
            // it was mapped to, so use the new persistent id
            attemptPerpetuatePersisID (newPersisTrap->persisId, newTrap);
         }
      }
      else
      {
         // this trap does not already have a persistent id so free to map the
         // overlapping trap's persistent id
         attemptPerpetuatePersisID (newPersisTrap->persisId, newTrap);
      }
   }
   
   // if have not assigned a persistent id by now,
   // generate a new one
   if ( newTrap->persisId == Cauldron_Null_Trap  )
   {
      generateUniqueTrapId (newTrap);
   }
}

/// PersisTraps::generateUniqueTrapId is called for the first grid to be examined
/// since all of its traps starting from nothing, they need a persisent id to be generated for them
void PersisTraps::generateUniqueTrapId (TrapInfo *trap)
{
   trap->persisId = ++m_trapIdCount;
}

/// PersisTraps::attemptPerpetuatePersisID checks to see if it is appropriate to map a persistent id to a
/// newly found trap
bool PersisTraps::attemptPerpetuatePersisID (TrapId persisId, TrapInfo *newTransTrap)
{
   // first check if the persisent id has already been 
   // used elsewhere on this grid
   
   TrapInfo *oldTransTrap = m_currTrapList->getTrapWithPersisId (persisId);
   
   if ( oldTransTrap != NULL )
   {
      // have found another trap on this grid that is
      // already mapped to the persistent trap so
      // must compare the two transient traps and see which
      // the persisent trap should map to
      if ( newTrapBetter (*oldTransTrap, *newTransTrap) )
      {
         // the new trap is better so give it the old trap's 
         // persistent id and generate a new persisent id for the old trap
         newTransTrap->persisId = oldTransTrap->persisId;
         generateUniqueTrapId (oldTransTrap);
      }
   }
   else
   {
      // persistent trap id not mapped to anywhere else on grid so
      // free to map to this trap
      newTransTrap->persisId = persisId;
   }
   
   return newTransTrap->persisId != Cauldron_Null_Trap ;
}

/// PersisTraps::extractSnapshot extracts a snapshot name from a filename
PersisTraps::Snapshot PersisTraps::extractSnapshot (const string &name)
{
  // assuming name in form of:
  // ResRockTrapId_<snapshot>_<ReservoirName>_
  // and trying to isolate the snapshot
  int startNum = name.find_first_of ('_') + 1;
  int endNum   = name.find_first_of ('_', startNum);
  string snapshotStr = name.substr (startNum, endNum - startNum);
   
  return atof (snapshotStr.c_str());
}

/// PersisTraps::extractReservoirName extracts a reservoir name from a filename
string PersisTraps::extractReservoirName (const GridType &grid)
{
  // assuming name in format ResRockTrapId_<age>_<ReservoirName>_
  // as written from Cauldron to ResRockTrapId files
  string gridname = grid.gridName ();

  // get the index of the 3rd field separated by '_'
  int startIndex = gridname.find_first_of ('_') + 1;
  startIndex = gridname.find_first_of ('_', startIndex) + 1;
  string reservoirName = gridname.substr (startIndex, gridname.size() - startIndex - 1);
   
  // if this reservoir name has underscores they may be spaces in the project file, 
  // so need to convert to spaces if this is the case
  convertUstoSpace (reservoirName);

  return reservoirName;
}

void PersisTraps::reset (void)
{
   // empty grid list
   deleteAllGrids ();
   
   // delete trap info list
   deleteTrapInfo ();
   
   // empty trap output vector
   m_trapHistory.erase (m_trapHistory.begin(), m_trapHistory.end());
   
   // reset unique trap id
   m_trapIdCount = -1;
}

void PersisTraps::deleteAllGrids (void)
{
   if ( m_allGrids.size () < 1 ) return;
   
   ResGridIt allGridIt, endAllGrids = m_allGrids.end();
   for ( allGridIt = m_allGrids.begin(); allGridIt != endAllGrids; ++allGridIt )
   {
      deleteGridList (allGridIt->second);
   }
   
   m_allGrids.erase (m_allGrids.begin(), m_allGrids.end());
}

void PersisTraps::deleteGridList (GridList &grids)
{
   if ( grids.size() < 1 ) return;
   
   GridIt gridIt, endGrids = grids.end();
   for ( gridIt = grids.begin(); gridIt != endGrids; ++gridIt )
   {
      delete (gridIt->second);
   }
   
   grids.erase (grids.begin(), grids.end());
}

void PersisTraps::deleteTrapInfo (void)
{
   TrapInfoHistoryIT resIt, endRes = m_trapInfoHistory.end();
   TrapInfoMapIT snapshotIt, endSnapshots;
   
   for ( resIt = m_trapInfoHistory.begin(); resIt != endRes; ++resIt )
   {
      endSnapshots = (resIt->second).end();
      for ( snapshotIt = (resIt->second).begin(); snapshotIt != endSnapshots; ++snapshotIt )
      {
         // delete trap list pointer
         delete snapshotIt->second;
      }
   }
   
   m_trapInfoHistory.erase (m_trapInfoHistory.begin(), m_trapInfoHistory.end());
}

bool PersisTraps::nullTrap (const TrapId &trapId) const
{
   return (trapId == Cauldron_Null_Trap ||
           trapId == Ibslink_Null_Float ||
           trapId == Cauldron_Null_Float);
}

/// PersisTraps::openTables opens all necessary tables at once, before the main 
/// functionality starts
bool PersisTraps::openTables (const string& projectFilename)
{ 
   ReadOnlyDatabase *dbase = ReadOnlyDatabase::getReadOnlyDatabase ();
   
   if ( ! dbase->openDatabase (projectFilename.c_str()) ) return false;
   
   m_trapIoTbl = dbase->getTable ("TrapIoTbl");
   if ( ! m_trapIoTbl  )
   {
      m_errDesc += "Error opening Trap Table in Project File.\n";
   }
   
   m_resIoTbl = dbase->getTable ("ReservoirIoTbl");
   if ( ! m_resIoTbl )
   {
      m_errDesc += "Error opening Reservoir Table in Project File.\n";
   }
   
   m_timeIoTbl = dbase->getTable ("TimeIoTbl");
   if ( ! m_timeIoTbl )
   {
      m_errDesc += "Error opening TimeIoTable in Project File.\n";
   }
   
   return m_resIoTbl && m_trapIoTbl && m_timeIoTbl;
}

/// PersisTraps::calculateTrapLocation opens the TrapIoTbl to read a
/// trap's top depth coordinates
void PersisTraps::calculateTrapLocation (TrapInfo &trapInfo, const GridType &grid)
{
   // get first (i,j) position of trap and convert to coordinates
   /*
   trapInfo.locTopDepth = LocationCoords 
                          (convertToXCoord (grid, trapInfo.coordList[0].i), 
                           convertToYCoord (grid, trapInfo.coordList[0].j)); 
   */
   
   // loop TrapIoTbl to find transient trap
   int tableSize = m_trapIoTbl->size();
   Record *rec;
   for ( int i=0; i < tableSize;  ++i )
   {
      rec = m_trapIoTbl->getRecord (i);
      
      if ( (getTrapID (rec) == trapInfo.transId) &&
           (getAge (rec) == m_currSnapshot) &&
           (getReservoirName (rec) == m_currReservoir) )
      {
         trapInfo.locTopDepth = LocationCoords (getXCoord (rec), getYCoord (rec));
      }
   }
}

void PersisTraps::convertUstoSpace (string &fileResName)
{
   if ( ! m_resIoTbl )  return;
      
   string recResName;
   int tableSize = m_resIoTbl->size();
   
   // loop table and look for match with fileResName
   for ( int i=0; i < tableSize; ++i )
   {
      recResName = getReservoirName (m_resIoTbl->getRecord (i));

      // if the file reservoir name = the record reservoir name except for _ and ' ' then
      // use the record reservoir name 
      if ( us_onlyDiff (fileResName, recResName) )
      {
         fileResName = recResName;
         break;
      }
   }
}

bool PersisTraps::us_onlyDiff (const string& str1, const string& str2)
{
   // returns true if the only differences between the two strings is
   // underscores and spaces
   int strSize = str1.size() < str2.size() ? str1.size() : str2.size();
   bool diff = false;
      
   int i;
   for ( i=0; i < strSize; ++i )
   {
      if ( str1[i] != str2[i] ) 
      {
         diff = true;
         if ( (str1[i] == ' ' && str2[i] == '_') || (str1[i] == '_' && str2[i] == ' ') )
         {
            continue;
         }
         else
         {
            break;
         }
      }
   }
   
   return i == strSize && diff;
}

bool PersisTraps::createGridList (DirectoryInfo::DirVector &fileList)
{
   if ( ! m_timeIoTbl )  return false;
      
   int tableSize = m_timeIoTbl->size();
   
   // loop table and look for match with fileResName
   for ( int i=0; i < tableSize; ++i )
   {
      const string & propertyName = getPropertyName (m_timeIoTbl->getRecord (i));
      if (propertyName == "ResRockTrapId")
      {
         const string & gridName = getPropertyGrid (m_timeIoTbl->getRecord (i));
	 fileList.push_back (gridName);
      }
   }

   return  ! fileList.empty ();
}


