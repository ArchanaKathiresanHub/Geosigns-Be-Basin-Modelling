//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/VisualizationAPI.h"
#include "../src/VisualizationIO_native.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <gtest/gtest.h>

using namespace CauldronIO;
using namespace std;

TEST(Project, Create)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	int xmlVersionMjr = 2;
	int xmlVersionMnr = 1;

	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, xmlVersionMjr, xmlVersionMnr));

	EXPECT_STREQ(project->getName().c_str(), projectName.c_str());
	EXPECT_STREQ(project->getDescription().c_str(), description.c_str());
	EXPECT_STREQ(project->getProgramVersion().c_str(), version.c_str());
	EXPECT_STREQ(project->getTeam().c_str(), teamName.c_str());
	EXPECT_EQ(project->getModelingMode(), mode);
	EXPECT_EQ(project->getXmlVersionMajor(), xmlVersionMjr);
	EXPECT_EQ(project->getXmlVersionMinor(), xmlVersionMnr);
}

TEST(SnapShot, Create)
{
	double age = 10.11;
	SnapShotKind kind = SYSTEM;
	bool isMinor = true;

	std::shared_ptr<SnapShot> snap(new SnapShot(age, kind, isMinor));
	EXPECT_DOUBLE_EQ(snap->getAge(), age);
	EXPECT_EQ(snap->getKind(), kind);
	EXPECT_EQ(snap->isMinorShapshot(), isMinor);
}

TEST(SnapShot, Create_HandleNegativeAge)
{
	double age = -10.11;
	SnapShotKind kind = SYSTEM;
	bool isMinor = true;
	try{
		std::shared_ptr<SnapShot> snap(new SnapShot(age, kind, isMinor));
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "SnapShot age cannot be negative");
	}
	catch (...){
		FAIL() << "Expected: SnapShot age cannot be negative";
	}

}

TEST(Project, AddSnapShot)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;

	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2,1));
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));

	const SnapShotList& snapShotList = project->getSnapShots();
	EXPECT_EQ(snapShotList.size(), 0);
	project->addSnapShot(snapShot);
	EXPECT_EQ(snapShotList.size(), 1);
}

TEST(Project, AddSnapShot_HandleEmptySnapShot)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	std::shared_ptr<SnapShot> snapShot;

	const SnapShotList& snapShotList = project->getSnapShots();

	try{
		project->addSnapShot(snapShot);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add empty snapshot");
		EXPECT_EQ(snapShotList.size(), 0);
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add empty snapshot";
	}
}

TEST(Project, AddSnapShot_HandleDuplicateSnapShot)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));
	project->addSnapShot(snapShot);
	const SnapShotList& snapShotList = project->getSnapShots();

	try{
		project->addSnapShot(snapShot);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add snapshot twice");
		EXPECT_EQ(snapShotList.size(), 1);
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add snapshot twice";
	}
}

TEST(Project, AddProperty)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	const string name = "Depth";
	const string cauldronName = "cDepth";
	const string userName = "uDepth";
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;

	std::shared_ptr<const Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const PropertyList& propertyList = project->getProperties();
	EXPECT_EQ(propertyList.size(), 0);
	project->addProperty(prop);
	EXPECT_EQ(propertyList.size(), 1);
}

TEST(Project, AddProperty_HandleDuplicateProperty)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	const string name = "Depth";
	const string cauldronName = "cDepth";
	const string userName = "uDepth";
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;

	std::shared_ptr<const Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const PropertyList& propertyList = project->getProperties();

	project->addProperty(prop);
	EXPECT_EQ(propertyList.size(), 1);
	project->addProperty(prop);
	EXPECT_EQ(propertyList.size(), 1);
}

TEST(Project, FindProperty)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	const string name = "Depth";
	const string cauldronName = "cDepth";
	const string userName = "uDepth";
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;

	std::shared_ptr<const Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const PropertyList& propertyList = project->getProperties();

	project->addProperty(prop);
	std::shared_ptr<const Property> newProp = project->findProperty(name);
	EXPECT_STREQ(newProp->getName().c_str(), name.c_str());
}

TEST(Project, AddFormation)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const FormationList& formationList = project->getFormations();
	EXPECT_EQ(formationList.size(), 0);
	project->addFormation(formation);
	EXPECT_EQ(formationList.size(), 1);
}

TEST(Project, AddFormation_HandleEmptyFormation)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;

	std::shared_ptr<const Formation> formation;
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const FormationList& formationList = project->getFormations();

	try{
		project->addFormation(formation);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add empty formation");
		EXPECT_EQ(formationList.size(), 0);
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add empty formation";
	}
}

TEST(Project, AddFormation_HandleDuplicateFormation)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const FormationList& formationList = project->getFormations();
	project->addFormation(formation);

	try{
		project->addFormation(formation);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add formation twice");
		EXPECT_EQ(formationList.size(), 1);
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add formation twice";
	}
}

TEST(Project, FindFormation)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	project->addFormation(formation);

	std::shared_ptr<const Formation> newFormation = project->findFormation(formationName);
	EXPECT_STREQ(newFormation->getName().c_str(), formationName.c_str());
}

TEST(Project, AddReservoir)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	const string reservoirName("reservoir");

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<const Reservoir> reservoir(new Reservoir(reservoirName, formation));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const ReservoirList& reservoirList = project->getReservoirs();
	EXPECT_EQ(reservoirList.size(), 0);
	project->addReservoir(reservoir);
	EXPECT_EQ(reservoirList.size(), 1);
}

TEST(Project, AddReservoir_HandleEmptyReservoir)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	const string reservoirName("reservoir");
	std::shared_ptr<const Formation> formation;
	std::shared_ptr<const Reservoir> reservoir;
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const ReservoirList& reservoirList = project->getReservoirs();

	try{
		project->addReservoir(reservoir);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add empty reservoir");
		EXPECT_EQ(reservoirList.size(), 0);
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add empty reservoir";
	}
}

TEST(Project, AddReservoir_HandleDuplicateReservoir)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	const string reservoirName("reservoir");

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<const Reservoir> reservoir(new Reservoir(reservoirName, formation));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	const ReservoirList& reservoirList = project->getReservoirs();
	project->addReservoir(reservoir);

	try{
		project->addReservoir(reservoir);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add reservoir twice");
		EXPECT_EQ(reservoirList.size(), 1);
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add reservoir twice";
	}
}

TEST(Project, FindReservoir)
{
	const string projectName("project");
	const string teamName("team");
	const string description("descript");
	const string version("version");
	ModellingMode mode = MODE1D;
	
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	const string reservoirName("reservoir");

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<const Reservoir> reservoir(new Reservoir(reservoirName, formation));
	std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
	project->addReservoir(reservoir);

	std::shared_ptr<const Reservoir> newReservoir = project->findReservoir(reservoirName);
	EXPECT_STREQ(newReservoir->getName().c_str(), reservoirName.c_str());
}

TEST(Property, Create)
{
	// create a volume to add to the snapshot
	const string name = "Depth";
	const string cauldronName = "cDepth";
	const string userName = "uDepth";
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;
	std::shared_ptr<Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));

	EXPECT_STREQ(prop->getName().c_str(), name.c_str());
	EXPECT_STREQ(prop->getCauldronName().c_str(), cauldronName.c_str());
	EXPECT_STREQ(prop->getUserName().c_str(), userName.c_str());
	EXPECT_STREQ(prop->getUnit().c_str(), unit.c_str());
	EXPECT_EQ(prop->getAttribute(), attrib);
	EXPECT_EQ(prop->getType(), type);

}

TEST(Property, Create_HandleEmptyPropertyName)
{
	// create a volume to add to the snapshot
	const string name;
	const string cauldronName = "cDepth";
	const string userName = "uDepth";
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;

	try{
		std::shared_ptr<Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Property name cannot be empty");

	}
	catch (...)
	{
		FAIL() << "Expected: Property name cannot be empty";
	}
}

TEST(Property, Create_HandleEmptyCauldronName)
{
	// create a volume to add to the snapshot
	const string name = "Depth";
	const string cauldronName;
	const string userName = "uDepth";
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;

	try{
		std::shared_ptr<Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cauldron name cannot be empty");

	}
	catch (...)
	{
		FAIL() << "Expected: Cauldron name cannot be empty";
	}
}

TEST(Property, Create_HandleEmptyUserName)
{
	// create a volume to add to the snapshot
	const string name = "Depth";
	const string cauldronName = "cDepth";
	const string userName;
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;

	try{
		std::shared_ptr<Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "User name cannot be empty");

	}
	catch (...)
	{
		FAIL() << "Expected: User name cannot be empty";
	}
}

TEST(Property, OperatorEquals)
{
	// create a volume to add to the snapshot
	const string name = "Depth";
	const string cauldronName = "cDepth";
	const string userName = "uDepth";
	const string unit = "unit";
	PropertyType type = FormationProperty;
	PropertyAttribute attrib = Continuous3DProperty;
	std::shared_ptr<Property> prop1(new Property(name, userName, cauldronName, unit, type, attrib));
	std::shared_ptr<Property> prop2(new Property(name, userName, cauldronName, unit, type, attrib));
	bool isEqual = *prop1 == *prop2;
	EXPECT_EQ(isEqual, true);
}

TEST(SnapShot, Add)
{
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));

	const FormationVolumeList& formVolumes = snapShot->getFormationVolumeList();
	const SurfaceList& surfaceList = snapShot->getSurfaceList();
	const TrapperList& trapperList = snapShot->getTrapperList();

	EXPECT_EQ(formVolumes.size(), 0);
	EXPECT_EQ(surfaceList.size(), 0);
	EXPECT_EQ(trapperList.size(), 0);

	// create a formation volume to add to the snapshot
	std::shared_ptr<const Geometry3D> geometry3D(new Geometry3D(2, 2, 2, 0, 100, 100, 0, 0));
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<Volume> volume(new Volume(Sediment));
	FormationVolume formationVolume = FormationVolume(formation, volume);
	snapShot->addFormationVolume(formationVolume);
	EXPECT_EQ(formVolumes.size(), 1);

	// create a surface to add to the snapshot
	const string surfaceName = "waterbottom";
	const string propName = "Depth";
	const string unit = "m";
	std::shared_ptr<const Property> prop(new Property(propName, propName, propName, unit, FormationProperty, Continuous3DProperty));
	std::shared_ptr<const Geometry2D> geometry(new Geometry2D(2, 2, 100, 100, 0, 0));
	std::shared_ptr<SurfaceData> valueMap(new MapNative(geometry));
	std::shared_ptr<Surface> surface(new Surface(surfaceName, Sediment));
	PropertySurfaceData propSurface = PropertySurfaceData(prop, valueMap);
	surface->addPropertySurfaceData(propSurface);
	snapShot->addSurface(surface);
	EXPECT_EQ(surfaceList.size(), 1);

	//create a trapper to add to the snapshot
	int ID = 1234;
	int persistentID = 2345;
	std::shared_ptr<Trapper> trapper(new Trapper(ID, persistentID));
	snapShot->addTrapper(trapper);
	EXPECT_EQ(trapperList.size(), 1);

}

TEST(SnapShot, AddSurface_HandleEmptySurface)
{
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));

	const SurfaceList& surfaceList = snapShot->getSurfaceList();
	EXPECT_EQ(surfaceList.size(), 0);

	// create a surface to add to the snapshot
	const string surfaceName = "waterbottom";
	std::shared_ptr<const Geometry2D> geometry(new Geometry2D(2, 2, 100, 100, 0, 0));
	std::shared_ptr<SurfaceData> valueMap(new MapNative(geometry));
	std::shared_ptr<Surface> surface;
	try{
		snapShot->addSurface(surface);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add empty surface");
		EXPECT_EQ(surfaceList.size(), 0);

	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add empty surface";

	}

}

TEST(SnapShot, AddSurface_HandleDuplicateSurface)
{
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));

	const SurfaceList& surfaceList = snapShot->getSurfaceList();
	EXPECT_EQ(surfaceList.size(), 0);

	// create a surface to add to the snapshot
	const string propName = "Depth";
	const string unit = "m";
	std::shared_ptr<const Property> prop(new Property(propName, propName, propName, unit, FormationProperty, Continuous3DProperty));
	const string surfaceName = "waterbottom";
	std::shared_ptr<const Geometry2D> geometry(new Geometry2D(2, 2, 100, 100, 0, 0));
	std::shared_ptr<SurfaceData> valueMap(new MapNative(geometry));
	std::shared_ptr<Surface> surface(new Surface(surfaceName, Sediment));
	PropertySurfaceData propSurface = PropertySurfaceData(prop, valueMap);
	surface->addPropertySurfaceData(propSurface);
	snapShot->addSurface(surface);

	try{
		snapShot->addSurface(surface);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add surface twice");
		EXPECT_EQ(surfaceList.size(), 1);

	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add surface twice";

	}

}

TEST(SnapShot, AddFormationVolume_HandleDuplicateVolume)
{
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));
	const FormationVolumeList& formVolumes = snapShot->getFormationVolumeList();
	std::shared_ptr<const Geometry3D> geometry3D(new Geometry3D(2, 2, 2, 0, 100, 100, 0, 0));
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<Volume> volume(new Volume(Sediment));
	FormationVolume formationVolume = FormationVolume(formation, volume);
	snapShot->addFormationVolume(formationVolume);

	try{
		snapShot->addFormationVolume(formationVolume);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add volume twice");
		EXPECT_EQ(formVolumes.size(), 1);

	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add volume twice";
	}

}

TEST(SnapShot, AddTrapper_HandleEmptyTrapper)
{
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));
	const TrapperList& trapperList = snapShot->getTrapperList();

	int ID = 1234;
	int persistentID = 2345;
	std::shared_ptr<Trapper> trapper;

	try{
		snapShot->addTrapper(trapper);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add empty trapper");
		EXPECT_EQ(trapperList.size(), 0);

	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add empty trapper";
	}
}

TEST(SnapShot, AddTrapper_HandleDuplicateTrapper)
{
	std::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));
	const TrapperList& trapperList = snapShot->getTrapperList();

	int ID = 1234;
	int persistentID = 2345;
	std::shared_ptr<Trapper> trapper(new Trapper(ID, persistentID));
	snapShot->addTrapper(trapper);

	try{
		snapShot->addTrapper(trapper);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add trapper twice");
		EXPECT_EQ(trapperList.size(), 1);
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add trapper twice";
	}
}

TEST(Formation, Create)
{
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	unsigned int start, end;
	formation->getK_Range(start, end);
	EXPECT_EQ(start, kStart);
	EXPECT_EQ(end, kEnd);
	EXPECT_EQ(formation->getName().c_str(), formationName);
	EXPECT_EQ(formation->isMobileLayer(), isMobileLayer);
	EXPECT_EQ(formation->isSourceRock(), isSourceRock);
}

TEST(Formation, Create_HandleEmptyName)
{
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName;
	bool isSourceRock = true;
	bool isMobileLayer = true;
	try{
		std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Formation name cannot be empty");
	}
	catch (...)
	{
		FAIL() << "Expected: Formation name cannot be empty";
	}
}

TEST(Formation, OperatorEquals)
{
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	std::shared_ptr<const Formation> formation1(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	std::shared_ptr<const Formation> formation2(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	bool isEqual = *formation1 == *formation2;
	EXPECT_EQ(isEqual, true);

}

TEST(Surface, Create)
{
	const string surfaceName = "waterbottom";
	std::shared_ptr<Surface> surface(new Surface(surfaceName, Sediment));
	EXPECT_STREQ(surface->getName().c_str(), surfaceName.c_str());
	EXPECT_EQ(surface->getSubSurfaceKind(), Sediment);

}

TEST(Surface, SetFormation)
{
	const string surfaceName = "waterbottom";
	std::shared_ptr<Surface> surface(new Surface(surfaceName, Sediment));
	size_t kStart = 1;
	size_t kEnd = 2;
	const string formationName("formation");
	bool isSourceRock = true;
	bool isMobileLayer = true;
	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer));
	surface->setFormation(formation, true);
	EXPECT_STREQ(surface->getTopFormation()->getName().c_str(), formationName.c_str());

}

TEST(Surface, AddPropertySurfaceData_HandleDuplicateData)
{
	const string propName = "Depth";
	const string unit = "m";
	std::shared_ptr<const Property> prop(new Property(propName, propName, propName, unit, FormationProperty, Continuous3DProperty));
	const string surfaceName = "waterbottom";
	std::shared_ptr<const Geometry2D> geometry(new Geometry2D(2, 2, 100, 100, 0, 0));
	std::shared_ptr<SurfaceData> valueMap(new MapNative(geometry));
	std::shared_ptr<Surface> surface(new Surface(surfaceName, Sediment));
	PropertySurfaceData propSurface = PropertySurfaceData(prop, valueMap);
	surface->addPropertySurfaceData(propSurface);
	try{
		surface->addPropertySurfaceData(propSurface);
		FAIL();
	}
	catch (CauldronIOException const & err)
	{
		EXPECT_STREQ(err.what(), "Cannot add property-surfaceData twice");
	}
	catch (...)
	{
		FAIL() << "Expected: Cannot add property-surfaceData twice";
	}

}

TEST(Geometry2D, Create)
{
	size_t numI = 2;
	size_t numJ = 3;
	double deltaI = 1.5;
	double deltaJ = 2.5;
	double minI = 1.2;
	double minJ = 3.2;
	double maxI = minI + deltaI * numI;
	double maxJ = minJ + deltaJ * numJ;
	std::shared_ptr<const Geometry2D> geometry(new Geometry2D(numI, numJ, deltaI, deltaJ, minI, minJ));
	EXPECT_EQ(geometry->getNumI(), numI);
	EXPECT_EQ(geometry->getNumJ(), numJ);
	EXPECT_DOUBLE_EQ(geometry->getDeltaI(), deltaI);
	EXPECT_DOUBLE_EQ(geometry->getDeltaJ(), deltaJ);
	EXPECT_DOUBLE_EQ(geometry->getMinI(), minI);
	EXPECT_DOUBLE_EQ(geometry->getMinJ(), minJ);
	EXPECT_DOUBLE_EQ(geometry->getMaxI(), maxI);
	EXPECT_DOUBLE_EQ(geometry->getMaxJ(), maxJ);
}

TEST(Volume, Create)
{
	size_t numI = 2;
	size_t numJ = 3;
	size_t numK = 4;
	size_t offsetK = 3;
	double deltaI = 1.5;
	double deltaJ = 2.5;
	double minI = 1.2;
	double minJ = 3.2;

	SubsurfaceKind kind = Sediment;
	std::shared_ptr<Volume> volume(new Volume(kind));
	EXPECT_EQ(volume->getSubSurfaceKind(), kind);
}

TEST(Geometry3D, Create)
{
	size_t numI = 2;
	size_t numJ = 3;
	size_t numK = 4;
	size_t offsetK = 3;
	double deltaI = 1.5;
	double deltaJ = 2.5;
	double minI = 1.2;
	double minJ = 3.2;
	double maxI = minI + deltaI * numI;
	double maxJ = minJ + deltaJ * numJ;
	size_t lastK = offsetK + numK - 1;
	std::shared_ptr<const Geometry3D> geometry(new Geometry3D(numI, numJ, numK, offsetK, deltaI, deltaJ, minI, minJ));
	EXPECT_EQ(geometry->getNumI(), numI);
	EXPECT_EQ(geometry->getNumJ(), numJ);
	EXPECT_EQ(geometry->getNumK(), numK);
	EXPECT_EQ(geometry->getFirstK(), offsetK);
	EXPECT_DOUBLE_EQ(geometry->getDeltaI(), deltaI);
	EXPECT_DOUBLE_EQ(geometry->getDeltaJ(), deltaJ);
	EXPECT_DOUBLE_EQ(geometry->getMinI(), minI);
	EXPECT_DOUBLE_EQ(geometry->getMinJ(), minJ);
	EXPECT_DOUBLE_EQ(geometry->getMaxI(), maxI);
	EXPECT_DOUBLE_EQ(geometry->getMaxJ(), maxJ);
	EXPECT_EQ(geometry->getLastK(), lastK);

}

TEST(Trapper, Create)
{
	int ID = 1234;
	int persistentID = 2345;
	std::shared_ptr<const Trapper> trapper(new Trapper(ID, persistentID));

	EXPECT_EQ(trapper->getID(), ID);
	EXPECT_EQ(trapper->getPersistentID(), persistentID);

}