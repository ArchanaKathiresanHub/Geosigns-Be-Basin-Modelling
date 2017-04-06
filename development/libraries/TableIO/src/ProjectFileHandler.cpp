//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ProjectFileHandler.h"

#include <memory>

// TableIO
#include "dataschema.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

// Utilities
#include "ConstantsNames.h"
using namespace Utilities;
#include "FormattingException.h"
#include "LogHandler.h"

// FileSystem
#include "FilePath.h"
#include "FolderPath.h"

const std::string database::ProjectFileHandler::OutputTablesFileIoTableName = "OutputTablesFileIoTbl";
const std::string database::ProjectFileHandler::OutputTablesIoTableName = "OutputTablesIoTbl";
const std::string database::ProjectFileHandler::OutputTablesFileName = "Output.iotables3d";

database::ProjectFileHandler::ProjectFileHandler () 
{
   std::unique_ptr<database::DataSchema> cauldronSchema( database::createCauldronSchema() );
   database::TableDefinition * tableDef = cauldronSchema->getTableDefinition( "DepthIoTbl" );
   
   if ( tableDef != nullptr ) {
      // Adding (volatile, won't be output) definition for DepositionSequence field
      // Required to properly sort the DepthIoTbl, not to be output
      tableDef->addVolatileFieldDefinition( "DepositionSequence", datatype::Int, "", "0" );
   }

   m_inputDataBase.reset( database::Database::CreateFromSchema( *(cauldronSchema.get()) ) );

   // Now that we have the cauldron schema, use it to set the list of all possible table names.
   m_allTableNames.reserve ( cauldronSchema->size ());

   for ( size_t i = 0; i < cauldronSchema->size(); ++i ) {
      m_allTableNames.push_back ( cauldronSchema->getTableDefinition ( i )->name ());
   }
}

database::ProjectFileHandler::ProjectFileHandler ( const std::string& fileName ) 
{
   if ( ! ibs::FilePath( fileName ).exists() ) {
      throw formattingexception::GeneralException() << "Project file " << fileName << " does not exist";
   }

   loadFromFile ( fileName );
   loadOutputTables ();
}

database::ProjectFileHandler::ProjectFileHandler ( const std::string&              fileName
                                                 , const std::vector<std::string>& outputTableNames 
                                                 ) 
                                                 : ProjectFileHandler ( fileName )
{
   for ( size_t i = 0; i < outputTableNames.size (); ++i ) {

      if ( !m_outputDataBase || !m_outputDataBase->hasTable ( outputTableNames [ i ] ) ) {
         setTableAsOutput ( outputTableNames [ i ]);
      }
   }
}

database::ProjectFileHandler::~ProjectFileHandler () {
}


bool database::ProjectFileHandler::saveToFile ( const std::string& fileName ) {

   bool status = true;

   if ( m_inputDataBase ) {
      status = m_inputDataBase->saveToFile( fileName );
   }

   if ( m_outputDataBase ) {
      database::Table* outputTablesFileTbl = m_inputDataBase->getTable ( OutputTablesFileIoTableName );

      if ( outputTablesFileTbl != nullptr and outputTablesFileTbl->size () == 1 ) {
         database::Record* record = outputTablesFileTbl->getRecord ( 0 );
         std::string outputTableFileName = database::getFilename ( record );

         std::string outputDirectory = stripExtension ( fileName ) + Names::CauldronOutputDir;

         ibs::FolderPath( outputDirectory ).create(); // checks for directory existence inside

         std::string outputFileName = outputDirectory + "/" + outputTableFileName;
         status = m_outputDataBase->saveToFile ( outputFileName ) and status;
      }
   }
   return status;
}


bool database::ProjectFileHandler::saveInputDataBaseToStream ( std::ostream& os ) {

   return m_inputDataBase ? m_inputDataBase->saveToStream ( os ) : false;
}


void database::ProjectFileHandler::loadFromFile ( const std::string& fileName ) {

   database::DataSchema * cauldronSchema = database::createCauldronSchema();
   database::TableDefinition *tableDef = cauldronSchema->getTableDefinition( "DepthIoTbl" );

   if ( tableDef != nullptr ) {
      // Adding (volatile, won't be output) definition for DepositionSequence field
      // Required to properly sort the DepthIoTbl, not to be output
      tableDef->addVolatileFieldDefinition( "DepositionSequence", datatype::Int, "", "0" );
   }

   m_inputDataBase.reset( database::Database::CreateFromFile( fileName, *cauldronSchema ) );
   database::upgradeAllTablesInCauldronSchema( m_inputDataBase.get() );
   
   // Now that we have the cauldron schema, use it to set the list of all possible table names.
   m_allTableNames.reserve ( cauldronSchema->size ());

   for ( size_t i = 0; i < cauldronSchema->size (); ++i ) {
      m_allTableNames.push_back ( cauldronSchema->getTableDefinition ( i )->name ());
   }

   delete cauldronSchema;
}


std::string database::ProjectFileHandler::stripExtension ( const std::string& fileName ) const {

   std::string projectName = fileName;
   std::string::size_type dotPos = projectName.rfind( ".project" );

   if ( dotPos != std::string::npos )
   {
      projectName.erase( dotPos, std::string::npos );
   }

   return projectName;
}


void database::ProjectFileHandler::loadOutputTables () {

   database::Table* outputTablesFileTbl = m_inputDataBase->getTable ( OutputTablesFileIoTableName );
   database::Table* outputTablesTbl     = getTable ( OutputTablesIoTableName );

   if ( outputTablesFileTbl != nullptr and outputTablesFileTbl->size () == 1 and outputTablesTbl != nullptr and outputTablesTbl->size () > 0 ) {
      database::Record* record        = outputTablesFileTbl->getRecord ( 0 );
      std::string outputTableFileName = database::getFilename ( record );
      std::string outputDirectory     = stripExtension ( m_inputDataBase->getFileName ()) + Names::CauldronOutputDir;
      std::string fullFileName        = outputDirectory + "/" + outputTableFileName;
      bool        tableHasBeenAdded   = false;
      std::unique_ptr<database::DataSchema>  outputSchema( new database::DataSchema );

      for ( size_t i = 0; i < outputTablesTbl->size (); ++i ) {
         record                       = outputTablesTbl->getRecord ( (int)i );
         const std::string& tableName = database::getTableName ( record );
         database::Table*   table     = m_inputDataBase->getTable ( tableName );

         if ( tableName == OutputTablesFileIoTableName or tableName == OutputTablesIoTableName ) {
            LogHandler ( LogHandler::ERROR_SEVERITY ) << "(" << __FUNCTION__ << ") Cannot make table " << tableName << " an output table.";
         } else if ( table != nullptr ) {
            const database::TableDefinition& tableDefinition = table->getTableDefinition ();
            // The table should be populated in either the input project file or the
            // output tables file so copying of the data here is not necessary.
            outputSchema->addTableDefinition ( tableDefinition.deepCopy ());
            m_inputDataBase->deleteTable (     tableDefinition.name ());
            tableHasBeenAdded = true;
         } else {
            LogHandler ( LogHandler::ERROR_SEVERITY ) << "(" << __FUNCTION__ << ") Cannot find table name " << tableName;
         }

      }

      if ( tableHasBeenAdded ) {

         if ( ibs::FolderPath ( outputDirectory ).exists() and
              ibs::FilePath ( fullFileName ).exists() ) {
            m_outputDataBase.reset( database::Database::CreateFromFile ( fullFileName, *(outputSchema.get()) ) );
         } else {
            LogHandler ( LogHandler::WARNING_SEVERITY ) << "(" << __FUNCTION__ << ") Output tables file does not exist: "
                                                        << outputDirectory << "/" << fullFileName;
            m_outputDataBase.reset( database::Database::CreateFromSchema ( *(outputSchema.get()) ) );
         }

      }

   } else if ( outputTablesFileTbl != nullptr and outputTablesFileTbl->size () > 1 ) {
      LogHandler ( LogHandler::ERROR_SEVERITY ) << "(" << __FUNCTION__ << ") " << outputTablesFileTbl->name () << " table has more than 1 row";
   }

}


database::Table* database::ProjectFileHandler::getTable ( const std::string& tableName ) const {

   database::Table* table = m_inputDataBase->getTable ( tableName );

   if ( table == nullptr and m_outputDataBase != nullptr ) {
      table = m_outputDataBase->getTable ( tableName );
   }

   return table;
}


bool database::ProjectFileHandler::setTableAsOutput ( const std::string& tableName ) {

   bool status = false;

   if (        m_outputDataBase and m_outputDataBase->getTable ( tableName ) != nullptr ) {
      LogHandler ( LogHandler::WARNING_SEVERITY ) << "(" << __FUNCTION__ << ") Table " << tableName << " is already an output table.";
   } else if ( tableName == OutputTablesFileIoTableName or tableName == OutputTablesIoTableName ) {
      LogHandler ( LogHandler::ERROR_SEVERITY   ) << "(" << __FUNCTION__ << ") Cannot make table " << tableName << " an output table.";
   } else if ( m_inputDataBase->getTable ( tableName ) == nullptr ) {
      LogHandler ( LogHandler::ERROR_SEVERITY   ) << "(" << __FUNCTION__ << ") Table " << tableName << " cannot be found in the input data schema.";
   } else {

      const database::DataSchema& inputSchema = m_inputDataBase->getDataSchema ();
      // tableDefinition cannot be null here, because m_inputDataBase->getTable ( tableName )
      // would have returned a nullptr and would have been caught above
      database::TableDefinition* tableDefinition = inputSchema.getTableDefinition ( tableName );

      database::Table*  inputTable          = m_inputDataBase->getTable ( tableName );
      database::Table*  outputTablesFileTbl = m_inputDataBase->getTable ( OutputTablesFileIoTableName );
      database::Table*  outputTablesTbl     = m_inputDataBase->getTable ( OutputTablesIoTableName );
      database::Record* record              = outputTablesTbl->createRecord ();

      database::setTableName ( record, tableName );

      if ( m_outputDataBase == nullptr ) {
         database::DataSchema* outputSchema = new database::DataSchema;
         outputSchema->addTableDefinition ( tableDefinition->deepCopy ());
         m_outputDataBase.reset( database::Database::CreateFromSchema ( *outputSchema ) );
         delete outputSchema;
      } else {
         m_outputDataBase->addTableDefinition ( tableDefinition->deepCopy ());
      }

      if ( outputTablesFileTbl->size () == 0 ) {
         database::Record* record = outputTablesFileTbl->createRecord ();
         database::setFilename ( record, OutputTablesFileName );
      }

      database::Table* outputTable = m_outputDataBase->getTable ( tableName );
      inputTable->copyTo ( outputTable );
      inputTable->clear ();
      inputTable = nullptr;
      m_inputDataBase->deleteTable ( tableName );
      status = true;
   }

   return status;
}

void database::ProjectFileHandler::mergeTablesToInput () {

   if ( m_outputDataBase != nullptr ) {
      database::Table* outputTablesFileTbl = m_inputDataBase->getTable ( OutputTablesFileIoTableName );
      database::Table* outputTablesTbl     = m_inputDataBase->getTable ( OutputTablesIoTableName );

      if ( outputTablesTbl->size () > 0 ) {

         for ( int i = 0; i < outputTablesTbl->size (); ++i ) {
            std::string tableName = database::getTableName ( outputTablesTbl->getRecord ( i ));
            database::Table* outputTable = m_outputDataBase->getTable ( tableName );

            // Do not add the table definition to the input database if it has it already.
            if ( outputTable != nullptr and m_inputDataBase->getTable ( outputTable->name ()) == nullptr ) {
               m_inputDataBase->addTableDefinition ( outputTable->getTableDefinition ().deepCopy ());

               database::Table* inputTable = m_inputDataBase->getTable ( tableName );
               outputTable->copyTo ( inputTable );
               outputTable->clear ();
               m_outputDataBase->deleteTable ( tableName );
            }

         }

         // Now that all the tables have been moved to the input database the table can be cleared.
         outputTablesTbl->clear ();
         outputTablesFileTbl->clear ();
      }

      m_outputDataBase.reset( nullptr );
   }

}
