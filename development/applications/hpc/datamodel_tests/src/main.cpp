#include "VisualizationAPI.h"
#include "ImportProjectHandle.h"
#include <ctime>
#include <cstring>

int main(int argc, char ** argv)
{
    if (argc <= 1) return 1;

    string projectFileName = argv[1];
    bool verbose = true;
    if (argc == 3 && std::strcmp(argv[2],"non-verbose") == 0) verbose = false;
    double readMB;
    size_t readSurfaces, readFormations, nonzeroSurfaces, nonzeroFormations;

    clock_t start = clock();
    boost::shared_ptr<CauldronIO::Project> project = ImportProjectHandle::CreateFromProjectHandle(projectFileName, verbose, readMB, 
        readSurfaces, nonzeroSurfaces, readFormations, nonzeroFormations);

    float timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
    cout << "Read " << readMB << " MB in " << timeInSeconds << " seconds: " << readMB / timeInSeconds << " MB/s" << endl;
    cout << "Read " << nonzeroSurfaces << " non-constant surfaces (" << readSurfaces << " total). Read " << nonzeroFormations << " non-constant formations ("
        << readFormations << " total)" << endl;
        
    return 0;
}

#if 0
// Forward declarations
void GetInfo(boost::shared_ptr<Interface::ProjectHandle> projectHandle);
void ListProperties(boost::shared_ptr<PropertyList> &propertyList, const int prop2D3D);
string typeToString(PropertyType pType) const;
string storageToString(PropertyStorage storage) const;

//////////////////////////////////////////////////////////////////////////

void GetInfo(boost::shared_ptr<Interface::ProjectHandle> projectHandle)
{
   boost::shared_ptr<PropertyList> propertyList;

   // Get a list of all (397) properties
   propertyList.reset(projectHandle->getProperties(true, allSelection, 0, 0, 0, 0, VOLUME));
   cout << "Found " << propertyList->size() << " total volume properties" << endl;
   propertyList.reset(projectHandle->getProperties(true, allSelection, 0, 0, 0, 0, MAP));
   cout << "Found " << propertyList->size() << " total map properties" << endl;
   // Find actual properties
   propertyList.reset(projectHandle->getProperties(false, allSelection, 0, 0, 0, 0, MAP));
   cout << "Found " << propertyList->size() << " actual map properties" << endl;
   propertyList.reset(projectHandle->getProperties(false, allSelection, 0, 0, 0, 0, VOLUME)); 
   cout << "Found " << propertyList->size() << " actual volume properties" << endl;

   /*
   // Display all properties
   cout << "VOLUME properties" << endl;
   ListProperties(propertyList, VOLUME);
   cout << "MAP properties" << endl;
   propertyList.reset(projectHandle->getProperties(false, allSelection, 0, 0, 0, 0, MAP));
   ListProperties(propertyList, MAP);
   */

   // Find all snapshots
   boost::shared_ptr<SnapshotList> snapShots;
   snapShots.reset(projectHandle->getSnapshots());
   cout << "Found " << snapShots->size() << " snapshots" << endl;

   /*
   // Show snapshot times
   for (int i = 0; i < snapShots->size(); i++)
   {
	   const Snapshot* snapShot = snapShots->at(i);
	   cout << "=== Snapshot " << i << " Age: " << snapShot->getTime() << " Ma ===" << endl;
	   // List all properties per snapshot
	   propertyList.reset(projectHandle->getProperties(false, allSelection, snapShot, 0, 0, 0, VOLUME));
	   ListProperties(propertyList, VOLUME);
   }
   */

   // Read some property values
   if (snapShots->size() < 1) abort();
   const Snapshot* snapShot = snapShots->at(0);
   propertyList.reset(projectHandle->getProperties(false, FORMATION, snapShot, 0, 0, 0, VOLUME));

   if (propertyList->size() > 0)
   {
	   const Property* prop = propertyList->at(0);
	   cout << "Snapshot Age: " << snapShot->getTime() << endl;
	   cout << "Property name: " << prop->getName() << endl;
	   
	   // Get propertyValue list for first 3D property in last snapshot
	   boost::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(FORMATION, prop, snapShot, 0, 0, 0, VOLUME));

	   // List attributes of these property value objects
	   for (int i = 0; i < propValues->size(); ++i)
	   {
		   const PropertyValue* propval = propValues->at(i);
		   cout << " == PropertyValue " << i << " == " << endl;
		   cout << " -- storage type: " << storageToString(propval->getStorage()) << endl;
		   const Formation* formation = propval->getFormation();
		   if (formation)
		   {
			   cout << " -- found formation " << formation->getName() << endl;
		   }
		   GridMap* gridmap = propval->getGridMap(); // this query will actually get the data itself
		   if (gridmap)
		   {
			   cout << " -- found gridmap " << "size(i,j,k): (" << gridmap->numI() << "," << gridmap->numJ() << "," << 1 + gridmap->getDepth() << ")" << endl;
		   }
	   }
   }
}

string typeToString(PropertyType pType)
{
    switch (pType)
    {
    case FORMATIONPROPERTY:
        return string("FORMATIONPROPERTY");
    case RESERVOIRPROPERTY:
        return string("RESERVOIRPROPERTY");
    case TRAPPROPERTY:
        return string("TRAPPROPERTY");
    }
    return string();
}

string storageToString(PropertyStorage storage)
{
    switch (storage)
    {
    case SNAPSHOTIOTBL:
        return string("SNAPSHOTIOTBL");
    case THREEDTIMEIOTBL:
        return string("THREEDTIMEIOTBL");
    case TIMEIOTBL:
        return string("TIMEIOTBL");
    }
    return string();
}

void ListProperties(boost::shared_ptr<PropertyList> &propertyList, const int prop2D3D)
{
    for (int i = 0; i < propertyList->size(); ++i)
    {
        const Property* prop = propertyList->at(i);
        if (prop->hasPropertyValues(allSelection, 0, 0, 0, 0, prop2D3D))
        {
            std::cout << "Property:     " << prop->getName() << endl;
            // Get its attribute
            PropertyAttribute attrb = prop->getPropertyAttribute();
            std::cout << "-- attribute: " << getPropertyAttributeImage(attrb) << endl;
            Interface::PropertyType pType = prop->getType();
            std::cout << "------- type: " << typeToString(pType) << endl;
        }
    }
}

#endif
