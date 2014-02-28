#include "experiment.h"
#include "DatadrillerProperty.h"
#include "Property.h"
#include "parameter.h"
#include "SAUAParameters.h"

#include <sstream>
#include <iostream>

#include <gtest/gtest.h>

class Empty : public Property
{
public:
   virtual void reset() {}
   virtual void createParameter( Scenario & project ) const {}
   virtual void nextValue() {}
   virtual void lastValue() {}
   virtual bool isPastEnd() const { return true; }
};

class NonEmpty : public Property
{
public:
   class Param : public Parameter
   {
   public:
      Param(int n ) : m_n(n) {}
      virtual void  print( std::ostream & output ) { output << m_n; }

      virtual void changeParameter(Project & ) {}

      /// Returns true if parameter is continious (float)
      virtual bool isContinuous() const { return false; }

      /// Return vector of values for parameter if it could be converted to vector of doubles
      virtual std::vector<double> toDblVector() const { return std::vector<double>( 1, m_n ); }

      /// Set values for parameter if it could be set from vector of doubles
      virtual void fromDblVector( const std::vector<double> & prms ) { m_n = prms[0]; }

   private:
      int m_n;
   };

   NonEmpty() : m_i(0) {}
   virtual void reset() { m_i = 0; } 
   virtual void createParameter( Scenario & project ) const { project.addParameter( new Param( m_i ) ); }
   virtual void nextValue() { m_i++; }
   virtual void lastValue() { m_i = 2; }
   virtual bool isPastEnd() const { return m_i >= 3; }

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
   std::vector< boost::shared_ptr<Property> > m_params;
   std::vector<DatadrillerProperty>           m_noDataDriller;
   RuntimeConfiguration                       m_noRTInfo;
   SAUAParameters                             m_sauap;
};

// test case 1: empty set of properties
TEST_F( ExperimentExperimentTest, NoProps )
{  
   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Scenarios of experiment\n  1) NONE\n" ,  s.str() );
}


// test case 2: one property with empty range
TEST_F( ExperimentExperimentTest, OnePropWithEmptyRange )
{  
   m_params.push_back( boost::shared_ptr<Property>(new Empty));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Experiment has no scenarios\n" ,  s.str() );
}

// test case 2: one property with non-empty range
TEST_F( ExperimentExperimentTest, OnePropWithNonEmptyRange )
{  
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));
   SAUAParameters sauap;

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

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

   Experiment e(m_params, m_noDataDriller, m_noRTInfo, m_sauap );

   std::ostringstream s;
   e.printScenarios(s);

   ASSERT_EQ( "Experiment has no scenarios\n" ,  s.str() );
}

// test case 4: one property with empty range and one with non-empty range
TEST_F( ExperimentExperimentTest, OnePropWithEmptyRangeAndOnePropWithNonEmptyRange )
{ 
   m_params.push_back( boost::shared_ptr<Property>(new Empty));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

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

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

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

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

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

