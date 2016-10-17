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
#include "ImportExport.h"
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
		cout << "Usage: VisualizationIO_convert [mode] [options] " << endl
			<< "  Modes: "
			<< "  -import-native <xml-file>              : loads xml reads all the data into memory" << endl
			<< "  -import-projectHandle <projectHandle>  : loads the specified projectHandle into memory" << endl
			<< "  -convert <projectHandle> [<directory>] : converts the specified projectHandle to new native format, " << endl
			<< "                                           output to directory if specified, otherwise same as input directory" << endl
			<< " Options: " << endl
			<< " -threads=x                              : use x threads for compression during export or parallel importing" << endl
			<< " -center                                 : cell-center all properties except depth" << endl
			<< " -verbose								 : output debugging information" << endl;
        return 1;
    }

    string mode = argv[1];

    // Check options
    int numThreads = 1;
    bool center = false;
	bool verbose = false;
    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]).find("threads") != std::string::npos)
        {
            numThreads = std::atoi(argv[i] + 9);
            numThreads = min(24, (int)max(1, (int)numThreads));
        }
		if (std::string(argv[i]).find("center") != std::string::npos)
		{
			center = true;
		}
		if (std::string(argv[i]).find("verbose") != std::string::npos)
		{
			verbose = true;
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

            cout << "Starting import from XML" << endl;
            shared_ptr<CauldronIO::Project> project = CauldronIO::ImportExport::importFromXML(xmlName);

            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
            cout << "Finished import in " << timeInSeconds << " seconds " << endl;

            // Retrieve data
            cout << "Retrieving data" << endl;
            start = clock();

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
            cout << "Finished retrieve in " << timeInSeconds << " seconds " << endl;

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

            // Try to open the projectHandle
            cout << "Opening project handle " << endl;
            shared_ptr<DataAccess::Interface::ObjectFactory> factory(new DataAccess::Interface::ObjectFactory());
            shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle(DataAccess::Interface::OpenCauldronProject(projectFileName, "r", factory.get()));
            if (!projectHandle)
            {
                cerr << "Could not open the project handle" << endl;
                return 0;
            }
            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
            cout << "Finished opening project handle in " << timeInSeconds << " seconds " << endl;

            shared_ptr<CauldronIO::Project> project;
            
            // Import from ProjectHandle
            cout << "Importing from project handle (requires reading depth formations)" << endl;
            project = ImportProjectHandle::createFromProjectHandle(projectHandle, verbose);
            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
            cout << "Finished import in " << timeInSeconds << " seconds " << endl;

            if (mode == "-import-projectHandle")
            {
                // Retrieve data
                cout << "Retrieving data" << endl;
                start = clock();

                for (const std::shared_ptr<CauldronIO::SnapShot>& snapShot : project->getSnapShots())
                {
                    CauldronIO::VisualizationUtils::retrieveAllData(snapShot, numThreads);
                    snapShot->release();
                }

                timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
                cout << "Finished retrieve in " << timeInSeconds << " seconds " << endl;
            }
            else // mode is convert
            {
                // Export to native format: it will retrieve data when needed
                cout << "Writing to new format" << endl;
                start = clock();

                // Check for explicit output path (should be argument 3)
                ibs::FilePath absPath(projectFileName);
                if (argc > 3)
                {
                    std::string argOutput(argv[3]);
                    // Don't confuse with options
                    if (argOutput.find("threads") == std::string::npos && argOutput.find("center") == std::string::npos)
                    {
                        absPath = ibs::FilePath(argOutput) << absPath.fileName();
                    }
                }

                CauldronIO::ImportExport::exportToXML(project, absPath.path(), numThreads, center);
                timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
                cout << "Wrote to new format in " << timeInSeconds << " seconds" << endl;
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

