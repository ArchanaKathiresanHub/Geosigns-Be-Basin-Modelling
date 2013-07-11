#include "experiment.h"
#include "DatadrillerProperty.h"
#include "Property.h"
#include "parameter.h"

#include <sstream>
#include <iostream>

int verbose = false;

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
   RuntimeConfiguration noRTInfo( "", "", "", "2012.1008", "-temperature");

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

      assert( s.str() == "Experiment has no scenarios\n" );
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

      assert( s.str() == "Experiment has no scenarios\n" );
   }

   // test case 4: one property with empty range and one with non-empty range
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new Empty));
      params.push_back( boost::shared_ptr<Property>(new NonEmpty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Experiment has no scenarios\n" );
   }

   // test case 5: one empty range between two non-empty ranges
   {  std::vector< boost::shared_ptr< Property > > params;

      params.push_back( boost::shared_ptr<Property>(new NonEmpty));
      params.push_back( boost::shared_ptr<Property>(new Empty));
      params.push_back( boost::shared_ptr<Property>(new NonEmpty));

      Experiment e(params, noDataDriller, noRTInfo );

      std::ostringstream s;
      e.printScenarios(s);

      assert( s.str() == "Experiment has no scenarios\n" );
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

void test_Experiment_printField()
{
   std::vector< boost::shared_ptr<Property> > properties;
   std::vector< DatadrillerProperty > probes;

   // case 1: first field = true, comma is field separator, no fixed width
   {  RuntimeConfiguration info("", "", "", "", "", ',' ,0);
      std::ostringstream s;
      Experiment( properties, probes, info).printField(true, s);
      s << "x";
      assert( s.str() == "x");
   }

   // case 2: first field = false, semi-colon is field separator, fixed width = 10
   {  RuntimeConfiguration info("", "", "", "", "", ';' ,10);
      std::ostringstream s;
      Experiment( properties, probes, info).printField(false, s);
      s << "y";
      assert( s.str() == ";         y");
   }
}

void test_Experiment_printTable()
{
   std::vector< boost::shared_ptr<Property> > properties;
   { // case 1: no probes, no entries
      std::vector< DatadrillerProperty > probes;
      RuntimeConfiguration info("", "", "", "", "", ',', 0);

      Experiment experiment(properties, probes, info);
      Experiment::ResultsTable table;

      std::ostringstream s;
      experiment.printTable(table, s);
      assert( s.str() == "X,Y,Z,Age\n");
   }

   { // case 1: one probe, one entry
      std::vector< DatadrillerProperty > probes;
      probes.push_back(DatadrillerProperty("A", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
      RuntimeConfiguration info("", "", "", "", "", ',', 0);

      Experiment experiment(properties, probes, info);
      Experiment::ResultsTable table;
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 5.0));

      std::ostringstream s;
      experiment.printTable(table, s);
      assert( s.str() == "X,Y,Z,Age,A\n0,1,2,3,5\n");
   }

   { // case 2: two probes, four entries on the same space coordinate. two of them can be coalesced. The other two
      // cannot be merged with because the probe names collide
      std::vector< DatadrillerProperty > probes;
      probes.push_back(DatadrillerProperty("A", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
      probes.push_back(DatadrillerProperty("B", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
      RuntimeConfiguration info("", "", "", "", "", ',', 0);

      Experiment experiment(properties, probes, info);
      Experiment::ResultsTable table;
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 5.0));
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 6.0));
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 7.0));
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 8.0));

      std::ostringstream s;
      experiment.printTable(table, s);
      assert( s.str() == "X,Y,Z,Age,A,B\n0,1,2,3,6, \n0,1,2,3,8,5\n0,1,2,3, ,7\n");
   }

   { // case 3: three probes, four entries on the same space coordinate. three of them can be coalesced. The fourth
      // cannot be merged with another, because the position is different
      std::vector< DatadrillerProperty > probes;
      probes.push_back(DatadrillerProperty("A", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
      probes.push_back(DatadrillerProperty("B", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
      probes.push_back(DatadrillerProperty("C", 0.0, 0.0, 0.0, 0.0, 1.0, 0.1));
      RuntimeConfiguration info("", "", "", "", "", ',', 0);

      Experiment experiment(properties, probes, info);
      Experiment::ResultsTable table;
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 5.0));
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 6.0));
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 2.0, 3.0, 3.0), 2, 7.0));
      table.push_back( Experiment::Entry( Experiment::PositionAndTime(0.0, 1.0, 2.0, 3.0), 2, 8.0));

      std::ostringstream s;
      experiment.printTable(table, s);
      assert( s.str() == "X,Y,Z,Age,A,B,C\n0,1,2,3,5,6,8\n0,2,3,3, , ,7\n");
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
   else if (std::strcmp(argv[1], "printField")==0)
      test_Experiment_printField();
   else if (std::strcmp(argv[1], "printTable")==0)
      test_Experiment_printTable();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }

   return 0;
}
