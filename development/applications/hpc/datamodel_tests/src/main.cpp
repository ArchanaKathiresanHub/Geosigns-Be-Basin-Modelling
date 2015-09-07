#include "VisualizationAPI.h"
#include "ImportProjectHandle.h"
#include "ImportExport.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include <ctime>
#include <cstring>

int main(int argc, char ** argv)
{
    if (argc <= 1)
    {
        cerr << "Please specify project3D file" << endl;
        return 1;
    }

    string projectFileName = argv[1];
    bool verbose = true;
    if (argc == 3 && std::strcmp(argv[2],"non-verbose") == 0) verbose = false;
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
    
    // Retrieve data
    cout << "Retrieving data" << endl;
    start = clock();
    project->retrieve();
    timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
    cout << "Finished retrieve in " << timeInSeconds << " seconds " << endl;

    // Export to native format
    cout << "Writing to new format" << endl;
    start = clock();
    CauldronIO::ImportExport::exportToXML(project, "output-dir");
    timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
    cout << "Wrote to new format in " << timeInSeconds << " seconds" << endl;
    return 0;
}

