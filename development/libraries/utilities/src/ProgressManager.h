#ifndef __Utilities_ProgressManager_HH__
#define __Utilities_ProgressManager_HH__

#include "WallTime.h"
#include <string>

namespace utilities
{
   /// Class to report progress message with elapsed time
   class ProgressManager
   {
      public:
         ProgressManager();
	 virtual ~ProgressManager();

	 /// Start the clock
	 void start();
	 /// report progress message with elapsed time
	 std::string report (const std::string & message);
      private:
	 WallTime::Time m_timeStarted;
   };
}
#endif
