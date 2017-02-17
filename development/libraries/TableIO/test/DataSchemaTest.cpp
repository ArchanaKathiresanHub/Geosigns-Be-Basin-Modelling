#include "../src/datatype.h"
#include "../src/dataschema.h"

#include <gtest/gtest.h>

TEST ( TableIoDataSchemaTest, FieldDefinitionTest ) {
   using namespace database;
   using namespace datatype;

   DataSchema dataSchema;

   TableDefinition* tableDefinition = dataSchema.addTableDefinition ( "MyTestTable1", "Description of MyTestTable1" );

   tableDefinition->addFieldDefinition ( "IntegerField1", Int, "", "1", 0 );
   tableDefinition->addFieldDefinition ( "IntegerField2", Int, "", "2", 1 );
   tableDefinition->addFieldDefinition ( "IntegerField3", Int, "", "3", 2 );

   tableDefinition = dataSchema.addTableDefinition ( "MyTestTable2", "Description of MyTestTable2" );

   tableDefinition->addFieldDefinition ( "StringField", String, "", "SomeText", 0 );
   tableDefinition->addFieldDefinition ( "FloatField",  Double, "", "1.23",     1 );

   //--------------------------------
   EXPECT_EQ ( dataSchema.size (), 2 );
   EXPECT_EQ ( dataSchema.getIndex ( "MyTestTable1" ),  0 );
   EXPECT_EQ ( dataSchema.getIndex ( "MyTestTable2" ),  1 );
   EXPECT_EQ ( dataSchema.getIndex ( "MyTestTable3" ), -1 );


   //--------------------------------
   // Test for first MyTestTable1
   tableDefinition = dataSchema.getTableDefinition ( "MyTestTable1" );
   EXPECT_NE ( tableDefinition, nullptr );
   // Should find the same table in the data schema
   EXPECT_EQ ( dataSchema.getIndex ( "MyTestTable1" ), 0 );
   EXPECT_EQ ( tableDefinition->size (), 3 );
   EXPECT_EQ ( tableDefinition->name (), "MyTestTable1" );

   // Check field names.
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 0 )->name (), "IntegerField1" );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 1 )->name (), "IntegerField2" );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 2 )->name (), "IntegerField3" );

   // Check field type.
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 0 )->dataType (), Int );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 1 )->dataType (), Int );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 2 )->dataType (), Int );

   // Check field default values.
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 0 )->defaultValue (), "1" );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 1 )->defaultValue (), "2" );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 2 )->defaultValue (), "3" );

   // Check field index values.
   EXPECT_EQ ( tableDefinition->getIndex ( "IntegerField1" ),  0 );
   EXPECT_EQ ( tableDefinition->getIndex ( "IntegerField2" ),  1 );
   EXPECT_EQ ( tableDefinition->getIndex ( "IntegerField3" ),  2 );
   EXPECT_EQ ( tableDefinition->getIndex ( "IntegerField4" ), -1 );


   //--------------------------------
   // Test for first MyTestTable2
   tableDefinition = dataSchema.getTableDefinition ( "MyTestTable2" );
   EXPECT_NE ( tableDefinition, nullptr );
   EXPECT_EQ ( tableDefinition->size (), 2 );
   EXPECT_EQ ( tableDefinition->name (), "MyTestTable2" );

   // Check field names.
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 0 )->name (), "StringField" );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 1 )->name (), "FloatField" );

   // Check field type.
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 0 )->dataType (), String );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 1 )->dataType (), Double );

   // Check field default values.
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 0 )->defaultValue (), "SomeText" );
   EXPECT_EQ ( tableDefinition->getFieldDefinition ( 1 )->defaultValue (), "1.23" );

   // Check field index values.
   EXPECT_EQ ( tableDefinition->getIndex ( "StringField" ), 0 );
   // This should return -1 because the field name is not a part of this table.
   EXPECT_EQ ( tableDefinition->getIndex ( "IntegerField1" ), -1 );
   EXPECT_EQ ( tableDefinition->getIndex ( "IntegerField4" ), -1 );


   //--------------------------------
   tableDefinition = dataSchema.getTableDefinition ( "ANonExistingTable" );
   EXPECT_EQ ( tableDefinition, nullptr );
}
