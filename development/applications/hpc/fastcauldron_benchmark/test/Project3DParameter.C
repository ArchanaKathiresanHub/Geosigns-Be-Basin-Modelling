#include "project3dparameter.h"
#include "parser.h"

#include "test.project3d.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST( Project3DParameterParse, explicitNumber)
{
   boost::shared_ptr<hpc::ExplicitProject3DParameter> param
      = boost::dynamic_pointer_cast<hpc::ExplicitProject3DParameter>(
            hpc::Project3DParameter::parse("RunOptionsIoTbl . CompactionAlgorithm : string . 0")
          );

   EXPECT_EQ( "RunOptionsIoTbl", param->m_table);
   EXPECT_EQ( "CompactionAlgorithm", param->m_field);
   EXPECT_EQ( 0, param->m_recordNumber);
   EXPECT_EQ( hpc::Project3DParameter::STRING, param->m_type);
}




TEST( Project3DParameterParse, explicitPattern)
{
   boost::shared_ptr<hpc::ExplicitProject3DParameter> param
      = boost::dynamic_pointer_cast<hpc::ExplicitProject3DParameter>(
            hpc::Project3DParameter::parse("StratIoTbl .  ElementRefinementZ : int . *")
          );

   EXPECT_EQ( "StratIoTbl", param->m_table);
   EXPECT_EQ( "ElementRefinementZ", param->m_field);
   EXPECT_EQ( -1, param->m_recordNumber);
   EXPECT_EQ( hpc::Project3DParameter::INT, param->m_type);
}


TEST( Project3DParameterParse, implicit)
{
   boost::shared_ptr<hpc::ImplicitProject3DParameter> param
      = boost::dynamic_pointer_cast<hpc::ImplicitProject3DParameter>(
            hpc::Project3DParameter::parse(
              "LithotypeIoTbl . Lithotype : string .[Density : double = 3200]"
         ) );

   EXPECT_EQ( "LithotypeIoTbl", param->m_table);
   EXPECT_EQ( "Lithotype", param->m_field);
   EXPECT_EQ( hpc::Project3DParameter::STRING, param->m_type);
   EXPECT_EQ( "Density", param->m_conditionField);
   EXPECT_EQ( "3200", param->m_conditionValue);
   EXPECT_EQ( hpc::Project3DParameter::DOUBLE, param->m_conditionalType); 
}


TEST( Project3DParameterParse, choice)
{
   using ::testing::ElementsAre;

   boost::shared_ptr<hpc::ChoiceProject3DParameter> param
      = boost::dynamic_pointer_cast<hpc::ChoiceProject3DParameter>(
            hpc::Project3DParameter::parse(
      "RunOptionsIoTbl . TempDiffBasedStepping : int . 0 { BurialRateTimeStepping = 0, TempDiffTimeStepping = 1 }"
        ) );

   boost::shared_ptr<hpc::ExplicitProject3DParameter> embeddedParam
      = boost::dynamic_pointer_cast<hpc::ExplicitProject3DParameter>( param->m_parameter) ;

   EXPECT_EQ( "RunOptionsIoTbl", embeddedParam->m_table );
   EXPECT_EQ( "TempDiffBasedStepping", embeddedParam->m_field );
   EXPECT_EQ( 0, embeddedParam->m_recordNumber);
   EXPECT_EQ( hpc::Project3DParameter :: INT, embeddedParam->m_type);

   EXPECT_THAT( param->m_names, ElementsAre( "BurialRateTimeStepping", "TempDiffTimeStepping") );
   EXPECT_THAT( param->m_values, ElementsAre("0", "1"));
}

TEST( Project3DParameterParse, recordSpecifierError)
{
   EXPECT_THROW( hpc::Project3DParameter::parse("Table.Field : int.Record"), hpc::ParseException);
   EXPECT_THROW( hpc::Project3DParameter::parse("Table.Field : int .12345678901234567890"), hpc::ParseException);
   EXPECT_THROW( hpc::Project3DParameter::parse("Table.Field : int.-1"), hpc::ParseException );
}

TEST( Project3DParameterParse, choiceError)
{
   EXPECT_THROW( hpc::Project3DParameter::parse("Table.Field : int.1 bla"), hpc::ParseException);
   EXPECT_THROW( hpc::Project3DParameter::parse("Table.Field : int.1 { bla = 1 p"), hpc::ParseException);
}

TEST( Project3DParameterParseType, boolean)
{
   EXPECT_EQ( hpc::Project3DParameter::BOOL, hpc::Project3DParameter::parseType("bool"));
}

TEST( Project3DParameterParseType, integer)
{
   EXPECT_EQ( hpc::Project3DParameter::INT, hpc::Project3DParameter::parseType("int"));
}

TEST( Project3DParameterParseType, longinteger)
{
   EXPECT_EQ( hpc::Project3DParameter::LONG, hpc::Project3DParameter::parseType("long"));
}

TEST( Project3DParameterParseType, singlePrecisionFloat)
{
   EXPECT_EQ( hpc::Project3DParameter::FLOAT, hpc::Project3DParameter::parseType("float"));
}

TEST( Project3DParameterParseType, doublePrecisionFloat)
{
   EXPECT_EQ( hpc::Project3DParameter::DOUBLE, hpc::Project3DParameter::parseType("double"));
}

TEST( Project3DParameterParseType, string)
{
   EXPECT_EQ( hpc::Project3DParameter::STRING, hpc::Project3DParameter::parseType("string"));
}

TEST( Project3DParameterParseType, error)
{
   EXPECT_EQ( hpc::Project3DParameter::Type(6), hpc::Project3DParameter::parseType("error"));
}

TEST( ExplicitProject3DParameterReadValue, explicitNumber)
{
   using namespace hpc;
   TestProject3DFile testFile("testFile");

   // normal case
   EXPECT_EQ( "Water bottom", 
         ExplicitProject3DParameter("StratIoTbl", "SurfaceName", Project3DParameter::STRING, 0).
         readValue(testFile.project())
       );

   // Table does not exist
   EXPECT_THROW(ExplicitProject3DParameter("Idontexist", "Idontexist", Project3DParameter::STRING, 0).
         readValue(testFile.project()), 
         Project3DParameter::QueryException
       );

   // table contains no records
   EXPECT_THROW(ExplicitProject3DParameter("MobLayThicknIoTbl", "LayerName", Project3DParameter::STRING, 0).
         readValue(testFile.project()), 
         Project3DParameter::QueryException
       );

}

TEST( ExplicitProject3DParameterReadValue, explicitPattern)
{
   using namespace hpc;
   TestProject3DFile testFile("testFile");

   // Table does not exist
   EXPECT_THROW(ExplicitProject3DParameter("Idontexist", "Idontexist", Project3DParameter::STRING, -1).
         readValue(testFile.project()), 
         Project3DParameter::QueryException
       );

   // when the table contains no records
   EXPECT_THROW(ExplicitProject3DParameter("MobLayThicknIoTbl", "LayerName", Project3DParameter::STRING,-1).
         readValue(testFile.project()), 
         Project3DParameter::QueryException
       );


   // table contains multiple records but with different field values
   EXPECT_THROW( hpc::ExplicitProject3DParameter("SnapshotIoTbl", "Time", hpc::Project3DParameter::DOUBLE, -1).
         readValue(testFile.project()),
         Project3DParameter::QueryException
       );

   // table contains one record
   EXPECT_EQ( "Effective Stress", 
         ExplicitProject3DParameter("RunOptionsIoTbl", "CompactionAlgorithm", Project3DParameter::STRING, -1).
         readValue(testFile.project())
       );

   // table contains multiple records with the same field value
   EXPECT_EQ( "System Generated", 
         ExplicitProject3DParameter("SnapshotIoTbl", "TypeOfSnapshot", Project3DParameter::STRING, -1).
         readValue(testFile.project())
       );
}

TEST( ExplicitProject3DParameterWriteValue, explicitNumber)
{
   using namespace hpc;
   TestProject3DFile testFile("testFile");

   // Table does not exist
   EXPECT_THROW(ExplicitProject3DParameter("Idontexist", "Idontexist", Project3DParameter::STRING, 0).
         writeValue(testFile.project(), "A string"), 
         Project3DParameter::QueryException
       );

   // table does not contain requested record
   EXPECT_THROW(ExplicitProject3DParameter("MobLayThicknIoTbl", "LayerName", Project3DParameter::STRING, 0).
         writeValue(testFile.project(), "Something"), 
         Project3DParameter::QueryException
       );

   // Successful write to project
   ExplicitProject3DParameter p("StratIoTbl", "SurfaceName", Project3DParameter::STRING, 0);
   p.writeValue(testFile.project(), "Another name");
   EXPECT_EQ( "Another name", p.readValue(testFile.project()) );
}

TEST( ExplicitProject3DParameterWriteValue, explicitPattern)
{
   using namespace hpc;
   TestProject3DFile testFile("testFile");

   // Table does not exist
   EXPECT_THROW(ExplicitProject3DParameter("Idontexist", "Idontexist", Project3DParameter::STRING, -1).
         writeValue(testFile.project(), "A string"), 
         Project3DParameter::QueryException
       );

   // table contains any number of records
   ExplicitProject3DParameter p("StratIoTbl", "ElementRefinementZ", Project3DParameter::INT, -1);
   p.writeValue(testFile.project(), "2");
   EXPECT_EQ( "2", p.readValue(testFile.project()) );
}

TEST( ImplicitProject3DParameterReadValue, normal )
{
   using namespace hpc;
   TestProject3DFile testFile("testFile");

   // normal case
   EXPECT_EQ( "Water bottom", 
         ImplicitProject3DParameter("StratIoTbl", "SurfaceName", Project3DParameter::STRING, 
            "LayerName", Project3DParameter::STRING, "Quaternary").
         readValue(testFile.project())
       );

   // Table does not exist
   EXPECT_THROW(ImplicitProject3DParameter("Idontexist", "Idontexist", Project3DParameter::STRING, 
            "LayerName", Project3DParameter::STRING, "Quaternary").
         readValue(testFile.project()), 
         Project3DParameter::QueryException
       );

   // Record not found
   EXPECT_THROW(ImplicitProject3DParameter("StratIoTbl", "SurfaceName", Project3DParameter::STRING,
            "LayerName", Project3DParameter::STRING, "Idontexist").
         readValue(testFile.project()), 
         Project3DParameter::QueryException
       );
}

TEST( ImplicitProject3DParameterWriteValue, normal )
{
   using namespace hpc;
   TestProject3DFile testFile("testFile");

   // Table does not exist
   EXPECT_THROW(ImplicitProject3DParameter("Idontexist", "Idontexist", Project3DParameter::STRING, 
            "LayerName", Project3DParameter::STRING, "Quaternary").
         writeValue(testFile.project(), "A string"), 
         Project3DParameter::QueryException
       );

   // Record not found
   EXPECT_THROW(ImplicitProject3DParameter("StratIoTbl", "SurfaceName", Project3DParameter::STRING,
            "LayerName", Project3DParameter::STRING, "Idontexist").
         writeValue(testFile.project(), "A string"), 
         Project3DParameter::QueryException
       );

   // normal case
   ImplicitProject3DParameter p("StratIoTbl", "SurfaceName", Project3DParameter::STRING, 
            "LayerName", Project3DParameter::STRING, "Quaternary");
   p.writeValue(testFile.project(), "Another name");
   EXPECT_EQ( "Another name", p.readValue(testFile.project()));
}

class MockProject3DParameter : public hpc::Project3DParameter
{
public:
   MOCK_CONST_METHOD1( readValue, std::string(const DataAccess::Interface::ProjectHandle *) );
   MOCK_CONST_METHOD2( writeValue, void( DataAccess::Interface::ProjectHandle *, const std::string &));
};

TEST( ChoiceProject3DParameterReadValue, normal )
{
   using namespace hpc;
   using ::testing::Return;

   // create mock embedded param
   boost::shared_ptr<Project3DParameter> embeddedParam( new MockProject3DParameter );

   EXPECT_CALL( dynamic_cast<MockProject3DParameter &>(*embeddedParam), readValue(0))
      .Times(2)
      .WillOnce(Return("X"))
      .WillOnce(Return("2"));
   
   // create choice parameter
   std::vector< std::string > names; names.push_back( "A"); names.push_back("B"); names.push_back("C");
   std::vector< std::string > values; values.push_back("1"); values.push_back("2"); values.push_back("3");
   ChoiceProject3DParameter p( embeddedParam, names, values);


   // returns an unexpected value (it reads "X" instead one of "1", "2", or "3"
   EXPECT_THROW( p.readValue(0), Project3DParameter::QueryException );

   // normal case:
   EXPECT_EQ( "B", p.readValue(0) );
}

