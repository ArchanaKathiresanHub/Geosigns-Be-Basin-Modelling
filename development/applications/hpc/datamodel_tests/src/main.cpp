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
#include "ImportProjectHandle.h"
#include "ImportExport.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include <ctime>
#include <cstring>

/// \brief Small wrapper application for the VisualizationIO libraries
int main(int argc, char ** argv)
{
    if (argc <= 1)
    {
        cout << "Usage: datamodel_test.exe [ -import-native | -import-projectHandle <projectHandle> | -convert <projectHandle> ] " << endl
             << " -import-native       : loads cauldron_outputs.xml from the current directory, reads all the data into memory" << endl
             << " -import-projectHandle: loads the specified projectHandle into memory" << endl
             << " -convert             : converts the specified projectHandle to new native format" << endl;
        return 1;
    }

    string mode = argv[1];

    if (mode == "-import-native")
    {
        clock_t start = clock();
        float timeInSeconds;

        boost::shared_ptr<CauldronIO::Project> project = CauldronIO::ImportExport::importFromXML(CauldronIO::ImportExport::getXMLIndexingFileName("."));

        timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
        cout << "Finished import in " << timeInSeconds << " seconds " << endl;

        // Retrieve data
        cout << "Retrieving data" << endl;
        start = clock();
        project->retrieve();
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
        boost::shared_ptr<DataAccess::Interface::ObjectFactory> factory(new DataAccess::Interface::ObjectFactory());
        boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle(DataAccess::Interface::OpenCauldronProject(projectFileName, "r", factory.get()));
        if (!projectHandle)
        {
            cerr << "Could not open the project handle" << endl;
            return 0;
        }
        timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
        cout << "Finished opening project handle in " << timeInSeconds << " seconds " << endl;

        // Import from ProjectHandle
        cout << "Importing from project handle (requires reading depth formations)" << endl;
        boost::shared_ptr<CauldronIO::Project> project = ImportProjectHandle::createFromProjectHandle(projectHandle, false);
        timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
        cout << "Finished import in " << timeInSeconds << " seconds " << endl;

        if (mode == "-import-projectHandle")
        {
            // Retrieve data
            cout << "Retrieving data" << endl;
            start = clock();
            project->retrieve();
            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
            cout << "Finished retrieve in " << timeInSeconds << " seconds " << endl;
        }
        else
        {
            // Export to native format: it will retrieve data when needed
            cout << "Writing to new format" << endl;
            start = clock();
            CauldronIO::ImportExport::exportToXML(project, "output-dir");
            timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
            cout << "Wrote to new format in " << timeInSeconds << " seconds" << endl;
        }

        return 0;
    }

    cerr << "Unknown command line parameter. Exiting..." << endl;
    return 1;
}

