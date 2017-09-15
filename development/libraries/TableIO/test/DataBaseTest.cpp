#include "../src/datatype.h"
#include "../src/dataschema.h"
#include "../src/database.h"
using namespace database;
using namespace datatype;

#include <string>
#include <sstream>

#include <gtest/gtest.h>

const std::string Table1 = "MyTestTable1";
const std::string Table2 = "MyTestTable2";

DataSchema* createSchema ();
Database*   createDataBase ();

std::string getExpectedContents ();

int getIntegerField1 ( Record* rec );
int getIntegerField2 ( Record* rec );
int getIntegerField3 ( Record* rec );

std::string getStringField ( Record* rec );
double      getFloatField  ( Record* rec );

void setIntegerField1 ( Record* rec, int i );
void setIntegerField2 ( Record* rec, int i );
void setIntegerField3 ( Record* rec, int i );

void setStringField ( Record* rec, const std::string& s );
void setFloatField  ( Record* rec, double d );

TEST ( TableIoDataBaseTest, FieldDefinitionTest ) {

   Database* database =  createDataBase ();
   Table* table;
   Record* rec;
   std::stringstream buffer;

   EXPECT_EQ ( database->size (), 2 );


   //--------------------------------
   // Test table 1
   EXPECT_EQ ( database->hasTable ( Table1 ), true );
   EXPECT_NE ( database->getTable ( Table1 ), nullptr );

   table = database->getTable ( 0 );
   EXPECT_EQ ( table->size (), 2 );
   EXPECT_EQ ( table->version(), 100 );

   rec = table->getRecord ( 0 );
   EXPECT_EQ ( getIntegerField1 ( rec ), 10 );
   EXPECT_EQ ( getIntegerField2 ( rec ), 20 );
   EXPECT_EQ ( getIntegerField3 ( rec ), 30 );

   rec = table->getRecord ( 1 );
   EXPECT_EQ ( getIntegerField1 ( rec ), 11 );
   EXPECT_EQ ( getIntegerField2 ( rec ), 21 );
   EXPECT_EQ ( getIntegerField3 ( rec ), 31 );


   //--------------------------------
   // Test table 2
   EXPECT_EQ ( database->hasTable ( Table2 ), true );
   EXPECT_NE ( database->getTable ( Table2 ), nullptr );

   table = database->getTable ( 1 );
   EXPECT_EQ ( table->size (), 3 );
   EXPECT_EQ ( table->version(), 100 );

   rec = table->getRecord ( 0 );
   EXPECT_EQ ( getStringField ( rec ), "Field1" );
   EXPECT_EQ ( getFloatField  ( rec ), 10.0 );

   rec = table->getRecord ( 1 );
   EXPECT_EQ ( getStringField ( rec ), "Field2" );
   EXPECT_EQ ( getFloatField  ( rec ), 20.0 );

   rec = table->getRecord ( 2 );
   EXPECT_EQ ( getStringField ( rec ), "Field3" );
   EXPECT_EQ ( getFloatField  ( rec ), 30.0 );

   // Get second record again.
   rec = table->getRecord ( 1 );
   EXPECT_EQ ( getStringField ( rec ), "Field2" );
   EXPECT_EQ ( getFloatField  ( rec ), 20.0 );

   database->clearTable ( Table2 );
   table = database->getTable ( 1 );
   EXPECT_EQ ( table->size (), 0 );


   //--------------------------------
   // Operations on non existing table.
   EXPECT_EQ ( database->hasTable ( "" ), false );
   EXPECT_EQ ( database->hasTable ( "UnknownTableName" ), false );
   EXPECT_EQ ( database->getTable ( 10 ), nullptr );

   delete database;
}

TEST ( TableIoDataBaseTest, SaveToStreamTest ) {

   Database* database =  createDataBase ();
   std::stringstream contents;
   std::string expectedContents = getExpectedContents ();

   Database::SetFieldWidth ( 24 );
   database->saveToStream ( contents );

   EXPECT_EQ ( contents.str (), expectedContents );

   delete database;
}


DataSchema* createSchema () {

   DataSchema* dataSchema = new DataSchema;

   TableDefinition* tableDefinition = dataSchema->addTableDefinition ( Table1, "Description of MyTestTable1", 100 );

   tableDefinition->addFieldDefinition ( "IntegerField1", Int, "", "1", 0 );
   tableDefinition->addFieldDefinition ( "IntegerField2", Int, "", "2", 1 );
   tableDefinition->addFieldDefinition ( "IntegerField3", Int, "", "3", 2 );

   tableDefinition = dataSchema->addTableDefinition ( Table2, "Description of MyTestTable2", 100 );

   tableDefinition->addFieldDefinition ( "StringField", String, "", "SomeText", 0 );
   tableDefinition->addFieldDefinition ( "FloatField",  Double, "", "1.23",     1 );

   return dataSchema;
}

Database* createDataBase () {
   DataSchema* dataSchema = createSchema ();
   Database* database  = Database::CreateFromSchema ( *dataSchema );
   delete dataSchema;

   Table* table = database->getTable ( Table1 );
   Record* rec = table->createRecord ();

   setIntegerField1 ( rec, 10 );
   setIntegerField2 ( rec, 20 );
   setIntegerField3 ( rec, 30 );

   rec = table->createRecord ();

   setIntegerField1 ( rec, 11 );
   setIntegerField2 ( rec, 21 );
   setIntegerField3 ( rec, 31 );


   table = database->getTable ( Table2 );
   rec = table->createRecord ();

   setStringField ( rec, "Field1" );
   setFloatField  ( rec, 10.0 );

   rec = table->createRecord ();

   setStringField ( rec, "Field2" );
   setFloatField  ( rec, 20.0 );

   rec = table->createRecord ();

   setStringField ( rec, "Field3" );
   setFloatField  ( rec, 30.0 );

   return database;
}

int getIntegerField1 ( Record* rec ) {
   static int index = 0;
   return rec->getValue<int>( "IntegerField1", &index );
}

int getIntegerField2 ( Record* rec ) {
   static int index = 1;
   return rec->getValue<int>( "IntegerField2", &index );
}

int getIntegerField3 ( Record* rec ) {
   static int index = 2;
   return rec->getValue<int>( "IntegerField3", &index );
}

std::string getStringField ( Record* rec ) {
   static int index = 0;
   return rec->getValue<std::string>( "StringField", &index );
}

double getFloatField  ( Record* rec ) {
   static int index = 1;
   return rec->getValue<double>( "FloatField", &index );
}

void setIntegerField1 ( Record* rec, int i ) {
   static int index = 0;
   rec->setValue<int>( "IntegerField1", i, &index );
}

void setIntegerField2 ( Record* rec, int i ) {
   static int index = 1;
   rec->setValue<int>( "IntegerField2", i, &index );
}

void setIntegerField3 ( Record* rec, int i ) {
   static int index = 2;
   rec->setValue<int>( "IntegerField3", i, &index );
}

void setStringField ( Record* rec, const std::string& s ) {
   static int index = 0;
   rec->setValue<std::string>( "StringField", s, &index );
}

void setFloatField  ( Record* rec, double d ) {
   static int index = 1;
   rec->setValue<double>( "FloatField", d, &index );
}


std::string getExpectedContents () {

   std::stringstream contents;

   contents << ";"                                                                            << std::endl;
   contents << "; Description of MyTestTable1"                                                << std::endl;
   contents << ";"                                                                            << std::endl;
   contents << "[MyTestTable1]"                                                               << std::endl;
   contents << ";v100"                                                                        << std::endl;
   contents << "           IntegerField1            IntegerField2            IntegerField3"   << std::endl;
   contents << "                      ()                       ()                       ()"   << std::endl;
   contents << "                      10                       20                       30"   << std::endl;
   contents << "                      11                       21                       31"   << std::endl;
   contents << "[End]"                                                                        << std::endl;
   contents << ";"                                                                            << std::endl;
   contents << ";"                                                                            << std::endl;
   contents << "; Description of MyTestTable2"                                                << std::endl;
   contents << ";"                                                                            << std::endl;
   contents << "[MyTestTable2]"                                                               << std::endl;
   contents << ";v100"                                                                        << std::endl;
   contents << "             StringField               FloatField"                            << std::endl;
   contents << "                      ()                       ()"                            << std::endl;
   contents << "                \"Field1\"                       10"                          << std::endl;
   contents << "                \"Field2\"                       20"                          << std::endl;
   contents << "                \"Field3\"                       30"                          << std::endl;
   contents << "[End]"                                                                        << std::endl;
   contents << ";"                                                                            << std::endl;

   return contents.str ();
}
