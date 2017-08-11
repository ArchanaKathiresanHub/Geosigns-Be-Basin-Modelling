#include "project3dparameter.h"
#include "parser.h"

#include "test.project3d.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST( Project3DParameterParse, explicitNumber)
{
   boost::shared_ptr<hpc::Project3DParameter> param = 
            hpc::Project3DParameter::parse("RunOptionsIoTbl .  CompactionAlgorithm : string . 0");

   EXPECT_EQ( hpc::ExplicitProject3DParameter( 
                "RunOptionsIoTbl",
                "CompactionAlgorithm", 
                hpc::Project3DParameter::STRING, 
                0
                ),
              *param
            ) ;
}




TEST( Project3DParameterParse, explicitPattern)
{
   boost::shared_ptr<hpc::Project3DParameter> param
      = hpc::Project3DParameter::parse("StratIoTbl .  ElementRefinementZ : int . *");

   EXPECT_EQ( hpc::ExplicitProject3DParameter(
                "StratIoTbl",
                "ElementRefinementZ",
                hpc::Project3DParameter::INT,
                -1
                ),
              *param
            );
}


TEST( Project3DParameterParse, implicit)
{
   boost::shared_ptr<hpc::Project3DParameter> param
      = hpc::Project3DParameter::parse(
              "LithotypeIoTbl . Lithotype : string .[Density : double = 3200]"
         );

   EXPECT_EQ( hpc::ImplicitProject3DParameter(
                "LithotypeIoTbl", "Lithotype", hpc::Project3DParameter::STRING,
                "Density", hpc::Project3DParameter::DOUBLE, "3200"
                ),
              *param
            );
}


TEST( Project3DParameterParse, choice)
{
   using namespace hpc;
   using ::testing::ElementsAre;

   boost::shared_ptr<Project3DParameter> param
      = Project3DParameter::parse(
      "RunOptionsIoTbl . TempDiffBasedStepping : int . 0 { BurialRateTimeStepping = 0, TempDiffTimeStepping = 1 }"
        );

   std::vector<std::string> names, values;
   names.push_back("BurialRateTimeStepping");
   names.push_back("TempDiffTimeStepping");
   values.push_back("0");
   values.push_back("1");
   boost::shared_ptr<Project3DParameter> expectParameter(
         new ChoiceProject3DParameter( 
            boost::shared_ptr<Project3DParameter>(
              new ExplicitProject3DParameter( 
                 "RunOptionsIoTbl",
                 "TempDiffBasedStepping",
                 Project3DParameter::INT, 
                 0
                 )
              ),
            names,
            values
            )
         );
            

   EXPECT_EQ( *expectParameter, *param);
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

class Project3DParameterParseTypeTest : public hpc::Project3DParameter, public ::testing::Test
{
public:
   virtual std::string readValue(const DataAccess::Interface::ProjectHandle *) const
   { return ""; }

   virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string &) const
   { }

   virtual bool isEqual(const Project3DParameter & ) const
   { return false; }

   virtual void print( std::ostream & output) const
   { }
};


TEST_F( Project3DParameterParseTypeTest, boolean)
{
   EXPECT_EQ( BOOL, parseType("bool"));
}

TEST_F( Project3DParameterParseTypeTest, integer)
{
   EXPECT_EQ( INT, parseType("int"));
}

TEST_F( Project3DParameterParseTypeTest, longinteger)
{
   EXPECT_EQ( LONG, parseType("long"));
}

TEST_F( Project3DParameterParseTypeTest, singlePrecisionFloat)
{
   EXPECT_EQ( FLOAT, parseType("float"));
}

TEST_F( Project3DParameterParseTypeTest, doublePrecisionFloat)
{
   EXPECT_EQ( DOUBLE, parseType("double"));
}

TEST_F( Project3DParameterParseTypeTest, string)
{
   EXPECT_EQ( STRING, parseType("string"));
}

TEST_F( Project3DParameterParseTypeTest, error)
{
   EXPECT_EQ( Type(6), parseType("error"));
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
   MOCK_CONST_METHOD1( isEqual, bool (const Project3DParameter &) );
   MOCK_CONST_METHOD1( print, void ( std::ostream & ) );
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

