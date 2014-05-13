#include "experiment.h"
#include "DatadrillerProperty.h"
#include "Property.h"
#include "parameter.h"
#include "SAUAParameters.h"

#include <sstream>
#include <iostream>

#include <gtest/gtest.h>

class NonEmpty : public Property
{
public:
   class Param : public Parameter
   {
   public:
      Param( int n ) : m_n( n ) {}
      virtual void  print( std::ostream & output ) { output << m_n; }

      virtual void changeParameter(Project & ) {}

      /// Returns true if parameter is continious (float)
      virtual bool isContinuous() const { return true; }

      /// Return vector of values for parameter if it could be converted to vector of doubles
      virtual std::vector<double> toDblVector() const { return std::vector<double>( 1, m_n ); }

      /// Set values for parameter if it could be set from vector of doubles
      virtual void fromDblVector( const std::vector<double> & prms ) { m_n = prms[0]; }

   private:
      int m_n;
   };

   NonEmpty( int mn, int mx ) : m_i(mn), m_mn(mn), m_mx(mx) {}

   virtual void reset() { m_i = m_mn; } 
   virtual void createParameter( Scenario & project ) const { project.addParameter( new Param( m_i ) ); }
   virtual void nextValue() { m_i++; }
   virtual void lastValue() { m_i = m_mx; }
   virtual bool isPastEnd() const { return m_i > m_mx; }

private:
   int m_mn;
   int m_mx;
   int m_i;
};


// input domain of Experiment::Experiment
//  *  params = 3 params, differnet DoE
//  none, one empty, one non-empty, 
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

// test case 1: three non-empty ranges, Tornado design
TEST_F( ExperimentExperimentTest, ThreeNonEmptyRangesTornadoDoE )
{  
   m_sauap.setDoE( "Tornado", "" );

   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(1,3)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(5,9)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(10,16)));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

   std::ostringstream s;
   e.printScenarios(s);

   std::cout << s << std::endl;

   ASSERT_EQ( "Scenarios of experiment\n"
              "  1) 2; 7; 13\n"
              "  2) 1; 7; 13\n"
              "  3) 3; 7; 13\n"
              "  4) 2; 5; 13\n"
              "  5) 2; 9; 13\n"
              "  6) 2; 7; 10\n"
              "  7) 2; 7; 16\n", s.str()
         );
}

// test case 2: three non-empty ranges,  Plackett-Burman design
TEST_F( ExperimentExperimentTest, ThreeNonEmptyRangesPlBrmDoE )
{  
   m_sauap.setDoE( "Plackett-Burman", "" );

   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(1,3)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(5,9)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(10,16)));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

   std::ostringstream s;
   e.printScenarios(s);

   std::cout << s << std::endl;

   ASSERT_EQ( "Scenarios of experiment\n"
              "  1) 3; 9; 16\n"
              "  2) 1; 9; 10\n"
              "  3) 3; 5; 10\n"
              "  4) 1; 5; 16\n", s.str()
         );
}

// test case 3: three non-empty ranges,  Box Behnken design
TEST_F( ExperimentExperimentTest, ThreeNonEmptyRangesBxBehnDoE )
{  
   m_sauap.setDoE( "Box Behnken", "" );

   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(1,3)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(5,9)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(10,16)));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

   std::ostringstream s;
   e.printScenarios(s);

   std::cout << s << std::endl;

   ASSERT_EQ( "Scenarios of experiment\n"               
              "  1) 2; 7; 13\n"
              "  2) 3; 9; 13\n"
              "  3) 1; 9; 13\n"
              "  4) 3; 5; 13\n"
              "  5) 1; 5; 13\n"
              "  6) 3; 7; 16\n"
              "  7) 1; 7; 16\n"
              "  8) 3; 7; 10\n"
              "  9) 1; 7; 10\n"
              " 10) 2; 9; 16\n"
              " 11) 2; 5; 16\n"
              " 12) 2; 9; 10\n"
              " 13) 2; 5; 10\n", s.str()
         );
}

// test case 5: three non-empty ranges,  Optimised LHD design
TEST_F( ExperimentExperimentTest, ThreeNonEmptyRangesOptimisedLHDDoE )
{  
   m_sauap.setDoE( "Optimised LHD", "13" );

   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(1,3)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(5,9)));
   m_params.push_back( boost::shared_ptr<Property>(new NonEmpty(10,16)));

   Experiment e( m_params, m_noDataDriller, m_noRTInfo, m_sauap );

   std::ostringstream s;
   e.printScenarios(s);

   std::cout << s << std::endl;

   ASSERT_EQ( "Scenarios of experiment\n"               
              "  1) 1; 8; 11\n"
              "  2) 1; 6; 13\n"
              "  3) 1; 7; 15\n"
              "  4) 1; 8; 13\n"
              "  5) 1; 5; 11\n"
              "  6) 1; 5; 14\n"
              "  7) 2; 7; 10\n"
              "  8) 2; 7; 15\n"
              "  9) 2; 5; 12\n"
              " 10) 2; 8; 13\n"
              " 11) 2; 7; 12\n"
              " 12) 2; 6; 10\n"
              " 13) 2; 6; 14\n", s.str()
         );
}

