#include "ExperimentResultsTablePrinter.h"

#include "RuntimeConfiguration.h"

#include <iostream>
#include <iomanip>

ExperimentResultsTablePrinter :: ExperimentResultsTablePrinter( char fieldSeparator, int fieldWidth )
   : m_fieldSeparator( fieldSeparator )
   , m_fixedWidth( fieldWidth )
{}

void ExperimentResultsTablePrinter :: newField( bool first, std::ostream & output) const
{
   if (!first)
      output << m_fieldSeparator;

   if (m_fixedWidth > 0)
   {
      output << std::setw( m_fixedWidth );

      if (m_fixedWidth > 3)
         output << std::setprecision(m_fixedWidth - 2);
   }
}

void ExperimentResultsTablePrinter :: newRecord( std::ostream & output) const
{
   output << '\n';
}

