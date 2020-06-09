#include "StatisticsHandler.h"
#include "NumericFunctions.h"
#include "System.h"
#include <sstream>

double Utilities::CheckMemory::StatisticsHandler::s_virtualMemoryUsage = 0.0;
double Utilities::CheckMemory::StatisticsHandler::s_residentMemoryUsage = 0.0;

namespace Utilities
{
	namespace CheckMemory
	{
		void StatisticsHandler::initialise()
		{
			s_virtualMemoryUsage = 0.0;
			s_residentMemoryUsage = 0.0;
		}

		void StatisticsHandler::update() {

#ifdef _MSC_VER // TODO_SK
			s_virtualMemoryUsage = 0;
			s_residentMemoryUsage = 0;
#else
			StatM stat;

			getStatM(stat);
			s_virtualMemoryUsage = NumericFunctions::Maximum(s_virtualMemoryUsage, double(stat.size) * double(getPageSize()));
			s_residentMemoryUsage = NumericFunctions::Maximum(s_residentMemoryUsage, double(stat.resident) * double(getPageSize()));
#endif

		}

		std::string StatisticsHandler::print(const int rank)
		{
			std::stringstream buffer;

			buffer << "  <memory_usage>" << std::endl;
			buffer << "    <rank> " << rank << " </rank>" << std::endl;
			buffer << "    <virtual> " << s_virtualMemoryUsage << " </virtual>" << std::endl;
			buffer << "    <resident> " << s_residentMemoryUsage << " </resident>" << std::endl;
			buffer << "  </memory_usage>" << std::endl;

			return buffer.str();
		}
	}
}
