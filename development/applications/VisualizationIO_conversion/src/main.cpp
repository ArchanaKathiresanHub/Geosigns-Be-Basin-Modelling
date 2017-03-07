//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "VisualizationAPI.h"
#include "ImportFromXML.h"
#include "ExportToXML.h"
#include "ImportProjectHandle.h"
#include "VisualizationIO_native.h"
#include "VisualizationUtils.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "FilePath.h"

#include <boost/filesystem/path.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>

#include <ctime>
#include <cstring>

/// \brief method to retrieve data on a separate thread
void retrieveDataQueue(std::vector < CauldronIO::VisualizationIOData* >* allData, boost::lockfree::queue<int>* queue, boost::atomic<bool>* done)
{
	int value;
	while (!*done) {
		while (queue->pop(value))
		{
			CauldronIO::VisualizationIOData* data = allData->at(value);
			assert(!data->isRetrieved());
			data->retrieve();
		}
	}

	while (queue->pop(value))
	{
		CauldronIO::VisualizationIOData* data = allData->at(value);
		assert(!data->isRetrieved());
		data->retrieve();
	}
}

/// \brief Small wrapper application for the VisualizationIO libraries
int main(int argc, char ** argv)
{
	if (argc <= 1)
	{
		std::cout << "Usage: VisualizationIO_convert [mode] [options] " << endl
			<< " Modes: " << endl
			<< "  -import-native <xml-file>               : loads xml reads all the data into memory" << endl
			<< "  -import-projectHandle <project3D file>  : loads the specified project3D file into memory" << endl
			<< "  -convert <project3D file>               : converts the specified project3D file to new native format, " << endl
			<< " Options: " << endl
			<< "  -threads=x                              : use x threads for compression during export or parallel importing" << endl
			<< "  -center                                 : cell-center all properties except depth" << endl
			<< "  -extend <xml-file>                      : if data is existing in the given xml-file, that data will not be converted but referred to" << endl
			<< "  -verbose                                : output debugging information" << endl
			<< "  -outputDir <directory>                  : output to this directory instead of input directory" << endl;

		return 1;
	}

	string mode = argv[1];

	// Check options
	int numThreads = 1;
	bool center = false;
	bool verbose = false;
	std::string extendXMLfile;
	std::string outputDirStr;

	for (int i = 3; i < argc; i++)
	{
		if (std::string(argv[i]).find("threads") != std::string::npos)
		{
			numThreads = std::atoi(argv[i] + 9);
			numThreads = min(24, (int)max(1, (int)numThreads));
			cout << "Using " << numThreads << " threads" << endl;
		}
		else if (std::string(argv[i]).find("center") != std::string::npos)
		{
			center = true;
			cout << "Applying cell-centering to converted maps and volumes" << endl;
		}
		else if (std::string(argv[i]).find("verbose") != std::string::npos)
		{
			verbose = true;
			cout << "Running in verbose mode" << endl;
		}
		else if (std::string(argv[i]).find("extend") != std::string::npos)
		{
			if (i == argc - 1)
			{
				cerr << "Missing xml-file location" << endl;
				exit(1);
			}
			else
			{
				extendXMLfile.assign(argv[i + 1]);
				i++;

				if (!boost::filesystem::exists(boost::filesystem::path(extendXMLfile)))
				{
					cerr << "extending with xml-file " << extendXMLfile << " not possible since it can't be found" << endl;
					exit(1);
				}
				cout << "Extending xml-project " << extendXMLfile << endl;
			}
		}
		else if (std::string(argv[i]).find("outputDir") != std::string::npos)
		{
			if (i == argc - 1)
			{
				cerr << "Missing output directory location" << endl;
				exit(1);
			}
			else
			{
				outputDirStr.assign(argv[i + 1]);
				i++;
				cout << "Using as output directory: " << outputDirStr << endl;
			}
		}
		else
		{
			std::cerr << "Unrecognized option: " << argv[i] << std::endl;
			exit(1);
		}
	}

    try
    {
        if (mode == "-import-native")
        {
            if (argc < 3)
            {
                cerr << "Please specify xml indexing file name" << endl;
                return 1;
            }

            string xmlName = argv[2];
            clock_t start = clock();
            float timeInSeconds;

            std::cout << "Starting import from XML" << endl;
            shared_ptr<CauldronIO::Project> project = CauldronIO::ImportFromXML::importFromXML(xmlName);

            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished import in " << timeInSeconds << " seconds " << endl;

            // Retrieve data
			std::cout << "Retrieving data" << endl;
            start = clock();

			std::cout << "Retrieving input data" << endl;
			project->retrieveStratigraphyTable();

            for (shared_ptr<CauldronIO::SnapShot> snapShot : project->getSnapShots())
            {
                std::vector < CauldronIO::VisualizationIOData* > allReadData = snapShot->getAllRetrievableData();

                std::cout << "Retrieving snapshot " << snapShot->getAge() << " with " << numThreads << " threads" << endl;

                boost::lockfree::queue<int> queue(128);
                boost::atomic<bool> done(false);

                // Retrieve in separate threads
                boost::thread_group threads;
                for (int i = 0; i < numThreads - 1; ++i)
                    threads.add_thread(new boost::thread(retrieveDataQueue, &allReadData, &queue, &done));

                // Load the data on the main thread
                for (int i = 0; i < allReadData.size(); i++)
                {
                    allReadData[i]->prefetch();
                    queue.push(i);
                }
                done = true;

                // Single threaded: retrieve now
                if (numThreads == 1)
                    retrieveDataQueue(&allReadData, &queue, &done);
                threads.join_all();

                snapShot->release();
            }

            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished retrieve in " << timeInSeconds << " seconds " << endl;

            return 0;
        }
        else if (mode == "-import-projectHandle" || mode == "-convert")
        {
            if (argc < 3)
            {
                cerr << "Please specify project3D filename" << endl;
                return 1;
            }

            string projectFileName = argv[2];
            clock_t start = clock();
            float timeInSeconds;

            // Open the projectHandle
			std::cout << "Opening project3D file " << endl;
            shared_ptr<DataAccess::Interface::ObjectFactory> factory(new DataAccess::Interface::ObjectFactory());
            shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle(DataAccess::Interface::OpenCauldronProject(projectFileName, "r", factory.get()));
            if (!projectHandle)
            {
                cerr << "Could not open the project3D file" << endl;
                return 0;
            }
            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished opening project3D file in " << timeInSeconds << " seconds " << endl;
			start = clock();

            // Import from ProjectHandle
			std::cout << "Importing from project handle (requires reading depth formations)" << endl;
			shared_ptr<CauldronIO::Project> project = ImportProjectHandle::createFromProjectHandle(projectHandle, verbose);
            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished import in " << timeInSeconds << " seconds " << endl;

            if (mode == "-import-projectHandle")
            {
                // Retrieve data
				std::cout << "Retrieving data" << endl;
                start = clock();

                project->retrieveStratigraphyTable();

                for (const std::shared_ptr<CauldronIO::SnapShot>& snapShot : project->getSnapShots())
                {
					std::vector < CauldronIO::VisualizationIOData* > data = snapShot->getAllRetrievableData();
					CauldronIO::VisualizationUtils::retrieveAllData(data, numThreads);
					snapShot->release();
                }

                timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
				std::cout << "Finished retrieve in " << timeInSeconds << " seconds " << endl;
            }
            else // mode is convert
            {
				// Load existing XML project if needed
				shared_ptr<CauldronIO::Project> projectExisting;
				if (!extendXMLfile.empty())
				{
					std::cout << "Reading existing XML project" << endl;
					start = clock();
					projectExisting = CauldronIO::ImportFromXML::importFromXML(extendXMLfile);

					timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
					std::cout << "Finished import in " << timeInSeconds << " seconds " << endl;

					// Projects should match
					if (!(*project == *projectExisting))
					{
						std::cerr << "Cannot extend project since projects are not matching" << std::endl;
						exit(1);
					}
				}
				
				// Export to native format: it will retrieve data when needed
				std::cout << "Writing to new format" << endl;
                start = clock();

                // Check for explicit output path
                ibs::FilePath absPath(projectFileName);
				if (!outputDirStr.empty())
                {
					absPath = ibs::FilePath(outputDirStr) << absPath.fileName();
                }

                CauldronIO::ExportToXML::exportToXML(project, projectExisting, absPath.path(), numThreads, center);
                timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
				std::cout << "Wrote to new format in " << timeInSeconds << " seconds" << endl;
            }

            return 0;
        }

        cerr << "Unknown command line parameter. Exiting..." << endl;
        return 1;
    }
    catch (CauldronIO::CauldronIOException& except)
    {
        cerr << "Error occurred: " << except.what() << endl;
    }

    return 1;
}

