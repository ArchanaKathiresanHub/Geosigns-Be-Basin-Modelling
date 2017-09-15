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
	
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");

	std::shared_ptr<Formation> formation(new Formation(kStart, kEnd, formationName));
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
	
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");

	std::shared_ptr<Formation> formation;
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
	
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");

	std::shared_ptr<Formation> formation(new Formation(kStart, kEnd, formationName));
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
	
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");

	std::shared_ptr<Formation> formation(new Formation(kStart, kEnd, formationName));
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
	
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	const string reservoirName("reservoir");

	std::shared_ptr<Formation> formation(new Formation(kStart, kEnd, formationName));
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
	
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	const string reservoirName("reservoir");

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName));
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
	
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	const string reservoirName("reservoir");

	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName));
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
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName));
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
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	std::shared_ptr<Formation> formation(new Formation(kStart, kEnd, formationName));
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
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	std::shared_ptr<Formation> formation(new Formation(kStart, kEnd, formationName));
	int start, end;
	formation->getK_Range(start, end);
	EXPECT_EQ(start, kStart);
	EXPECT_EQ(end, kEnd);
	EXPECT_EQ(formation->getName().c_str(), formationName);
}

TEST(Formation, Create_HandleEmptyName)
{
	int kStart = 1;
	int kEnd = 2;
	const string formationName;
	try{
		std::shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formationName));
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
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	std::shared_ptr<const Formation> formation1(new Formation(kStart, kEnd, formationName));
	std::shared_ptr<const Formation> formation2(new Formation(kStart, kEnd, formationName));
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
	int kStart = 1;
	int kEnd = 2;
	const string formationName("formation");
	std::shared_ptr<Formation> formation(new Formation(kStart, kEnd, formationName));
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

	EXPECT_EQ(ID, trapper->getID());
	EXPECT_EQ(persistentID, trapper->getPersistentID());

	std::shared_ptr<Trapper> trapper1(new Trapper());
        trapper1->setFreeGasVolume(9339458859.7369);
        EXPECT_FLOAT_EQ(9339458859.7369, trapper1->getFreeGasVolume());

        trapper1->setSolutionGasMass(1450545555.65666, CauldronIO::SpeciesNamesId::C3);
        EXPECT_DOUBLE_EQ(1450545555.65666, trapper1->getSolutionGasMass( CauldronIO::SpeciesNamesId::C3));
        EXPECT_DOUBLE_EQ(0.0, trapper1->getSolutionGasMass( CauldronIO::SpeciesNamesId::C5));

        trapper1->setFreeGasMass(2682893.23758079, CauldronIO::SpeciesNamesId::C6_MINUS_14ARO);
        EXPECT_DOUBLE_EQ(2682893.23758079, trapper1->getFreeGasMass( CauldronIO::SpeciesNamesId::C6_MINUS_14ARO));
        EXPECT_DOUBLE_EQ(0.0, trapper1->getFreeGasMass( CauldronIO::SpeciesNamesId::C6_MINUS_14SAT_S));

        trapper1->setCondensateMass(201456.126648199, CauldronIO::SpeciesNamesId::N2);
        EXPECT_DOUBLE_EQ(201456.126648199, trapper1->getCondensateMass( CauldronIO::SpeciesNamesId::N2));
        EXPECT_DOUBLE_EQ(0.0, trapper1->getCondensateMass( CauldronIO::SpeciesNamesId::COX));

        trapper1->setStockTankOilMass(97582712.1753246, CauldronIO::SpeciesNamesId::ASPHALTENE);
        EXPECT_DOUBLE_EQ(97582712.1753246, trapper1->getStockTankOilMass( CauldronIO::SpeciesNamesId::ASPHALTENE));
        EXPECT_DOUBLE_EQ(0.0, trapper1->getStockTankOilMass( CauldronIO::SpeciesNamesId::LSC));
}

TEST(Formation, Properties)
{
	Formation formation(-1, -1, "Bogus");

	EXPECT_FALSE(formation.isDepthRangeDefined());

	int kStart = 0, kEnd = 10;
	formation.updateK_range(kStart, kEnd);

	EXPECT_TRUE(formation.isDepthRangeDefined());
	int kStartOut, kEndOut;
	formation.getK_Range(kStartOut, kEndOut);

	EXPECT_EQ(kStart, kStartOut);
	EXPECT_EQ(kEnd, kEndOut);

	// Test some defaults
	EXPECT_FALSE(formation.hasAllochthonousLithology());
	EXPECT_FALSE(formation.hasChemicalCompaction());
	EXPECT_FALSE(formation.isMobileLayer());
	EXPECT_FALSE(formation.hasThicknessMap());
	EXPECT_FALSE(formation.hasConstrainedOverpressure());
	EXPECT_FALSE(formation.hasSourceRockMixingHIMap());
	EXPECT_FALSE(formation.hasLithoType1PercentageMap());
	EXPECT_FALSE(formation.hasLithoType2PercentageMap());
	EXPECT_FALSE(formation.hasLithoType3PercentageMap());
	EXPECT_FALSE(formation.isSourceRock());
	EXPECT_FALSE(formation.isIgneousIntrusion());
	EXPECT_FALSE(formation.getEnableSourceRockMixing());

	formation.setAllochthonousLithology(true);
	formation.setEnableSourceRockMixing(true);
	formation.setIsMobileLayer(true);
	formation.setChemicalCompaction(true);
	formation.setConstrainedOverpressure(true);
	formation.setIgneousIntrusion(true);

	EXPECT_TRUE(formation.hasAllochthonousLithology());
	EXPECT_TRUE(formation.hasConstrainedOverpressure());
	EXPECT_TRUE(formation.hasChemicalCompaction());
	EXPECT_TRUE(formation.isMobileLayer());
	EXPECT_TRUE(formation.isIgneousIntrusion());
	EXPECT_TRUE(formation.getEnableSourceRockMixing());

	// Assign some maps
	std::shared_ptr<Geometry2D> geometry(new Geometry2D(2, 2, 0.5, 0.5, 0, 0, false));
	std::shared_ptr<MapNative> map(new MapNative(geometry));
	map->setConstantValue(0.5);

	std::shared_ptr<const Property> propertyHI(new Property("HI", "username", "cauldronname", "unit", FormationProperty, Surface2DProperty));
	std::shared_ptr<const Property> propertyThickness(new Property("Thickness", "username", "cauldronname", "unit", FormationProperty, Surface2DProperty));
	std::shared_ptr<const Property> propertyLitho1(new Property("Litho1", "username", "cauldronname", "unit", FormationProperty, Surface2DProperty));
	std::shared_ptr<const Property> propertyLitho2(new Property("Litho2", "username", "cauldronname", "unit", FormationProperty, Surface2DProperty));
	std::shared_ptr<const Property> propertyLitho3(new Property("Litho3", "username", "cauldronname", "unit", FormationProperty, Surface2DProperty));

	PropertySurfaceData propSurfDataHI(propertyHI, map);
	PropertySurfaceData propSurfDataThickness(propertyThickness, map);
	PropertySurfaceData propSurfDataLitho1(propertyLitho1, map);
	PropertySurfaceData propSurfDataLitho2(propertyLitho2, map);
	PropertySurfaceData propSurfDataLitho3(propertyLitho3, map);

	formation.setSourceRockMixingHIMap(propSurfDataHI);
	formation.setThicknessMap(propSurfDataThickness);
	formation.setLithoType1PercentageMap(propSurfDataLitho1);
	formation.setLithoType2PercentageMap(propSurfDataLitho2);
	formation.setLithoType3PercentageMap(propSurfDataLitho3);

	EXPECT_TRUE(formation.hasLithoType1PercentageMap());
	EXPECT_TRUE(formation.hasLithoType2PercentageMap());
	EXPECT_TRUE(formation.hasLithoType3PercentageMap());
	EXPECT_TRUE(formation.hasThicknessMap());
	EXPECT_TRUE(formation.hasSourceRockMixingHIMap());

	// Test some strings
	const string sr1name("SR1");
	const string sr2name("SR2");
	const string fluid("Fluid");
	const string lithotype1("litho1");
	const string lithotype2("litho2");
	const string lithotype3("litho3");
	const string mixingModel("mixing");
	const string allocht("allocht");

	EXPECT_EQ(formation.getSourceRock1Name().length(), 0);
	EXPECT_EQ(formation.getSourceRock2Name().length(), 0);
	EXPECT_EQ(formation.getFluidType().length(), 0);
	EXPECT_EQ(formation.getLithoType1Name().length(), 0);
	EXPECT_EQ(formation.getLithoType2Name().length(), 0);
	EXPECT_EQ(formation.getLithoType3Name().length(), 0);
	EXPECT_EQ(formation.getMixingModel().length(), 0);
	EXPECT_EQ(formation.getAllochthonousLithologyName().length(), 0);

	formation.setSourceRock1Name(sr1name);
	formation.setSourceRock2Name(sr2name);
	formation.setFluidType(fluid);
	formation.setLithoType1Name(lithotype1);
	formation.setLithoType2Name(lithotype2);
	formation.setLithoType3Name(lithotype3);
	formation.setMixingModel(mixingModel);
	formation.setAllochthonousLithologyName(allocht);

	EXPECT_EQ(formation.getSourceRock1Name(), sr1name);
	EXPECT_EQ(formation.getSourceRock2Name(), sr2name);
	EXPECT_EQ(formation.getFluidType(), fluid);
	EXPECT_EQ(formation.getLithoType1Name(), lithotype1);
	EXPECT_EQ(formation.getLithoType2Name(), lithotype2);
	EXPECT_EQ(formation.getLithoType3Name(), lithotype3);
	EXPECT_EQ(formation.getMixingModel(), mixingModel);
	EXPECT_EQ(formation.getAllochthonousLithologyName(), allocht);

	int depoSequence = 5;
	formation.setDepoSequence(depoSequence);
	EXPECT_EQ(formation.getDepoSequence(), depoSequence);

	int elementRef = 1;
	formation.setElementRefinement(elementRef);
	EXPECT_EQ(formation.getElementRefinement(), elementRef);

	// Surfaces
	std::shared_ptr<Surface> surfaceTop(new Surface("topSurface", Sediment));
	std::shared_ptr<Surface> surfaceBottom(new Surface("bottomSurface", Sediment));

	formation.setTopSurface(surfaceTop);
	formation.setBottomSurface(surfaceBottom);

	EXPECT_TRUE(surfaceBottom == formation.getBottomSurface());
	EXPECT_TRUE(surfaceTop == formation.getTopSurface());

}

TEST(MigrationEvent, Properties)
{
    const string projectName("project");
    const string teamName("team");
    const string description("descript");
    const string version("version");
    ModellingMode mode = MODE1D;
    std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));
    std::shared_ptr<CauldronIO::MigrationEvent> event(new CauldronIO::MigrationEvent());

    const string process("Expulsion");
    event->setMigrationProcess(process);
    EXPECT_STREQ(process.c_str(), event->getMigrationProcess().c_str());

    float sourceAge = 2.0f;
    event->setSourceAge(sourceAge);
    EXPECT_FLOAT_EQ(sourceAge, event->getSourceAge());
    
    int srcTrapID = 1205;
    event->setSourceTrapID(srcTrapID);
    EXPECT_EQ(srcTrapID, event->getSourceTrapID());

    const string sourceRockName("srcName");
    event->setSourceRockName(sourceRockName);
    EXPECT_STREQ(sourceRockName.c_str(), event->getSourceRockName().c_str());

    const string sourceRockResName("srcResName");
    event->setSourceReservoirName(sourceRockResName);
    EXPECT_STREQ(sourceRockResName.c_str(), event->getSourceReservoirName().c_str());

    float srcPtX = 0.1f;
    event->setSourcePointX(srcPtX);
    EXPECT_FLOAT_EQ(srcPtX, event->getSourcePointX());

    float srcPtY = 0.2f;
    event->setSourcePointY(srcPtY);
    EXPECT_FLOAT_EQ(srcPtY, event->getSourcePointY());

    float dstAge = 3.0f;
    event->setDestinationAge(dstAge);
    EXPECT_FLOAT_EQ(dstAge, event->getDestinationAge());

    const string destReservoir("destReservoir");
    event->setDestinationReservoirName(destReservoir);
    EXPECT_STREQ(destReservoir.c_str(), event->getDestinationReservoirName().c_str());

    int dstTrapID = 1206;
    event->setDestinationTrapID(dstTrapID);
    EXPECT_EQ(dstTrapID, event->getDestinationTrapID());

    float dstPtX = 0.11f;
    event->setDestinationPointX(dstPtX);
    EXPECT_FLOAT_EQ(dstPtX, event->getDestinationPointX());

    float dstPtY = 0.21f;
    event->setDestinationPointY(dstPtY);
    EXPECT_FLOAT_EQ(dstPtY, event->getDestinationPointY());

    double massC1 = 0.10101;
    event->setMassC1(massC1);
    EXPECT_DOUBLE_EQ(massC1, event->getMassC1());

    double massC2 = 0.10201;
    event->setMassC2(massC2);
    EXPECT_DOUBLE_EQ(massC2, event->getMassC2());

    double massC3 = 0.103;
    event->setMassC3(massC3);
    EXPECT_DOUBLE_EQ(massC3, event->getMassC3());

    double massC5 = 0.104;
    event->setMassC5(massC5);
    EXPECT_DOUBLE_EQ(massC5, event->getMassC5());

    double massN2 = 0.105;
    event->setMassN2(massN2);
    EXPECT_DOUBLE_EQ(massN2, event->getMassN2());

    double massCOx = 0.106;
    event->setMassCOx(massCOx);
    EXPECT_DOUBLE_EQ(massCOx, event->getMassCOx());

    double massH2S = 0.107;
    event->setMassH2S(massH2S);
    EXPECT_DOUBLE_EQ(massH2S, event->getMassH2S());

    double massC6_14Aro = 0.108;
    event->setMassC6_14Aro(massC6_14Aro);
    EXPECT_DOUBLE_EQ(massC6_14Aro, event->getMassC6_14Aro());

    double massC6_14Sat = 0.109;
    event->setMassC6_14Sat(massC6_14Sat);
    EXPECT_DOUBLE_EQ(massC6_14Sat, event->getMassC6_14Sat());

    double massC15Aro = 0.101;
    event->setMassC15Aro(massC15Aro);
    EXPECT_DOUBLE_EQ(massC15Aro, event->getMassC15Aro());

    double massC15Sat = 0.1011;
    event->setMassC15Sat(massC15Sat);
    EXPECT_DOUBLE_EQ(massC15Sat, event->getMassC15Sat());

    double massLSC = 0.1012;
    event->setMassLSC(massLSC);
    EXPECT_DOUBLE_EQ(massLSC, event->getMassLSC());

    double massC15AT = 0.1013;
    event->setMassC15AT(massC15AT);
    EXPECT_DOUBLE_EQ(massC15AT, event->getMassC15AT());

    double massC15AroS = 0.1014;
    event->setMassC15AroS(massC15AroS);
    EXPECT_DOUBLE_EQ(massC15AroS, event->getMassC15AroS());

    double massC15SatS = 0.1015;
    event->setMassC15SatS(massC15SatS);
    EXPECT_DOUBLE_EQ(massC15SatS, event->getMassC15SatS());

    double massC6_14BT = 0.1016;
    event->setMassC6_14BT(massC6_14BT);
    EXPECT_DOUBLE_EQ(massC6_14BT, event->getMassC6_14BT());

    double massC6_14DBT = 0.1017;
    event->setMassC6_14DBT(massC6_14DBT);
    EXPECT_DOUBLE_EQ(massC6_14DBT, event->getMassC6_14DBT());

    double massC6_14BP = 0.1018;
    event->setMassC6_14BP(massC6_14BP);
    EXPECT_DOUBLE_EQ(massC6_14BP, event->getMassC6_14BP());

    double massC6_14SatS = 0.1019;
    event->setMassC6_14SatS(massC6_14SatS);
    EXPECT_DOUBLE_EQ(massC6_14SatS, event->getMassC6_14SatS());

    double massC6_14AroS = 0.102;
    event->setMassC6_14AroS(massC6_14AroS);
    EXPECT_DOUBLE_EQ(massC6_14AroS, event->getMassC6_14AroS());

    double massresins = 0.1021;
    event->setMassresins(massresins);
    EXPECT_DOUBLE_EQ(massresins, event->getMassresins());

    double massasphaltenes = 0.1022;
    event->setMassasphaltenes(massasphaltenes);
    EXPECT_DOUBLE_EQ(massasphaltenes, event->getMassasphaltenes());
}

TEST(TrapList, Create)
{
   std::shared_ptr<CauldronIO::Trap> trap(new CauldronIO::Trap());

   EXPECT_STREQ("", trap->getReservoirName().c_str());

   const string reservoirName("Reservoir1");
   trap->setReservoirName(reservoirName);
   EXPECT_STREQ(reservoirName.c_str(), trap->getReservoirName().c_str());

   const float x = 2.0f;
   const float y = 50.0f;
   trap->setSpillPointPosition(x, y);
   float xpos, ypos;
   trap->getSpillPointPosition(xpos, ypos);
   EXPECT_FLOAT_EQ(x, xpos);
   EXPECT_FLOAT_EQ(y, ypos);

   const double C1mass = 0.0234;
   trap->setMass(C1mass, CauldronIO::SpeciesNamesId::C1);
   EXPECT_DOUBLE_EQ(C1mass, trap->getMass(CauldronIO::SpeciesNamesId::C1));
   EXPECT_DOUBLE_EQ(0.0, trap->getMass(CauldronIO::SpeciesNamesId::C6_MINUS_14ARO_S));  

   trap->setFractSealStrength( -1.0f );
   EXPECT_FLOAT_EQ( -1.0f, trap->getFractSealStrength() );

   trap->setCriticalTemperatureOil ( 294.05f );
   EXPECT_FLOAT_EQ( 294.05f, trap->getCriticalTemperatureOil() );
}

TEST(FileReferences, Create)
{
    const string projectName("project");
    const string teamName("team");
    const string description("descript");
    const string version("version");
    ModellingMode mode = MODE3D;
    std::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 2, 1));

    // test for genex history records
    EXPECT_EQ( 0, (project->getGenexHistoryList()).size() );
    project->addGenexHistoryRecord("HistoryFile1.dat");
    const std::vector<std::string>& historyFiles = project->getGenexHistoryList();
    EXPECT_EQ( 1, historyFiles.size() );
    EXPECT_STREQ( "HistoryFile1.dat", (*historyFiles.begin()).c_str() );
   
    // test for the mass balance file record
    EXPECT_STREQ( "", project->getMassBalance().c_str() );
    project->setMassBalance("project_MassBalance");
    EXPECT_STREQ( "project_MassBalance", project->getMassBalance().c_str() );

    // test for the burial history records
    EXPECT_EQ( 0, (project->getBurialHistoryList()).size() );
    project->addBurialHistoryRecord("Well.BHF");
    const std::vector<std::string>& bhFiles = project->getBurialHistoryList();
    EXPECT_EQ( 1, bhFiles.size() );
    EXPECT_STREQ( "Well.BHF", (*bhFiles.begin()).c_str() );
 
}

TEST(Tables1D, Create)
{
   // DisplayContour
   std::shared_ptr<CauldronIO::DisplayContour> entry(new CauldronIO::DisplayContour());

   EXPECT_STREQ("", entry->getPropertyName().c_str());

   const string propertyName("Temperature");
   entry->setPropertyName(propertyName);
   EXPECT_STREQ(propertyName.c_str(), entry->getPropertyName().c_str());

   const float contourValue = 122.0f;
   entry->setContourValue(contourValue);
   EXPECT_FLOAT_EQ(contourValue, entry->getContourValue());

   const string contourColour("oranje");
   entry->setContourColour(contourColour);
   EXPECT_STREQ(contourColour.c_str(), entry->getContourColour().c_str());

   // TemperatureIso or VrIso
   std::shared_ptr<CauldronIO::IsoEntry> tempiso(new CauldronIO::IsoEntry());
   
   tempiso->setContourValue(contourValue);
   EXPECT_FLOAT_EQ(contourValue, tempiso->getContourValue());
   
   tempiso->setAge(50.0f);
   EXPECT_FLOAT_EQ(50.0f, tempiso->getAge());
   tempiso->setNP(35);
   EXPECT_EQ(35, tempiso->getNP());
   tempiso->setSum(6789.01);
   EXPECT_DOUBLE_EQ(6789.01, tempiso->getSum());

   // Biomarkerm table
   std::shared_ptr<CauldronIO::Biomarkerm> biom(new CauldronIO::Biomarkerm());
   biom->setDepthIndex(345.0f);
   EXPECT_FLOAT_EQ(345.0f, biom->getDepthIndex());
   biom->setHopaneIsomerisation(0.26f);
   EXPECT_FLOAT_EQ(0.26f, biom->getHopaneIsomerisation());
   biom->setSteraneIsomerisation(0.47f);
   EXPECT_FLOAT_EQ(0.47f, biom->getSteraneIsomerisation());
   biom->setSteraneAromatisation(0.81f);
   EXPECT_FLOAT_EQ(0.81f, biom->getSteraneAromatisation());
   biom->setOptimization(true);
   EXPECT_TRUE(biom->getOptimization());

   // FtSample
   std::shared_ptr<CauldronIO::FtSample> fiss(new CauldronIO::FtSample());
   
   EXPECT_STREQ("", fiss->getFtSampleId().c_str());
   const string sampleId("sampleID");
   
   fiss->setFtSampleId(sampleId);
   EXPECT_STREQ(sampleId.c_str(), fiss->getFtSampleId().c_str());
   
   fiss->setFtMeanRatio(0.54f);
   EXPECT_FLOAT_EQ(0.54f, fiss->getFtMeanRatio());
   
   EXPECT_FALSE(fiss->getOptimization());
   fiss->setOptimization(true);
   EXPECT_TRUE(fiss->getOptimization());
   
   // FtGrain
   std::shared_ptr<CauldronIO::FtGrain> fgrain(new CauldronIO::FtGrain());
   
   EXPECT_STREQ("", fgrain->getFtSampleId().c_str());
   fgrain->setFtSampleId(sampleId);
   EXPECT_STREQ(sampleId.c_str(), fgrain->getFtSampleId().c_str());
   
   fgrain->setFtGrainAgeErr(0.0034f);
   EXPECT_FLOAT_EQ(0.0034f, static_cast<float>(fgrain->getFtGrainAgeErr()));
   
   fgrain->setFtInducedTrackNo(23);
   EXPECT_EQ(23, fgrain->getFtInducedTrackNo());
   
   // FtPredLengthCountsHist
   std::shared_ptr<CauldronIO::FtPredLengthCountsHist> fpred(new CauldronIO::FtPredLengthCountsHist());
   
   EXPECT_STREQ("", fpred->getFtSampleId().c_str());
   fpred->setFtSampleId(sampleId);
   EXPECT_STREQ(sampleId.c_str(), fpred->getFtSampleId().c_str());
   
   EXPECT_EQ(0, fpred->getFtPredLengthHistId());
   fpred->setFtPredLengthHistId(21);
   EXPECT_EQ(21, fpred->getFtPredLengthHistId());
   
   fpred->setFtPredLengthBinWidth(45.0f);
   EXPECT_FLOAT_EQ(45.0f, fpred->getFtPredLengthBinWidth());
   
   // FtPredLengthCountsHistData
   std::shared_ptr<CauldronIO::FtPredLengthCountsHistData> fdata(new CauldronIO::FtPredLengthCountsHistData());
   
   EXPECT_FLOAT_EQ(0.0f, fdata->getFtPredLengthBinCount());
   fdata->setFtPredLengthBinCount(222.0f);
   EXPECT_FLOAT_EQ(222.0f, fdata->getFtPredLengthBinCount());
   
   fdata->setFtPredLengthBinIndex(3);
   EXPECT_EQ(3, fdata->getFtPredLengthBinIndex());
   
   fdata->setFtPredLengthHistId(-1);
   EXPECT_EQ(-1, fdata->getFtPredLengthHistId());
   
   // FtClWeightPercBins
   std::shared_ptr<CauldronIO::FtClWeightPercBins> ftcl(new CauldronIO::FtClWeightPercBins());
   
   ftcl->setFtClWeightBinStart(45.5768);
   EXPECT_DOUBLE_EQ(45.5768, ftcl->getFtClWeightBinStart());
   ftcl->setFtClWeightBinWidth(0.9456976);
   EXPECT_DOUBLE_EQ(0.9456976, ftcl->getFtClWeightBinWidth());
   
   //SmectiteIllite
   std::shared_ptr<CauldronIO::SmectiteIllite> smect(new CauldronIO::SmectiteIllite());
   
   EXPECT_STREQ("", smect->getLabel().c_str());
   smect->setLabel(sampleId);
   EXPECT_STREQ(sampleId.c_str(), smect->getLabel().c_str());
   
   smect->setDepthIndex(356.078f);
   EXPECT_FLOAT_EQ(356.078f, smect->getDepthIndex());
   
   EXPECT_FALSE(smect->getOptimization());
   smect->setOptimization(true);
   EXPECT_TRUE(smect->getOptimization());
   
   // DepthIo
   std::shared_ptr<CauldronIO::DepthIo> depth(new CauldronIO::DepthIo());
   
   EXPECT_STREQ("", depth->getPropertyName().c_str());
   depth->setPropertyName(propertyName);
   EXPECT_STREQ(propertyName.c_str(), depth->getPropertyName().c_str());
   
   depth->setMinimum(0.00000123f);
   EXPECT_FLOAT_EQ(0.00000123f, depth->getMinimum());
   
   EXPECT_FLOAT_EQ(0.0f, depth->getMaximum());
   
   depth->setP85(345.098f);
   EXPECT_FLOAT_EQ(345.098f, depth->getP85());
   
   depth->setNP(34);
   EXPECT_EQ(34, depth->getNP());
   
   depth->setKurtosis(0.5f);
   EXPECT_FLOAT_EQ(0.5f, depth->getKurtosis());
   
   // 1DTimeIo
   std::shared_ptr<CauldronIO::TimeIo1D> time1d(new CauldronIO::TimeIo1D());
   
   const std::string name("Surface");

   time1d->setSurfaceName(name);
   EXPECT_STREQ(name.c_str(), time1d->getSurfaceName().c_str());
   
   EXPECT_STREQ("", time1d->getPropertyName().c_str());
   time1d->setPropertyName(propertyName);
   EXPECT_STREQ(propertyName.c_str(), time1d->getPropertyName().c_str());
   
   time1d->setValue(123.04f);
   EXPECT_FLOAT_EQ(123.04f, time1d->getValue());
   
}
