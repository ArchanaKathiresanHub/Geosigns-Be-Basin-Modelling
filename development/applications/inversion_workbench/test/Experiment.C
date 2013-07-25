#include "experiment.h"
#include "DatadrillerProperty.h"
#include "Property.h"
#include "parameter.h"

#include <sstream>
#include <iostream>

#include <gtest/gtest.h>

class Empty : public Property
{
public:
   virtual void reset() { } 
   virtual void createParameter(Scenario & project) const { }
   virtual void nextValue() { }
   virtual bool isPastEnd() const { return true; }
};

class NonEmpty : public Property
{
public:
   class Param : public Parameter
   {
   public:
      Param(int n ) : m_n(n) {}
      virtual void  print(std::ostream & output)
      { output << m_n; }

      virtual void changeParameter(Project & ) {}
   private:
      int m_n;
   };

   NonEmpty() : m_i(0) {}
   virtual void reset() { m_i = 0; } 
   virtual void createParameter(Scenario & project) const 
   { project.addParameter( new Param( m_i ) ) ;}
   virtual void nextValue() { m_i ++;  }
   virtual bool isPastEnd() const
   { return m_i >= 3; }

private:
   int m_i;
};


// input domain of Experiment::Experiment
//  *  params = none, one empty, one non-empty, 
//              one empty and one non-empty, one non-empty and one empty,
//              multiple non-empty and one empty, multiple non-empty
//  *  dataDriller = any
//  *  dataInfo = any

class ExperimentExperimentTest : public ::testing::Test
{
public:
   ExperimentExperimentTest()
      : m_params()
      , m_noDataDriller()
      , m_noRTInfo( "", "", "", "2012.1008", "-temperature")
   {}                                                            
protected:
   std::vector< boost::shared_ptr< Property > > m_params;
   std::vector< DatadrillerProperty > m_noDataDriller;
   RuntimeConfiguration m_noRTInfo;
};

// test case 1: empty set of properties
TEST_F( ExperimentExperimentTest, NoProps )
{  
   Experiment e( m_params, m_noDataDriller, m_noRTInfo );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Scenarios of experiment\n  1) NONE\n" ,  s.str() );
}


// test case 2: one property with empty range
TEST_F( ExperimentExperimentTest, OnePropWithEmptyRange )
{  
   m_params.push_back( boost::shared_ptr<Property>(new Empty));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Experiment has no scenarios\n" ,  s.str() );
}

// test case 2: one property with non-empty range
TEST_F( ExperimentExperimentTest, OnePropWithNonEmptyRange )
{  
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Scenarios of experiment\n"
              "  1) 0\n"
              "  2) 1\n"
              "  3) 2\n", s.str()
            );
}

// test case 3: one property with non-empty range and empty range
TEST_F( ExperimentExperimentTest, OnePropWithNonEmptyRangeAndOnePropWithEmptyRange )
{  
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));
   m_params.push_back( boost::shared_ptr<Property>(new Empty));

   Experiment e(m_params, m_noDataDriller, m_noRTInfo );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Experiment has no scenarios\n" ,  s.str() );
}

// test case 4: one property with empty range and one with non-empty range
TEST_F( ExperimentExperimentTest, OnePropWithEmptyRangeAndOnePropWithNonEmptyRange )
{ 
   m_params.push_back( boost::shared_ptr<Property>(new Empty));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));

   Experiment e(m_params, m_noDataDriller, m_noRTInfo );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Experiment has no scenarios\n" ,  s.str() );
}

// test case 5: one empty range between two non-empty ranges
TEST_F( ExperimentExperimentTest, OnePropWithEmptyRangeBetweenTwoNonEmptyRanges )
{ 
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));
   m_params.push_back( boost::shared_ptr<Property>(new Empty));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));

   Experiment e(m_params, m_noDataDriller, m_noRTInfo );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Experiment has no scenarios\n" ,  s.str() );
}

// test case 5: three non-empty ranges
TEST_F( ExperimentExperimentTest, ThreeNonEmptyRanges )
{  
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));

   Experiment e(m_params, m_noDataDriller, m_noRTInfo );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Scenarios of experiment\n"
              "  1) 0; 0; 0\n"
              "  2) 0; 0; 1\n"
              "  3) 0; 0; 2\n"
              "  4) 0; 1; 0\n"
              "  5) 0; 1; 1\n"
              "  6) 0; 1; 2\n"
              "  7) 0; 2; 0\n"
              "  8) 0; 2; 1\n"
              "  9) 0; 2; 2\n"
              " 10) 1; 0; 0\n"
              " 11) 1; 0; 1\n"
              " 12) 1; 0; 2\n"
              " 13) 1; 1; 0\n"
              " 14) 1; 1; 1\n"
              " 15) 1; 1; 2\n"
              " 16) 1; 2; 0\n"
              " 17) 1; 2; 1\n"
              " 18) 1; 2; 2\n"
              " 19) 2; 0; 0\n"
              " 20) 2; 0; 1\n"
              " 21) 2; 0; 2\n"
              " 22) 2; 1; 0\n"
              " 23) 2; 1; 1\n"
              " 24) 2; 1; 2\n"
              " 25) 2; 2; 0\n"
              " 26) 2; 2; 1\n"
              " 27) 2; 2; 2\n", s.str()
         );
}

class ExperimentPrintFieldTest : public ::testing::Test
{
protected:
   std::vector< boost::shared_ptr< Property > > m_properties;
   std::vector< DatadrillerProperty > m_probes;
};

   // case 1: first field = true, comma is field separator, no fixed width
TEST_F( ExperimentPrintFieldTest, SeparatorIsCommaVariableWidth )
{  RuntimeConfiguration info("", "", "", "", "", ',' ,0);
   std::ostringstream s;
   Experiment( m_properties, m_probes, info).printField(true, s);
   s << "x";
   ASSERT_EQ( "x",  s.str() );
}

   // case 2: first field = false, semi-colon is field separator, fixed width = 10
TEST_F( ExperimentPrintFieldTest, SeparatorIsSemicolonFixedWidth )
{  RuntimeConfiguration info("", "", "", "", "", ';' ,10);
   std::ostringstream s;
   Experiment( m_properties, m_probes, info).printField(false, s);
   s << "y";
   ASSERT_EQ( ";         y",  s.str() );
}

class ExperimentPrintTableTest : public ::testing::Test
{
public:
   ExperimentPrintTableTest()
      : m_properties()
      , m_probes()
      , m_info("", "", "", "", "", ',', 0)
      , m_table()                                          
   {}                                                            
protected:
   std::vector< boost::shared_ptr< Property > > m_properties;
   std::vector< DatadrillerProperty > m_probes;
   RuntimeConfiguration m_info;
   Experiment::ResultsTable m_table;
};

TEST_F( ExperimentPrintTableTest, noProbesNoEntries)
{ // case 1: no probes, no entries
   Experiment experiment(m_properties, m_probes, m_info);
   Experiment::ResultsTable m_table;

   std::ostringstream s;
   experiment.printTable(m_table, s);
   ASSERT_EQ( "X,Y,Z,Age\n",  s.str() );
}

TEST_F( ExperimentPrintTableTest, oneProbeOneEntry )
{ // case 1: one probe, one entry
   m_probes.push_back(DatadrillerProperty("A", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));

   Experiment experiment(m_properties, m_probes, m_info);
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 5.0));

   std::ostringstream s;
   experiment.printTable(m_table, s);
   ASSERT_EQ( "X,Y,Z,Age,A\n0,1,2,3,5\n",  s.str() );
}

TEST_F( ExperimentPrintTableTest, twoProbesFourEntries)
{ // case 2: two probes, four entries on the same space coordinate. two of them can be coalesced. The other two
   // cannot be merged with because the probe names collide
   m_probes.push_back(DatadrillerProperty("A", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
   m_probes.push_back(DatadrillerProperty("B", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));

   Experiment experiment(m_properties, m_probes, m_info);
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 5.0));
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 6.0));
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 7.0));
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 8.0));

   std::ostringstream s;
   experiment.printTable(m_table, s);
   ASSERT_EQ( "X,Y,Z,Age,A,B\n0,1,2,3,6, \n0,1,2,3,8,5\n0,1,2,3, ,7\n",  s.str() );
}

TEST_F( ExperimentPrintTableTest, threeProbesFourEntries)
{ // case 3: three probes, four entries on the same space coordinate. three of them can be coalesced. The fourth
   // cannot be merged with another, because the position is different
   m_probes.push_back(DatadrillerProperty("A", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
   m_probes.push_back(DatadrillerProperty("B", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
   m_probes.push_back(DatadrillerProperty("C", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));

   Experiment experiment(m_properties, m_probes, m_info);
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 5.0));
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 6.0));
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 2.0, 3.0, 3.0), 2, 7.0));
   m_table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 2, 8.0));

   std::ostringstream s;
   experiment.printTable(m_table, s);
   ASSERT_EQ( "X,Y,Z,Age,A,B,C\n0,1,2,3,5,6,8\n0,2,3,3, , ,7\n",  s.str() );
}


