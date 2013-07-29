#ifndef INVERSION_EXPERIMENTRESULTSTABLE_H
#define INVERSION_EXPERIMENTRESULTSTABLE_H

#include "ExperimentResultsTablePrinter.h"

#include <vector>
#include <utility>
#include <string>
#include <tr1/tuple>

class ExperimentResultsTable
{
public:
   typedef std::tr1::tuple< double, double, double, double > PositionAndTime;
   typedef unsigned ProbeID;
   typedef std::tr1::tuple< PositionAndTime, ProbeID, double > Entry;

   ExperimentResultsTable(const std::vector< std::string > & probeNames, char fieldSeparator, int fieldWidth);

   void clear();
   void add( const Entry & );
   void print( std::ostream & output ) ;

private:
   std::vector< std::string > m_probeNames;
   std::vector< Entry > m_table;
   ExperimentResultsTablePrinter m_printer;
};

#endif

