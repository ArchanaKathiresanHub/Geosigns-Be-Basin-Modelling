#include "../src/VisualizationAPI.h"
#include "../src/VisualizationIO_native.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>
#include <boost/shared_ptr.hpp>
#include <gtest/gtest.h>

using namespace CauldronIO;
using namespace std;

TEST( Project, Create )
{
    const string projectName("project");
    const string teamName("team");
    const string description("descript");
    const string version("version");
    ModellingMode mode = MODE1D;

    Project* project = new Project(projectName, description, teamName, version, mode, 0, 1);

    EXPECT_STREQ(project->getName().c_str(), projectName.c_str());
    EXPECT_STREQ(project->getDescription().c_str(), description.c_str());
    EXPECT_STREQ(project->getProgramVersion().c_str(), version.c_str());
    EXPECT_STREQ(project->getTeam().c_str(), teamName.c_str());
    EXPECT_EQ(project->getModelingMode(), mode);

    delete project;
}

TEST(SnapShot, Create)
{
    double age = 10.11;
    SnapShotKind kind = SYSTEM;
    bool isMinor = true;

    SnapShot* snap = new SnapShot(age, kind, isMinor);

    EXPECT_DOUBLE_EQ(snap->getAge(), age);
    EXPECT_EQ(snap->getKind(), kind);
    EXPECT_EQ(snap->isMinorShapshot(), isMinor);

    delete snap;
}

TEST(Project, AddSnapShot)
{
    const string projectName("project");
    const string teamName("team");
    const string description("descript");
    const string version("version");
    ModellingMode mode = MODE1D;

    boost::shared_ptr<Project> project(new Project(projectName, description, teamName, version, mode, 0, 1));
    boost::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));

    const SnapShotList& snapShotList = project->getSnapShots();
    EXPECT_EQ(snapShotList.size(), 0);
    project->addSnapShot(snapShot);
    EXPECT_EQ(snapShotList.size(), 1);
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
    boost::shared_ptr<Property> prop(new Property(name, userName, cauldronName, unit, type, attrib));

    EXPECT_STREQ(prop->getName().c_str(), name.c_str());
    EXPECT_STREQ(prop->getCauldronName().c_str(), cauldronName.c_str());
    EXPECT_STREQ(prop->getUserName().c_str(), userName.c_str());
    EXPECT_STREQ(prop->getUnit().c_str(), unit.c_str());
    EXPECT_EQ(prop->getAttribute(), attrib);
    EXPECT_EQ(prop->getType(), type);
}

TEST(SnapShot, Add)
{
    boost::shared_ptr<SnapShot> snapShot(new SnapShot(0, SYSTEM, false));
    
    const FormationVolumeList& formVolumes = snapShot->getFormationVolumeList();
    const SurfaceList& surfaceList = snapShot->getSurfaceList();

    EXPECT_EQ(formVolumes.size(), 0);
    EXPECT_EQ(surfaceList.size(), 0);

    // create a volume to add to the snapshot
    const string propName = "Depth";
    const string unit = "m";
    boost::shared_ptr<const Property> prop(new Property(propName, propName, propName, unit, FormationProperty, Continuous3DProperty));
    boost::shared_ptr<const Geometry3D> geometry3D(new Geometry3D(2, 2, 2, 0, 100, 100, 0, 0));
    boost::shared_ptr<Volume> volume(new Volume(Sediment, geometry3D));
    boost::shared_ptr<VolumeData> volumeData(new VolumeDataNative(geometry3D));
    PropertyVolumeData propVolume(prop, volumeData);
    volume->addPropertyVolumeData(propVolume);
    EXPECT_EQ(volume->getPropertyVolumeDataList().size(), 1);
    snapShot->setVolume(volume);

    // create a surface to add to the snapshot
    const string surfaceName = "waterbottom";
    boost::shared_ptr<const Geometry2D> geometry(new Geometry2D(2, 2, 100, 100, 0, 0));
    boost::shared_ptr<SurfaceData> valueMap(new MapNative(geometry));
    boost::shared_ptr<Surface> surface(new Surface(surfaceName, Sediment));
    surface->setGeometry(geometry);
    PropertySurfaceData propSurface(prop, valueMap);
    surface->addPropertySurfaceData(propSurface);
    snapShot->addSurface(surface);
    EXPECT_EQ(surfaceList.size(), 1);
}
