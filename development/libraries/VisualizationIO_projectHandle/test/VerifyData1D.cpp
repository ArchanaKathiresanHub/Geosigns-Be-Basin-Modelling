//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by Shell India Markets Pvt. Ltd.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <gtest/gtest.h>

#include "database.h"
#include "cauldronschemafuncs.h"
#include "VisualizationAPI.h"
#include "VisualizationIO_native.h"
#include "ImportFromXML.h"
#include "ExportToXML.h"
#include "VisualizationIO_projectHandle.h"
#include "ImportProjectHandle.h"
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "Formation.h"
#include "Property.h"
#include "Snapshot.h"
#include "Reservoir.h"
#include "PropertyValue.h"
#include "Trap.h"
#include "Trapper.h"
#include "GridMap.h"
#include "Surface.h"
#include "SerialGridMap.h"
#include "Snapshot.h"
#include "Interface.h"

using namespace CauldronIO;
using namespace std;

void compareSnapshots(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d);
void compareProperties(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d);
void compareReservoirs(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d);
void compareFormations(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d);
bool copyDir(boost::filesystem::path const & source, boost::filesystem::path const & destination);
string createTemporaryDataset(string xmlFileName);
int convertToXML(string filePathP3d, DataAccess::Interface::ObjectFactory *factoryP3d);


class CompareTest1d : public ::testing::Test
{

public:
	CompareTest1d(){ m_factoryP3d = new DataAccess::Interface::ObjectFactory(); }
	~CompareTest1d() { delete m_factoryP3d; }
	DataAccess::Interface::ObjectFactory *m_factoryP3d;

};

void compare2dPropertyValues1d(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{
	DataAccess::Interface::PropertyValueList *propValueListP3d = projectP3d->getPropertyValues();
	size_t sizeP3d = propValueListP3d->size();
	size_t nSnapShotsXml = projectXml->getSnapShots().size();

	//Used as string separator in maps for keys
	string sep = "$#!";
	//Map to store 2D property values with index
	std::map <string, int> propValue2DMapP3d;

	//Creating map for 2D property values in project3d
	for (size_t i = 0; i < sizeP3d; i++)
	{
		const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(i);
		if ((propValueP3d->getStorage() != DataAccess::Interface::PropertyStorage::TIMEIOTBL))
		{
			continue;
		}
		double ageP3d = propValueP3d->getSnapshot()->getTime();
		string propNameP3d = propValueP3d->getProperty()->getName();
		string userNameP3d = propValueP3d->getProperty()->getUserName();
		string cauldronNameP3d = propValueP3d->getProperty()->getCauldronName();
		string formationNameP3d = propValueP3d->getFormation() ? propValueP3d->getFormation()->getName() : "";

		formationNameP3d = ((formationNameP3d.empty()) && (propValueP3d->getReservoir())) ? propValueP3d->getReservoir()->getFormation()->getName() : formationNameP3d;
		string surfaceNameP3d = propValueP3d->getSurface() ? propValueP3d->getSurface()->getName() : "";

		string key = std::to_string(ageP3d) + sep + propNameP3d + sep + userNameP3d + sep + cauldronNameP3d + sep + formationNameP3d + sep + surfaceNameP3d;
		ASSERT_TRUE(propValue2DMapP3d.count(key) == 0) << "Error:: Key not unique: " << key;
		propValue2DMapP3d.insert(std::make_pair(key, static_cast<int>(i)));

	}

	//Comparing 2D property values
	for (size_t i = 0; i < nSnapShotsXml; i++)
	{
		size_t nSurfacesXml = projectXml->getSnapShots()[i]->getSurfaceList().size();
		for (size_t j = 0; j < nSurfacesXml; j++)
		{
			size_t nPropSurfaceDataXml = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getPropertySurfaceDataList().size();
			string surfaceNameXml = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getName();
			double ageXml = projectXml->getSnapShots()[i]->getAge();
			const CauldronIO::PropertySurfaceDataList propSurfaceDataListXml = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getPropertySurfaceDataList();
			for (size_t k = 0; k < nPropSurfaceDataXml; k++)
			{
				const CauldronIO::PropertySurfaceData propSurfaceDataXml = propSurfaceDataListXml.at(k);
				std::shared_ptr<const CauldronIO::Property> propXml = propSurfaceDataXml.first;
				std::shared_ptr<CauldronIO::SurfaceData> surfaceDataXml = propSurfaceDataXml.second;

				string formationNameXml = surfaceDataXml->getFormation() ? surfaceDataXml->getFormation()->getName() : "";
				formationNameXml = ((formationNameXml.empty()) && (surfaceDataXml->getReservoir())) ? surfaceDataXml->getReservoir()->getFormation()->getName() : formationNameXml;

				string key = std::to_string(ageXml) + sep + propXml->getName() + sep + propXml->getUserName() + sep + propXml->getCauldronName() + sep + formationNameXml + sep + surfaceNameXml;

				std::map<std::string, int>::iterator itP3d = propValue2DMapP3d.find(key);

				int indexP3d = itP3d->second;
				int numIXml = static_cast<int>(surfaceDataXml->getGeometry()->getNumI());
				int numJXml = static_cast<int>(surfaceDataXml->getGeometry()->getNumJ());

				const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(indexP3d);
				DataAccess::Interface::GridMap* gridP3d = propValueP3d->getGridMap();
				int numIP3d = gridP3d->numI();
				int numJP3d = gridP3d->numJ();

				EXPECT_EQ(numIXml, numIP3d);
				EXPECT_EQ(numJXml, numJP3d);
				if (surfaceDataXml->isRetrieved())
				{
					for (int indexI = 0; indexI < numIXml; indexI++)
					{
						for (int indexJ = 0; indexJ < numJXml; indexJ++)
						{
																											EXPECT_FLOAT_EQ(gridP3d->getValue((unsigned int)indexI, indexJ), surfaceDataXml->getValue(indexI, indexJ));
																								}
					}
				}
			}
		}
	}
	cout << "Project1d: 2d property value comparison done" << endl;
}

// Test to compare metadata and property values in binary data stored in project3d and XML formats. The property values include TimeIo table, 1D mode

TEST_F(CompareTest1d, CompareData1d)
{
				boost::filesystem::path filePathP3d = "data/Project-1d.project3d";
	string fileNameP3d = filePathP3d.filename().string();

	cout << "Creating temporary copy of data" << endl;
	string testDataPath = createTemporaryDataset(filePathP3d.string());
	ASSERT_NE("error", testDataPath);
	boost::filesystem::path newFilePathP3d(testDataPath);
	newFilePathP3d /= fileNameP3d;

	int check = convertToXML(newFilePathP3d.string(),m_factoryP3d);
	ASSERT_EQ(check, 0);
	string fileNameXml = newFilePathP3d.stem().string() + ".xml";
	boost::filesystem::path newFilePathXml(newFilePathP3d.parent_path());
	newFilePathXml /= fileNameXml;

	//Open from project3d
	cout << "Opening project handle " << endl;
	std::unique_ptr<DataAccess::Interface::ProjectHandle> projectP3d(DataAccess::Interface::OpenCauldronProject(newFilePathP3d.string(), m_factoryP3d));
	ASSERT_EQ(true, (bool)projectP3d);

        if(projectP3d->getModellingMode() == CauldronIO::MODE1D) {
           projectP3d->setActivityOutputGrid(projectP3d->getLowResolutionOutputGrid ());
        }

	//Import from XML
	cout << "Starting import from XML" << endl;
	std::shared_ptr<CauldronIO::Project> projectXml = CauldronIO::ImportFromXML::importFromXML(newFilePathXml.string());
	cout << "Comparing data" << endl;
	//Comparing Project information
	EXPECT_EQ(projectP3d->getModellingMode(), projectXml->getModelingMode());

	compareSnapshots(projectXml, projectP3d);
	compareFormations(projectXml, projectP3d);
	compareReservoirs(projectXml, projectP3d);
	compareProperties(projectXml, projectP3d);
	compare2dPropertyValues1d(projectXml, projectP3d);

	newFilePathP3d = testDataPath;

	cout << "Deleting temporary copy of data" << endl;
	boost::filesystem::remove_all(newFilePathP3d);
}


