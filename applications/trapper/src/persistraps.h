/// Class PersisTraps follows trap movements over time by
/// comparing 2d maps of traps from different times and extablishing
/// which traps bleed into which as time progresses

#ifndef __persistraps__
#define __persistraps__

#include "trapgrid.h"
#include "traplist.h"
using namespace TrapListNameSpace;

#include "persistrapfilestdio.h"
using namespace PersisTrapFileIo_NS;

#include "global_typedefs.h"
using namespace TrapIdData;

#include "directoryinfo.h"

#include <string>
#include <set>
#include <map>
using namespace std;

#include "readonlydatabase.h"

class PersisTraps
{
public:
   // CTOR / DTOR
   PersisTraps (void); 
   ~PersisTraps (); 

   // METHODS
   void createPersisTrapOutputFile (const char* filename);
   void generatePersisTrapOutput (const string& projectFilename, 
                                  const string &directoryName);
   void savePersisTrapOutputFile (void);
   void clearPersisTraps ();
  
   inline const TrapInfoHistory& getPersistentTraps (void) const; 
   inline bool trapGenerationOkay (void) const;
   
   const string& errorDescription () { return m_errDesc; }
   void resetErrorDescription () { m_errDesc = ""; }

   int getPersisId (const string& resName, const double age, const int transId) const;
   int getTransId (const string& resName, const double age, const int persisId) const;

   void convertUstoSpace (string &fileResName);
   
private:
   // TYPEDEFS
   typedef double Snapshot;
   typedef string ReservoirName;
   typedef int GridValue;

   // grid type
   typedef TrapGrid GridType;

   typedef map<Snapshot, GridType*> GridList; 
   typedef GridList::iterator GridIt;   
   typedef GridList::const_iterator const_GridIt;

   typedef map<string, GridList> ReservoirGridList;
   typedef ReservoirGridList::iterator ResGridIt;
   typedef ReservoirGridList::const_iterator const_ResGridIt;

   // METHODS
   // forward refs
   class GeneratePersistentTrapId;
   class GeneratePersistentTrapId_FirstTime;
   
   void extractGridTraps (const GridList &grid);
   void readGrid (GridType &grid, GeneratePersistentTrapId &idGen);
   void createGridList (void);
   void createTrapInfo (TrapInfo &newTrap, const GridType &grid, TrapId transId,  
                        int i, int j);
   void findTrapCoords (const GridType &grid, TrapInfo *trap, int i, int j);
   void calculateTrapMass (GridType &grid, TrapInfo *trap);
   void setupCurrentData (double snapshot);
   void savePreviousData (GridType *grid);
   bool newTrapBetter (const TrapInfo &oldTrap, const TrapInfo &newTrap);
   
   void findPersistentTrapId (TrapInfo *trap);
   void generateUniqueTrapId (TrapInfo *trap);
   
   bool attemptPerpetuatePersisID (TrapId persisId, TrapInfo *newTransTrap);
   void loadTrapHistory (void);
   void loadGridsFromFile (void);
   void reset (void);
   void setTrapListReserve (TrapList &trapList);
   void deleteAllGrids (void);
   void deleteGridList (GridList &grids);
   void readFromGridRange  (const_GridIt startGrid, const_GridIt endGrid, 
                            GeneratePersistentTrapId &idGenerator);
   void deleteTrapInfo (void);
   bool nullTrap (const TrapId &trapId) const;
   bool openTables (const string& projectFilename);
   void calculateTrapLocation (TrapInfo &trapInfo, const GridType &grid);
   bool createTrapList (const string& reservoirName, const double age);
   bool createGridList (DirectoryInfo::DirVector &fileList);
         
   static Snapshot extractSnapshot (const string &name);
   string extractReservoirName  (const GridType &grid);
   static bool us_onlyDiff (const string& str1, const string& str2);
   
   // friend references
   friend class GeneratePersistentTrapId;
   friend class GeneratePersistentTrapId_FirstTime;

   // class to generate persistent id
   class GeneratePersistentTrapId
   {
   public:
      GeneratePersistentTrapId (PersisTraps *p) : m_p (p) {}
      virtual void generateId (TrapInfo *trap) { m_p->findPersistentTrapId (trap); }
      virtual ~GeneratePersistentTrapId () {}
   protected:
      PersisTraps *m_p;
   };

   class GeneratePersistentTrapId_FirstTime : public GeneratePersistentTrapId
   {
   public:
      GeneratePersistentTrapId_FirstTime (PersisTraps *p) : GeneratePersistentTrapId (p) {}
      void generateId (TrapInfo *trap) { m_p->generateUniqueTrapId (trap); }
   };

   // INLINE METHODS
   inline GridValue gridVal (const GridType &grd, int i, int j) const;
   inline int minI  (const GridType &grd) const;
   inline int maxI (const GridType &grd) const;
   inline int minJ (const GridType &grd) const;
   inline int maxJ (const GridType &grd) const;
   inline GridType* createGrid  (void);
   inline void deleteGrid (GridType *grd);
   inline float convertToXCoord (const GridType &grd, int x) const;
   inline float convertToYCoord (const GridType &grd, int y) const;

   // MEMBERS
   ReservoirGridList m_allGrids;
   GridType* m_prevGrid;
   TrapList* m_prevTrapList;
   TrapList* m_currTrapList;
   string m_currReservoir;
   string m_directoryName;
   TrapId m_trapIdCount;
   Snapshot m_currSnapshot;
   ReservoirTrapHistory  m_trapHistory;
   TrapInfoHistory m_trapInfoHistory;
   PersisTrapFileIo* m_persisTrapOutput;
   Table *m_timeIoTbl;
   Table *m_trapIoTbl;
   Table *m_resIoTbl;
   string m_errDesc;
};

// PersisTraps inline methods
const TrapInfoHistory& PersisTraps::getPersistentTraps (void) const
{
   return m_trapInfoHistory; 
}

bool PersisTraps::trapGenerationOkay (void) const
{
   return ! m_trapInfoHistory.empty ();
}

PersisTraps::GridValue PersisTraps::gridVal (const PersisTraps::GridType &grd, int i, int j) const
{ 
   return grd.getVal (i,j);
}

int PersisTraps::minI (const PersisTraps::GridType &grd) const
{
   return grd.minX();
}

int PersisTraps::maxI (const PersisTraps::GridType &grd) const
{
   return grd.maxX ();
}

int PersisTraps::minJ (const PersisTraps::GridType &grd) const
{
   return grd.minY();
}

int PersisTraps::maxJ (const PersisTraps::GridType &grd) const
{
   return grd.maxY ();
}

PersisTraps::GridType* PersisTraps::createGrid (void)
{
   return new TrapGrid ();
}

void PersisTraps::deleteGrid (GridType *grd)
{
   delete grd;
   grd = 0;
}

float PersisTraps::convertToXCoord (const GridType &grd, int x) const
{
    return grd.eastMin() + (grd.deltaX() * x);
}

float PersisTraps::convertToYCoord (const GridType &grd, int y) const
{
    return grd.northMin() + (grd.deltaY() * y);
}

#endif


