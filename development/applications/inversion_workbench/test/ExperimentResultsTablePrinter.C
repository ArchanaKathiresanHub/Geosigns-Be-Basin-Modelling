#include "ExperimentResultsTablePrinter.h"
#include "RuntimeConfiguration.h"

#include <gtest/gtest.h>


// case 1: first field = true, comma is field separator, no fixed width
TEST( ExperimentResultsTablePrinterPrintField, SeparatorIsCommaVariableWidth )
{  
   std::ostringstream s;
   ExperimentResultsTablePrinter(',').newField(true, s);
   s << "x";
   EXPECT_EQ( "x",  s.str() );
}

// case 2: first field = false, semi-colon is field separator, fixed width = 10
TEST( ExperimentResultsTablePrinterPrintField, SeparatorIsSemicolonFixedWidth )
{  
   std::ostringstream s;
   ExperimentResultsTablePrinter(';', 10).newField(false, s);
   s << "y";
   EXPECT_EQ( ";         y",  s.str() );
}

TEST( ExperimentResultsTablePrinterNewRecord, normal)
{
   std::ostringstream s;
   ExperimentResultsTablePrinter(',', 5).newRecord(s);
   EXPECT_EQ( "\n",  s.str() );
}
