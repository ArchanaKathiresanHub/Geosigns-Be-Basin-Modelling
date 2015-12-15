#ifndef TOUCHSTONE_H
#define TOUCHSTONE_H

#include "tslibI.h"
#include "TsLibLoader.h"
#include <geocosmexception.h>
#include <iostream>

#include <vector>
#include <map>
#include <string>
#include <sys/time.h>
#include <sys/resource.h>

// Utilities lib
#include "formattingexception.h"

using namespace Geocosm;

class TouchstoneFiles;

class TouchstoneWrapper
{

 public:

   TouchstoneWrapper( const char * burhistFile, char * filename, const char * results,  const char * status, const char * rank, int verboseLevel );

   ~TouchstoneWrapper( );

   bool loadTcf ( );

   void calculateWrite ( );
      
   struct Exception : formattingexception::BaseException< Exception > {}; 
   
   void message( const std::string & msg, int level = 0 )
   {  
      if ( level >= m_verboseLevel )
      {
         (level > 0 ? std::cout : std::cerr) << "MeSsAgE " << (level > 0 ? "warning " : "error " ) << msg << std::endl;
      }
   }

 private:
   int SaveResultHeader (TcfSchema::DetailHeadersType::modalHeaders_iterator& itor); 

   void setCategoriesMappingStatistics( );
      
   void writeTouchstoneResults(int timestepIndex, TouchstoneFiles& TouchstoneFiles);


   /** files to be loaded */

   const char *		m_burhistFile;

   char *		m_filename;

   const char *		m_results;
   
   const char *		m_status;
   
   const char *		m_rank;

   /** Touchstone Library Interface object.*/ 
   Geocosm::TsLib::TsLibInterface * m_tslib;

   /** Touchstone configuration file object.*/
   Geocosm::TsLib::TcfInfoInterface* m_tcfInfo;

   /** Touchstone calculation context. */
   Geocosm::TsLib::CalcContextInterface* m_tslibCalcContext;

   /** Touchstone BurialHistory info. */  
   Geocosm::TsLib::TsLibBurialHistory m_tslibBurialHistoryInfo;

   bool m_directAnalogRun;

   int m_nrOfRealizations;

   std::map <int, int> m_categoriesMappingOrder;

   std::vector < TcfSchema::ResultHeaderIdentificationType > m_iD;  

   static const int numberOfTouchstoneProperties = 7;
   static const int numberOfStatisticalOutputs   = 30;
   
   struct rlimit m_coreSize;
   
   int m_verboseLevel;
   
   enum 
   {
      stdev,
      mean,
      geomean,
      skewness,
      kurtosis,
      minimum,
      maximum,
      mode,
      P1,
      P5,
      P10,
      P15,
      P20,
      P25,
      P30,
      P35,
      P40,
      P45,
      P50,
      P55,
      P60,
      P65,
      P70,
      P75,
      P80,
      P85,
      P90,
      P95,
      P99,
      statSize
   };

   std::vector<TcfSchema::StatisticsType::value> m_statsVect;         
};

#endif



