#include "experiment.h"
#include "DatadrillerProperty.h"
#include "Property.h"
#include "parameter.h"

#include <sstream>
#include <iostream>


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


void test_Experiment_Experiment()
{
   // input domain of Experiment::Experiment
   //  *  params = none, one empty, one non-empty, 
   //              one empty and one non-empty, one non-empty and one empty,
   //              multiple non-empty and one empty, multiple non-empty
   //  *  dataDriller = any
   //  *  dataInfo = any

   std::vector< DatadrillerProperty > noDataDriller;
   RuntimeConfiguration noRTInfo( "", "", "");

   // test case 1: empty set of properties
   {  std::vector< boost::shared_ptr< Property > > params;
      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Scenarios of experiment\n  1) NONE\n" );
   }


   // test case 2: one property with empty range
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new Empty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Experiment has no cases\n" );
   }

   // test case 2: one property with non-empty range
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new NonEmpty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Scenarios of experiment\n"
                         "  1) 0\n"
                         "  2) 1\n"
                         "  3) 2\n"
            );
   }

   // test case 3: one property with non-empty range and empty range
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new NonEmpty));
      params.push_back( boost::shared_ptr<Property>(new Empty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Experiment has no cases\n" );
   }

   // test case 4: one property with empty range and one with non-empty range
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new Empty));
      params.push_back( boost::shared_ptr<Property>(new NonEmpty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Experiment has no cases\n" );
   }

   // test case 5: one empty range between two non-empty ranges
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new NonEmpty));
      params.push_back( boost::shared_ptr<Property>(new Empty));
      params.push_back( boost::shared_ptr<Property>(new NonEmpty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Experiment has no cases\n" );
   }

   // test case 5: three non-empty ranges
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new NonEmpty));
      params.push_back( boost::shared_ptr<Property>(new NonEmpty));
      params.push_back( boost::shared_ptr<Property>(new NonEmpty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Scenarios of experiment\n"
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
                         " 27) 2; 2; 2\n"
            );
   }


}


int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "Experiment")==0)
      test_Experiment_Experiment();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }

   return 0;
}
