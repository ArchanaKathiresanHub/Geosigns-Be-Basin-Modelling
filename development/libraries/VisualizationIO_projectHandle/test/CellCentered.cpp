//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>
#include "../src/VisualizationAPI.h"
#include "../src/VisualizationIO_native.h"
#include "../src/VisualizationUtils.h"

using namespace CauldronIO;
using namespace std;

TEST( Utilities, CellCenterData )
{
	// create a project
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	int xmlVersionMjr = 2;
	int xmlVersionMnr = 1;
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, xmlVersionMjr, xmlVersionMnr));

	// create a snapshot
	double age = 10.11;
	SnapShotKind kind = SYSTEM;
	bool isMinor = true;
	std::shared_ptr<SnapShot> snap(new SnapShot(age, kind, isMinor));

	// create a geometry
	size_t numI = 2;
	size_t numJ = 3;
	size_t numK = 3;
	std::shared_ptr<Geometry3D> geometry(new Geometry3D(numI, numJ, numK, 3, 1.5, 2.5, 1.2, 3.2));
	project->addGeometry(geometry);

	// create volumeData
	std::shared_ptr<VolumeData> volumeData(new VolumeDataNative(geometry));
	float data[18] = { 0,0,0,0,0,0, 1,1,1,1,1,1, 2,2,2,2,DefaultUndefinedValue,DefaultUndefinedValue };
	volumeData->setData_IJK(data);

	// create property
	std::shared_ptr<const Property> prop(new Property("Temperature", "Temperature", "Temperature", "Kelvin", FormationProperty, Continuous3DProperty));
	project->addProperty(prop);

	//create a volume
	std::shared_ptr<Volume> volume(new Volume(None));
	volume->addPropertyVolumeData(PropertyVolumeData(prop, volumeData));

	// CELL center the volume
	VisualizationUtils::cellCenterVolume(volume, project);

	// get the cell-centered volume
	auto newVolData = volume->getPropertyVolumeDataList().at(0).second;
	EXPECT_EQ(newVolData->getGeometry()->getNumI(), numI - 1);
	EXPECT_EQ(newVolData->getGeometry()->getNumJ(), numJ - 1);
	EXPECT_EQ(newVolData->getGeometry()->getNumK(), numK - 1);

	const float* floatData = newVolData->getVolumeValues_IJK();

	EXPECT_DOUBLE_EQ(floatData[0], 0.5f);
	EXPECT_DOUBLE_EQ(floatData[1], 0.5f);
	EXPECT_DOUBLE_EQ(floatData[2], 1.5f);
	EXPECT_DOUBLE_EQ(floatData[3], 1.33333337f);

	// create a map
	std::shared_ptr<SurfaceData> surfData(new MapNative(geometry));
	float mapData[6] = { 0,1, 2,3, DefaultUndefinedValue, DefaultUndefinedValue };
	surfData->setData_IJ(mapData);

	std::shared_ptr<Surface> surface(new Surface("surface", None));
	surface->addPropertySurfaceData(PropertySurfaceData(prop, surfData));
	snap->addSurface(surface);

	// CELL center map
	VisualizationUtils::cellCenterAllMaps(snap, project);
	auto newSurfData = surface->getPropertySurfaceDataList().at(0).second;

	EXPECT_EQ(newSurfData->getGeometry()->getNumI(), numI - 1);
	EXPECT_EQ(newSurfData->getGeometry()->getNumJ(), numJ - 1);

	floatData = newSurfData->getSurfaceValues();
	EXPECT_DOUBLE_EQ(floatData[0], 1.50f);
	EXPECT_DOUBLE_EQ(floatData[1], 2.50f);

	// Create a formationVolume
	std::shared_ptr<Geometry3D> geometry1(new Geometry3D(numI, numJ, numK, 0, 1.5, 2.5, 1.2, 3.2));
	project->addGeometry(geometry1);
	std::shared_ptr<VolumeData> volumeData1(new VolumeDataNative(geometry1));
	std::shared_ptr<Geometry3D> geometry2(new Geometry3D(numI, numJ, numK, 2, 1.5, 2.5, 1.2, 3.2));
	project->addGeometry(geometry2);
	std::shared_ptr<VolumeData> volumeData2(new VolumeDataNative(geometry2));
	std::shared_ptr<Geometry3D> geometry3(new Geometry3D(numI, numJ, numK, 4, 1.5, 2.5, 1.2, 3.2));
	project->addGeometry(geometry3);
	std::shared_ptr<VolumeData> volumeData3(new VolumeDataNative(geometry3));

	float data1[18] = { 0,0,0,0,0,0, 1,1,1,1,1,1, DefaultUndefinedValue,DefaultUndefinedValue,DefaultUndefinedValue,
		DefaultUndefinedValue,DefaultUndefinedValue,DefaultUndefinedValue };
	float data2[18] = { 4,4,4,4,4,4, DefaultUndefinedValue,1,1,1,1,DefaultUndefinedValue, 0,0,0,0,0,0 };
	float data3[18] = { DefaultUndefinedValue,DefaultUndefinedValue,0,0,0,0, 1,1,1,1,1,1, 2,2,2,2,2,2 };

	volumeData1->setData_IJK(data1);
	volumeData2->setData_IJK(data2);
	volumeData3->setData_IJK(data3);

	std::shared_ptr<const Formation> formation1(new Formation(0, 2, "formation1", false, false));
	std::shared_ptr<const Formation> formation2(new Formation(2, 4, "formation2", false, false));
	std::shared_ptr<const Formation> formation3(new Formation(4, 6, "formation3", false, false));
	project->addFormation(formation1);
	project->addFormation(formation2);
	project->addFormation(formation3);

	std::shared_ptr<Volume> volume1(new Volume(None));
	volume1->addPropertyVolumeData(PropertyVolumeData(prop, volumeData1));
	std::shared_ptr<Volume> volume2(new Volume(None));
	volume2->addPropertyVolumeData(PropertyVolumeData(prop, volumeData2));
	std::shared_ptr<Volume> volume3(new Volume(None));
	volume3->addPropertyVolumeData(PropertyVolumeData(prop, volumeData3));

	snap->addFormationVolume(FormationVolume(formation1, volume1));
	snap->addFormationVolume(FormationVolume(formation2, volume2));
	snap->addFormationVolume(FormationVolume(formation3, volume3));

	// Cell center the formation volume
	VisualizationUtils::cellCenterFormationVolumes(snap, project);

	// Validate output
	auto mergedVolume = snap->getVolume()->getPropertyVolumeDataList().at(0).second;
	EXPECT_EQ(6, mergedVolume->getGeometry()->getNumK());
	EXPECT_EQ(numI - 1, mergedVolume->getGeometry()->getNumI());
	EXPECT_EQ(numJ - 1, mergedVolume->getGeometry()->getNumJ());

	// Check surfaces
	floatData = mergedVolume->getSurface_IJ(0);
	EXPECT_DOUBLE_EQ(floatData[0], 0.50f);
	EXPECT_DOUBLE_EQ(floatData[1], 0.50f);

	floatData = mergedVolume->getSurface_IJ(1);
	EXPECT_DOUBLE_EQ(floatData[0], 1.f);
	EXPECT_DOUBLE_EQ(floatData[1], 1.f);

	floatData = mergedVolume->getSurface_IJ(2);
	EXPECT_DOUBLE_EQ(floatData[0], 19.f/7.f);
	EXPECT_DOUBLE_EQ(floatData[1], 19.f/7.f);

	floatData = mergedVolume->getSurface_IJ(3);
	EXPECT_DOUBLE_EQ(floatData[0], 3.f/7.f);
	EXPECT_DOUBLE_EQ(floatData[1], 3.f/7.f);

	floatData = mergedVolume->getSurface_IJ(4);
	EXPECT_DOUBLE_EQ(floatData[0], 2.f/3.f);
	EXPECT_DOUBLE_EQ(floatData[1], 0.50f);

	floatData = mergedVolume->getSurface_IJ(5);
	EXPECT_DOUBLE_EQ(floatData[0], 1.5f);
	EXPECT_DOUBLE_EQ(floatData[1], 1.5f);
}

TEST(Utilities, DoNotCellCenterDepth)
{
	// TODO
}