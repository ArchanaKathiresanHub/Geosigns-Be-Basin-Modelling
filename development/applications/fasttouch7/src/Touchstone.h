#ifndef TOUCHSTONE_H
#define TOUCHSTONE_H

#include "tslibI.h"
#include "TsLibLoader.h"
#include <geocosmexception.h>

#include <vector>
#include <map>
#include <string>
#include <sys/time.h>
#include <sys/resource.h>

using namespace Geocosm;

class TouchstoneFiles;

class TouchstoneWrapper
{

public:

   TouchstoneWrapper( const char * burhistFile, char * filename, const char * results,  const char * status);
   ~TouchstoneWrapper( );

   bool loadTcf ( );
   void calculateWrite ( );

private:
   int SaveResultHeader (TcfSchema::DetailHeadersType::modalHeaders_iterator& itor); 

   void setCategoriesMapping( );
   void write(int timestepIndex, TouchstoneFiles& TouchstoneFiles);


   /** files to be loaded */

   const char *		m_burhistFile;

   char *		m_filename;

   const char *		m_results;
   
   const char *		m_status;

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
   static const int numberOfStatisticalOutputs   = 29;
   
   struct rlimit m_coreSize;
};

#endif



