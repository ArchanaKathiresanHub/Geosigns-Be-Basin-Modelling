#include <stdlib.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <string.h>
#include <string>
#include <iostream>

using namespace std;

#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)

#include "PersistentTrapFactory.h"

#include "TrackProjectHandle.h"
#include "LogHandler.h"

using namespace PersistentTraps;

static char* argv0 = 0;
static void showUsage(char* message = 0);

bool verbose = false;

int main(int argc, char** argv)
{
	string projectFileName;
	string outputFileName;
	string directoryExtension;

	// the root object
	if ((argv0 = strrchr(argv[0], '/')) != 0)
	{
		++argv0;
	}
	else
	{
		argv0 = argv[0];
	}

	for (int i = 1; i < argc; i++)
	{
		if (strncmp(argv[i], "-project", Max(2, strlen(argv[i]))) == 0 && projectFileName.size() == 0)
		{
			if (i + 1 >= argc)
			{
				showUsage("Argument for '-project' is missing");
				return -1;
			}
			projectFileName = argv[++i];
		}
		else if (strncmp(argv[i], "-output", Max(2, strlen(argv[i]))) == 0)
		{
			if (i + 1 >= argc)
			{
				showUsage("Argument for '-output' is missing");
				return -1;
			}
			outputFileName = argv[++i];
		}
		else if (strncmp(argv[i], "-verbose", Max(2, strlen(argv[i]))) == 0)
		{
			verbose = true;
		}
		else if (strncmp(argv[i], "-", Max(1, strlen(argv[i]))) != 0 && projectFileName.size() == 0)
		{
			projectFileName = argv[i];
		}
		else
		{
			showUsage();
			return -1;
		}
	}

	if (projectFileName.size() == 0)
	{
		showUsage("No project file specified");
		return -1;
	}

	if (outputFileName.size() == 0)
	{
		outputFileName = projectFileName;
	}

	if (verbose) {
		LogHandler("peristentTraps", LogHandler::DIAGNOSTIC_LEVEL);
	}
	else {
		LogHandler("peristentTraps", LogHandler::DETAILED_LEVEL);
	}

	PersistentTrapFactory factory;
	std::unique_ptr<TrackProjectHandle> projectHandle(dynamic_cast<TrackProjectHandle*>(Interface::OpenCauldronProject(projectFileName, &factory)));

	if (!projectHandle)
	{
		LogHandler(LogHandler::FATAL_SEVERITY) << "Failed to read project file " << projectFileName;
		return -1;
	}

	if (verbose)
	{
		LogHandler(LogHandler::INFO_SEVERITY) << "Finished reading project " << projectFileName;
	}

	if (projectHandle->createPersistentTraps())
	{
		projectHandle->saveProject(outputFileName);
	}

	return 0;
}

void showUsage(char* message)
{
	cout << endl;
	if (message)
	{
		cout << argv0 << ": " << message << endl;
	}

	cout << "Usage: " << argv0
		<< " [-project] projectfile <-output filename> [-verbose]"
		<< endl;
	exit(-1);
}
