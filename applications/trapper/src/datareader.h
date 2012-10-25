/// Class DataReader is an abstract Base Class for reading data from Cauldron Project file
/// through the TableIo lib
/// Each class that inherits from it must define its own version of readData
/// Other protected methods are virtual so the inheriting class can redefine them 
/// for its own purposes. 

#ifndef __datareader__
#define __datareader__

#include "readonlydatabase.h"

#include "global_typedefs.h"
using namespace FileRequestData;
using namespace PlotData;

class CreateUserRequest;

class DataReader
{
public:
   // ctor / dtor
   virtual ~DataReader () {} 
   
   // public methods
   void setRequestFromUser (const CreateUserRequest *userReq) { m_userRequest = userReq; }
   const ReservoirLineGroup* getXYVector () const { return &m_plotLines; }
   void clearData ();
   const CreateUserRequest* userRequest () const { return m_userRequest; }
   
   // pure virtual public methods
   virtual string readData(const char* filename) = 0;
   
protected: 
   // protected methods   
   ReservoirLineGroup& plotLines () { return m_plotLines; }
   bool openProject (const char *filename);
   Table* getTable (const char *tableName);
   
   // protected virtual methods
   virtual void readTable (Table* table);
   virtual bool recordWanted (Record *rec);
   virtual void setupSearchIterators ();
   virtual int getRecordTrapId (Record *rec);
   virtual double getRecordAge (Record *rec);
   virtual const string& getRecordReservoirName(Record *rec);
   
   // pure virtual protected methods
   virtual void readRecord (Record *rec) = 0;
   
   // protected iterators for search request
   const_ReservoirTrapDataMapIT m_resIt, m_endResIt;
   const_TrapDataMapIT m_persisIt, m_endPersisIt;
   const_TransIdMapIT m_transIt, m_endTransIt;
   
   const ReservoirTrapDataMap *m_resData;
   const TrapDataMap *m_persisTrapData;
   const TransIdMap *m_transTrapData;
   
   const string* m_currReservoir;
   const int* m_currPersisTrapId;
   
private:
   ReadOnlyDatabase *m_readOnlyDatabase;
   const CreateUserRequest *m_userRequest;
   ReservoirLineGroup m_plotLines;
};

#endif
