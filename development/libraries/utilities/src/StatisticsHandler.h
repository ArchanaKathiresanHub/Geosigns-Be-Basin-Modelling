#ifndef _STATISTICS_HANDLER__H_
#define _STATISTICS_HANDLER__H_

#include <string>

/// \brief Handles all statistics that are to be output at the need of a run.
namespace Utilities
{
	namespace CheckMemory
	{
		class StatisticsHandler
		{

		public:

			/// \brief Initialise the internal state of the handler.
			static void initialise();

			/// \brief Update the state of the handler with the current values that are being collected.
			static void update();

			/// \brief Print the current state.
			///
			/// Will be output in xml-type format.
			static std::string print(const int rank);

		private:

			static double s_virtualMemoryUsage;
			static double s_residentMemoryUsage;

		};
	}
}


#endif // _STATISTICS_HANDLER__H_
