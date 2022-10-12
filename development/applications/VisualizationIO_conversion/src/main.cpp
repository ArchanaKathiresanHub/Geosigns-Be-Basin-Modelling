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
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "FilePath.h"

#include <boost/filesystem/path.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>

#include <ctime>
#include <cstring>

/// \brief method to retrieve data on a separate thread
void retrieveDataQueue(std::vector < CauldronIO::VisualizationIOData* >* allData, boost::lockfree::queue<int>* queue,
		boost::atomic<bool>* done, std::shared_ptr<CauldronIO::Project>& project)
{
	int value;
	while (!*done) {
		while (queue->pop(value))
		{
			CauldronIO::VisualizationIOData* data = allData->at(value);
			assert(!data->isRetrieved());
			try
			{
								data->retrieve();
						}
			catch (CauldronIO::CauldronIOException&)
			{
								// Find out what data did not load
								std::cout << "Basin_Error: Data " << value << " failed to load: ";
								CauldronIO::VisualizationUtils::findAndOutputData(data, project);
			}
		}
	}

	while (queue->pop(value))
	{
		CauldronIO::VisualizationIOData* data = allData->at(value);
		assert(!data->isRetrieved());

        try
        {
            data->retrieve();
        }
        catch (CauldronIO::CauldronIOException&)
        {
            // Find out what data did not load
            std::cout << "Basin_Error: Data " << value << " failed to load: ";
            CauldronIO::VisualizationUtils::findAndOutputData(data, project);
        }
    }
}

/// \brief Small wrapper application for the VisualizationIO libraries
int main(int argc, char ** argv)
{
	if (argc <= 1)
	{
		std::cout << "Usage: VisualizationIO_convert [mode] [options] " << std::endl
			<< " Modes: " << std::endl
			<< "  -import-native <xml-file>               : loads xml reads all the data into memory" << std::endl
			<< "  -import-projectHandle <project3D file>  : loads the specified project3D file into memory" << std::endl
			<< "  -convert <project3D file>               : converts the specified project3D file to new native format, " << std::endl
			<< " Options: " << std::endl
			<< "  -threads=x                              : use x threads for compression during export or parallel importing" << std::endl
			<< "  -center                                 : cell-center all properties except depth" << std::endl
			<< "  -extend <xml-file>                      : if data is existing in the given xml-file, that data will not be converted but referred to" << std::endl
			<< "  -verbose                                : output debugging information" << std::endl
			<< "  -outputDir <directory>                  : output to this directory instead of input directory" << std::endl;

		return -1;
	}

	std::string mode = argv[1];

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
			numThreads = std::min(24, (int)std::max(1, (int)numThreads));
			std::cout << "Using " << numThreads << " threads" << std::endl;
		}
		else if (std::string(argv[i]).find("center") != std::string::npos)
		{
			center = true;
			std::cout << "Applying cell-centering to converted maps and volumes" << std::endl;
		}
		else if (std::string(argv[i]).find("verbose") != std::string::npos)
		{
			verbose = true;
			std::cout << "Running in verbose mode" << std::endl;
		}
		else if (std::string(argv[i]).find("extend") != std::string::npos)
		{
			if (i == argc - 1)
			{
				std::cerr << "Missing xml-file location" << std::endl;
				return -1;
			}
			else
			{
				extendXMLfile.assign(argv[i + 1]);
				i++;

				if (!boost::filesystem::exists(boost::filesystem::path(extendXMLfile)))
				{
					std::cerr << "extending with xml-file " << extendXMLfile << " not possible since it can't be found" << std::endl;
					return -1;
				}
				std::cout << "Extending xml-project " << extendXMLfile << std::endl;
			}
		}
		else if (std::string(argv[i]).find("outputDir") != std::string::npos)
		{
			if (i == argc - 1)
			{
				std::cerr << "Missing output directory location" << std::endl;
				return -1;
			}
			else
			{
				outputDirStr.assign(argv[i + 1]);
				i++;
				std::cout << "Using as output directory: " << outputDirStr << std::endl;
			}
		}
		else
		{
			std::cerr << "Unrecognized option: " << argv[i] << std::endl;
			return -1;
		}
	}

    try
    {
        if (mode == "-import-native")
        {
            if (argc < 3)
            {
                std::cerr << "Please specify xml indexing file name" << std::endl;
                return -1;
            }

            std::string xmlName = argv[2];
            clock_t start = clock();
            float timeInSeconds;

            std::cout << "Starting import from XML" << std::endl;
            std::shared_ptr<CauldronIO::Project> project = CauldronIO::ImportFromXML::importFromXML(xmlName);

						timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished import in " << timeInSeconds << " seconds " << std::endl;

						// Retrieve data
			std::cout << "Retrieving data" << std::endl;
						start = clock();

			std::cout << "Retrieving input data" << std::endl;
			project->retrieveStratigraphyTable();

            for (std::shared_ptr<CauldronIO::SnapShot> snapShot : project->getSnapShots())
            {
                std::vector < CauldronIO::VisualizationIOData* > allReadData = snapShot->getAllRetrievableData();

                std::cout << "Retrieving snapshot " << snapShot->getAge() << " with " << numThreads << " threads" << std::endl;
                std::cout << "Nr. of data blocks to process: " << allReadData.size() << std::endl;

                if (numThreads == 1)
                {
                    snapShot->retrieve();
                }
                else
                {
                    boost::lockfree::queue<int> queue(allReadData.size());
                    boost::atomic<bool> done(false);

                    // Retrieve in separate threads
                    boost::thread_group threads;
                    for (int i = 0; i < numThreads - 1; ++i)
                        threads.add_thread(new boost::thread(retrieveDataQueue, &allReadData, &queue, &done, project));

                    // Load the data on the main thread
                    for (int i = 0; i < allReadData.size(); i++)
                    {
                        allReadData[i]->prefetch();
                        queue.push(i);
                    }
                    done = true;
                    threads.join_all();
                }

                snapShot->release();
            }

						timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished retrieve in " << timeInSeconds << " seconds " << std::endl;

            return 0;
        }
        else if (mode == "-import-projectHandle" || mode == "-convert")
        {
            if (argc < 3)
            {
                std::cerr << "Please specify project3D filename" << std::endl;
                return -1;
            }

            std::string projectFileName = argv[2];
            clock_t start = clock();
            float timeInSeconds;

						// Open the projectHandle
			std::cout << "Opening project3D file " << std::endl;
			std::shared_ptr<DataAccess::Interface::ObjectFactory> factory(new DataAccess::Interface::ObjectFactory());
			std::unique_ptr<DataAccess::Interface::ProjectHandle> projectHandle(DataAccess::Interface::OpenCauldronProject(projectFileName, factory.get()));
						if (!projectHandle)
						{
							std::cerr << "Could not open the project3D file" << std::endl;
								return -1;
						}
						timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished opening project3D file in " << timeInSeconds << " seconds " << std::endl;
			start = clock();

						// Import from ProjectHandle
			std::cout << "Importing from project handle (requires reading depth formations)" << std::endl;
			std::shared_ptr<CauldronIO::Project> project = ImportProjectHandle::createFromProjectHandle(*projectHandle, verbose);
						timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
			std::cout << "Finished import in " << timeInSeconds << " seconds " << std::endl;

            if (mode == "-import-projectHandle")
            {
                // Retrieve data
        std::cout << "Retrieving data" << std::endl;
                start = clock();

                project->retrieveStratigraphyTable();

                for (const std::shared_ptr<CauldronIO::SnapShot>& snapShot : project->getSnapShots())
                {
          std::vector < CauldronIO::VisualizationIOData* > data = snapShot->getAllRetrievableData();
          CauldronIO::VisualizationUtils::retrieveAllData(data, numThreads);
          snapShot->release();
                }

								timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
				std::cout << "Finished retrieve in " << timeInSeconds << " seconds " << std::endl;
						}
						else // mode is convert
						{
				// Load existing XML project if needed
				std::shared_ptr<CauldronIO::Project> projectExisting;
				if (!extendXMLfile.empty())
				{
					std::cout << "Reading existing XML project" << std::endl;
					start = clock();
					projectExisting = CauldronIO::ImportFromXML::importFromXML(extendXMLfile);

					timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
					std::cout << "Finished import in " << timeInSeconds << " seconds " << std::endl;

					// Projects should match
					if (!(*project == *projectExisting))
					{
						std::cerr << "Cannot extend project since projects are not matching" << std::endl;
						return -1;
					}
				}

				// Export to native format: it will retrieve data when needed
				std::cout << "Writing to new format" << std::endl;
								start = clock();

                // Check for explicit output path
                ibs::FilePath absPath(projectFileName);
        if (!outputDirStr.empty())
                {
          absPath = ibs::FilePath(outputDirStr) << absPath.fileName();
                }

                CauldronIO::ExportToXML::exportToXML(project, projectExisting, absPath.path(), numThreads, center);
                timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
        std::cout << "Wrote to new format in " << timeInSeconds << " seconds" << std::endl;
            }

            return 0;
        }

        std::cerr << "Unknown command line parameter. Exiting..." << std::endl;
        return -1;
    }
    catch (CauldronIO::CauldronIOException& except)
    {
        std::cerr << "Error occurred: " << except.what() << std::endl;
    }
    catch ( ... )
    {
       std::cerr << "Unknown exception happend" << std::endl;
    }

    return -1;
}

