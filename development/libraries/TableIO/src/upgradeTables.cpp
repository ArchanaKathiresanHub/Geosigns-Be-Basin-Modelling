//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
// This utility allow to load and then compare table by table 2 .project3d files

#include "cauldronschema.h"
#include "database.h"
#include "datatype.h"
#include "LogHandler.h"
#include "FormattingException.h"

typedef formattingexception::GeneralException ProjectUpgradeException;
using namespace std;

namespace database
{
   void upgradeReservoirOptionsIoTblTableVer100to101( Database * db, Table * tbl )
   {
      // check that this table was loaded from file and was not created by default
      bool loaded = db->getTablesInFile().count( "ReservoirOptionsIoTbl" ) > 0;

      if ( loaded && tbl->version() > 100 ) return; // version is correct, nothing to do

      if ( !loaded )
      {
         if ( db->getTablesInFile().count( "DetectedReservoirIoTbl" ) > 0 )
         {  TableDefinition * tblDef = new TableDefinition( "DetectedReservoirIoTbl", "DetectedReservoirIoTbl", 100 );

            // the old verion of the table is in the file, we need to recreate table definition and reload project
            if ( db->addTableDefinition( tblDef ) )
            {
               tblDef->addFieldDefinition( "TrapCapacity",          datatype::Double,  "m3",  "5e5", 0 );
               tblDef->addFieldDefinition( "BioDegradInd",          datatype::Int,      "",     "0", 1 );
               tblDef->addFieldDefinition( "OilToGasCrackingInd",   datatype::Int,      "",     "1", 2 );
               tblDef->addFieldDefinition( "DiffusionInd",          datatype::Int,      "",     "0", 3 );
               tblDef->addFieldDefinition( "MinOilColumnHeight",    datatype::Double,  "m",   "1.0", 4 );
               tblDef->addFieldDefinition( "MinGasColumnHeight",    datatype::Double,  "m",   "1.0", 5 );
               tblDef->addFieldDefinition( "BlockingInd",           datatype::Int,      "",     "0", 6 );
               tblDef->addFieldDefinition( "BlockingPermeability",  datatype::Double, "mD",  "1e-9", 7 );
               tblDef->addFieldDefinition( "BlockingPorosity",      datatype::Double,  "%",     "0", 8 );

               LogHandler( LogHandler::INFO_SEVERITY ) << R"(Upgrading table "DetectedReservoirIoTbl"->"ReservoirOptionsIoTbl")";
               LogHandler( LogHandler::INFO_SEVERITY ) << "For this dataschema was updated and project file will be reloaded...";
               db->reload( "DetectedReservoirIoTbl" );
            }
            else
            {
               delete tblDef;
               throw ProjectUpgradeException() << R"(Table "DetectedReservoirIoTbl" upgrade error)";
            }
         }
         else { return; } // no both tables in project files
      }

      Table * oldTbl = db->getTable( "DetectedReservoirIoTbl" );
      tbl = db->getTable( "ReservoirOptionsIoTbl" );

      if ( nullptr == oldTbl || oldTbl->size() == 0 )
      {  // empty table nothing to upgrade
         db->deleteTable( "DetectedReservoirIoTbl" );
         return;
      }

      // check condition for upgrading
      if ( nullptr == tbl || tbl->size() != 0 || oldTbl->size() > 1 )
      {
         LogHandler( LogHandler::FATAL_SEVERITY ) << R"(Can't upgrade table "DetectedReservoirIoTbl")";
         throw ProjectUpgradeException() << R"(Table "DetectedReservoirIoTbl" upgrade error)";
      }      

      // copying the same columns from old to new one;
      Record * newRec = tbl->createRecord();
      Record * oldRec = oldTbl->getRecord( 0 );
      
      const TableDefinition & tblDef    = tbl->getTableDefinition();
      const TableDefinition & oldTblDef = oldTbl->getTableDefinition();

      for ( size_t i = 0; i < tblDef.size(); ++i )
      {
         std::string colName = tblDef.getFieldDefinition( static_cast<int>( i ) )->name();
         int oldInd = oldTblDef.getIndex( colName ); 
         
         if ( oldInd < 0 ) { continue; } // column does not exist in old table
         
         switch ( tblDef.getFieldDefinition( static_cast<int>( i ) )->dataType() )
         {
            case datatype::Bool:   { newRec->setValue<bool>(   i, oldRec->getValue<bool>(   oldInd ) ); break; }
            case datatype::Int:    { newRec->setValue<int>(    i, oldRec->getValue<int>(    oldInd ) ); break; }
            case datatype::Long:   { newRec->setValue<long>(   i, oldRec->getValue<long>(   oldInd ) ); break; }
            case datatype::Float:  { newRec->setValue<float>(  i, oldRec->getValue<float>(  oldInd ) ); break; }
            case datatype::Double: { newRec->setValue<double>( i, oldRec->getValue<double>( oldInd ) ); break; }
            case datatype::String: { newRec->setValue<string>( i, oldRec->getValue<string>( oldInd ) ); break; }
            default: ProjectUpgradeException() << "Unknown data type for database record in ReservoirOptionsIoTbl table";
         }
      }
      // now we can delete old table and table definition from db
      db->deleteTable( "DetectedReservoirIoTbl" );
   }

   void upgradeFilterTimeIoTblTableVer100to101( Database * db, Table * tbl )
   {
     // check that this table was loaded from file and was not created by default
     bool loaded = db->getTablesInFile().count( "FilterTimeIoTbl" ) > 0 ;

     if ( loaded && tbl->version() > 100 ) return; // version is correct, nothing to do

     if (tbl->size() == 0) return;

     // Save latest version of table definition
     TableDefinition* newVersionTableDefinition = db->getTable("FilterTimeIoTbl")->getTableDefinition().deepCopy();

     // Delete updated table and table definition from db
     db->deleteTable( "FilterTimeIoTbl" );

     TableDefinition * tableDefinition = new TableDefinition( "FilterTimeIoTbl", "FilterTimeIoTbl", 100 );
     // the old verion of the table is in the file, we need to recreate table definition and reload project
     if ( db->addTableDefinition( tableDefinition ) )
     {
        tableDefinition->addFieldDefinition(  "PropertyName",          datatype::String,              "",              "",               0 );
        tableDefinition->addFieldDefinition(  "ModellingMode",         datatype::String,              "",              "",               1 );
        tableDefinition->addFieldDefinition(  "OutputOption",          datatype::String,              "",              "",               2 );
        tableDefinition->addFieldDefinition(  "ResultOption",          datatype::String,              "",              "",               3 );
     }
     db->reload("FilterTimeIoTbl");
     tbl = db->getTable("FilterTimeIoTbl");

     std::vector<int> deleteIndices;
     for (int i = tbl->size() - 1; i>=0; --i)
     {
        Record* rec = tbl->getRecord(i);
        if ("1d" == rec->getValue<std::string>("ModellingMode"))
        {
          deleteIndices.push_back(i);
        }
     }
     db->deleteTable( "FilterTimeIoTbl" );

     db->addTableDefinition(newVersionTableDefinition);
     db->reload("FilterTimeIoTbl");
     tbl = db->getTable("FilterTimeIoTbl");
     for (int i : deleteIndices)
     {
       tbl->deleteRecord(tbl->getRecord(i));
     }
   }  

   void upgradeProjectIoTblTableVer100to101( Database * /*db*/, Table * /*tbl*/ )
   {
     // ModellingMode is removed, no further action required
   }
}

