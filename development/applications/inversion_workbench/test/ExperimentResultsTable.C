#include "ExperimentResultsTable.h"

#include <gtest/gtest.h>


TEST( ExperimentResultsTable, noProbesNoEntries)
{ // case 1: no probes, no entries
   std::vector< std::string > noProbes;
   std::ostringstream s;
   ExperimentResultsTable( noProbes, ',', 0).print(s);
   EXPECT_EQ( "X,Y,Z,Age\n",  s.str() );
}

TEST( ExperimentResultsTable, oneProbeOneEntry )
{ // case 1: one probe, one entry
   std::vector< std::string > probes;
   probes.push_back("A");

   ExperimentResultsTable table( probes, ',', 0);
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 5.0));

   std::ostringstream s;
   table.print(s);
   EXPECT_EQ( "X,Y,Z,Age,A\n0,1,2,3,5\n",  s.str() );
}

TEST( ExperimentResultsTable, twoProbesFourEntries)
{ // case 2: two probes, four entries on the same space coordinate. two of them can be coalesced. The other two
   // cannot be merged with because the probe names collide
   std::vector< std::string > probes;
   probes.push_back("A");
   probes.push_back("B");

   ExperimentResultsTable table( probes, ',', 0);
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 5.0));
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 6.0));
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 7.0));
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 8.0));

   std::ostringstream s;
   table.print( s);
   EXPECT_EQ( "X,Y,Z,Age,A,B\n0,1,2,3,6, \n0,1,2,3,8,5\n0,1,2,3, ,7\n",  s.str() );
}

TEST( ExperimentResultsTable, threeProbesFourEntries)
{ // case 3: three probes, four entries on the same space coordinate. three of them can be coalesced. The fourth
   // cannot be merged with another, because the position is different
   std::vector< std::string > probes;
   probes.push_back("A");
   probes.push_back("B");
   probes.push_back("C");

   ExperimentResultsTable table( probes, ',', 0);
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 0, 5.0));
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 1, 6.0));
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 2.0, 3.0, 3.0), 2, 7.0));
   table.add( ExperimentResultsTable::Entry( ExperimentResultsTable::PositionAndTime(0.0, 1.0, 2.0, 3.0), 2, 8.0));

   std::ostringstream s;
   table.print(s);
   EXPECT_EQ( "X,Y,Z,Age,A,B,C\n0,1,2,3,5,6,8\n0,2,3,3, , ,7\n",  s.str() );
}


