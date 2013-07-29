#ifndef INVERSION_EXPERIMENTRESULTSTABLEPRINTER_H
#define INVERSION_EXPERIMENTRESULTSTABLEPRINTER_H

#include <iosfwd>

class RuntimeConfiguration;

class ExperimentResultsTablePrinter
{
public:
   explicit ExperimentResultsTablePrinter( char fieldSeparator, int fieldWidth = 0 );

   void newField( bool first, std::ostream & output) const;
   void newRecord( std::ostream & output) const;

private:
   char m_fieldSeparator;
   int m_fixedWidth;
};

#endif
