#include "../src/datatype.h"
#include "../src/dataschema.h"
#include "../src/database.h"
#include "../src/ProjectFileHandler.h"
using namespace database;
using namespace datatype;

#include <string>
#include <vector>
#include <sstream>

#include <gtest/gtest.h>

static const std::string ReferenceProject = "Reference.project3d";
static const std::string ReferenceProjectWithOutputTables = "ReferenceWithOutput.project3d";
static const std::string ReferenceProjectOutputTables = "ReferenceWithOutput_CauldronOutputDir/Output.iotables3d";
static const std::string TestFile = "ReferenceTestResult.project3d";
static const std::string TestProjectOutputTables = "ReferenceTestResult_CauldronOutputDir/Output.iotables3d";

void compareTables ( Table* correctTable, Table* testTable );

// Test the table names
TEST ( ProjectFileHandlerTest, TestTableNames ) {

   ProjectFileHandler* reference = new ProjectFileHandler ( ReferenceProject );
   ProjectFileHandler* referenceWithTables = new ProjectFileHandler ( ReferenceProjectWithOutputTables );

   const std::vector<std::string>& referenceNames = reference->getAllTableNames ();
   const std::vector<std::string>& referenceWithOutputNames = referenceWithTables->getAllTableNames ();

   EXPECT_EQ ( referenceNames.size (), referenceWithOutputNames.size ());

   for ( size_t i = 0; i < referenceNames.size (); ++i ) {
      EXPECT_EQ ( referenceNames [ i ], referenceWithOutputNames [ i ]);
   }

   delete reference;
   delete referenceWithTables;
}

// The reference table is contained in the project file, the referenceWithTables is contained in
// the Output.iotables3d file. The output tables are compared.
TEST ( ProjectFileHandlerTest, SetTableAsOutputTest ) {

   ProjectFileHandler* reference = new ProjectFileHandler ( ReferenceProject );
   ProjectFileHandler* referenceWithTables = new ProjectFileHandler ( ReferenceProjectWithOutputTables );

   reference->setTableAsOutput ( "TimeIoTbl" );

   compareTables ( referenceWithTables->getTable ( "TimeIoTbl" ), reference->getTable ( "TimeIoTbl" ));
   compareTables ( referenceWithTables->getTable ( "3DTimeIoTbl" ), reference->getTable ( "3DTimeIoTbl" ));

   reference->setTableAsOutput ( "3DTimeIoTbl" );
   compareTables ( referenceWithTables->getTable ( "3DTimeIoTbl" ), reference->getTable ( "3DTimeIoTbl" ));

   delete reference;
   delete referenceWithTables;
}

// The reference table is contained in the project file, the referenceWithTables is contained in
// the Output.iotables3d file. The output tables are compared.
TEST ( ProjectFileHandlerTest, SetDifferentTableAsOutputTest ) {

   ProjectFileHandler* reference = new ProjectFileHandler ( ReferenceProject );
   ProjectFileHandler* referenceWithTables = new ProjectFileHandler ( ReferenceProjectWithOutputTables );

   reference->setTableAsOutput ( "3DTimeIoTbl" );

   compareTables ( referenceWithTables->getTable ( "TimeIoTbl" ), reference->getTable ( "TimeIoTbl" ));

   delete reference;
   delete referenceWithTables;
}

// The reference table is contained in the project file, the referenceWithTables is contained in
// the Output.iotables3d file. The output tables are compared.
TEST ( ProjectFileHandlerTest, SetManyTablesAsOutputTest ) {

   ProjectFileHandler* reference = new ProjectFileHandler ( ReferenceProject );
   ProjectFileHandler* referenceWithTables = new ProjectFileHandler ( ReferenceProjectWithOutputTables );

   EXPECT_EQ ( reference->setTableAsOutput ( "TimeIoTbl" ), true );
   EXPECT_EQ ( reference->setTableAsOutput ( "3DTimeIoTbl" ), true );
   EXPECT_EQ ( reference->setTableAsOutput ( "1DTimeIoTbl" ), true );
   EXPECT_EQ ( reference->setTableAsOutput ( "TrapperIoTbl" ), true );
   EXPECT_EQ ( reference->setTableAsOutput ( "TrapIoTbl" ), true );
   // Cannot set the table OutputTablesFileIoTbl as an output table.
   EXPECT_EQ ( reference->setTableAsOutput ( "OutputTablesFileIoTbl" ), false );
   EXPECT_EQ ( reference->setTableAsOutput ( "AnUnknownTableName" ), false );

   compareTables ( referenceWithTables->getTable ( "TimeIoTbl" ), reference->getTable ( "TimeIoTbl" ));
   compareTables ( referenceWithTables->getTable ( "3DTimeIoTbl" ), reference->getTable ( "3DTimeIoTbl" ));
   compareTables ( referenceWithTables->getTable ( "1DTimeIoTbl" ), reference->getTable ( "1DTimeIoTbl" ));

   delete reference;
   delete referenceWithTables;
}

// The reference table is contained in the project file, the referenceWithTables is contained in
// the Output.iotables3d file. The output tables are compared.
TEST ( ProjectFileHandlerTest, ConstructTableAsOutputTest ) {

   std::vector<std::string> outputTables ({ "TimeIoTbl", "3DTimeIoTbl" });

   ProjectFileHandler* reference = new ProjectFileHandler ( ReferenceProject, outputTables );
   ProjectFileHandler* referenceWithTables = new ProjectFileHandler ( ReferenceProjectWithOutputTables );

   compareTables ( referenceWithTables->getTable ( "TimeIoTbl" ),   reference->getTable ( "TimeIoTbl" ));
   compareTables ( referenceWithTables->getTable ( "3DTimeIoTbl" ), reference->getTable ( "3DTimeIoTbl" ));

   delete reference;
   delete referenceWithTables;
}

// The reference project is loaded, then the TimeIoTbl is set as an output table. The project
// is then saved under a different name. This project is then reopened and the TimeioTbl and 3DTimeIoTbl tables
// compared with that of the referenceWithTables project.
TEST ( ProjectFileHandlerTest, SetTableAsOutputTestSaveAndOpen ) {

   ProjectFileHandler* reference = new ProjectFileHandler ( ReferenceProject );
   ProjectFileHandler* referenceWithTables = new ProjectFileHandler ( ReferenceProjectWithOutputTables );

   reference->setTableAsOutput ( "TimeIoTbl" );
   reference->saveToFile ( TestFile );
   ProjectFileHandler* testFile = new ProjectFileHandler ( TestFile );

   compareTables ( referenceWithTables->getTable ( "TimeIoTbl" ), reference->getTable ( "TimeIoTbl" ));

   delete reference;
   delete referenceWithTables;
   delete testFile;

}

void compareTables ( Table* correctTable, Table* testTable ) {

   EXPECT_NE ( correctTable, nullptr );
   EXPECT_NE ( testTable,    nullptr );

   EXPECT_EQ ( correctTable->size (), testTable->size ());

   const TableDefinition& correctTableDefinition = correctTable->getTableDefinition ();
   EXPECT_EQ ( correctTableDefinition.size (), testTable->getTableDefinition ().size ());

   for ( int i = 0; i < correctTable->size (); ++i ) {
      Record* correctRecord = correctTable->getRecord ( i );
      Record* testRecord    = testTable->getRecord ( i );
      EXPECT_EQ ( correctRecord->tableName (), testRecord->tableName ());

      for ( int j = 0; j < correctTableDefinition.size (); ++j ) {
         FieldDefinition* fieldDef = correctTableDefinition.getFieldDefinition ( j );

         EXPECT_NE ( fieldDef, nullptr );

         if ( fieldDef != nullptr ) {
            EXPECT_NE ( testRecord->getIndex ( fieldDef->name ()), -1 );

            switch ( fieldDef->dataType ()) {

               case datatype::String : {
                  std::string correctValue = correctRecord->getValue<std::string>( fieldDef->name (), nullptr );
                  std::string testValue    =    testRecord->getValue<std::string>( fieldDef->name (), nullptr );
                  EXPECT_EQ ( correctValue, testValue );
                  break;
               }

               case datatype::Double : {
                  double correctValue = correctRecord->getValue<double>( fieldDef->name (), nullptr );
                  double testValue    =    testRecord->getValue<double>( fieldDef->name (), nullptr );
                  EXPECT_EQ ( correctValue, testValue );
                  break;
               }

               case datatype::Int : {
                  int correctValue = correctRecord->getValue<int>( fieldDef->name (), nullptr );
                  int testValue    =    testRecord->getValue<int>( fieldDef->name (), nullptr );
                  EXPECT_EQ ( correctValue, testValue );
                  break;
               }

            }

         }

      }

   }

}
