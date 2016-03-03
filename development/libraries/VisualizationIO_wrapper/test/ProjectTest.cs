/*******************************************************************
 * 
 * Copyright (C) 2015 Shell International Exploration & Production.
 * All rights reserved.
 * 
 * Developed under license for Shell by PDS BV.
 * 
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell.
 * 
 *******************************************************************/


using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;
using Shell.BasinModeling.CauldronIO;

namespace Shell.BasinModeling.CauldronIO.Test
{
    /// <summary>
    ///This is a test class for suaAPI
    ///</summary>
    [TestClass()]
    public class ProjectTests
    {
        [TestMethod]
        public void LoadProjectNative()
        {
            Project project = ImportExport.importFromXML("../../../csharp-test/HydrostaticGL_out.xml");
            int count = project.getSnapShots().Count;
            Assert.IsTrue(count == 14);
            count = project.getSurfaceNames().Count;
            Assert.IsTrue(count == 5);
            count = project.getProperties().Count;
            Assert.IsTrue(count == 23);
            count = project.getFormations().Count;
            Assert.IsTrue(count == 4);
            count = project.getReservoirs().Count;
            Assert.IsTrue(count == 0);
            count = project.getSnapShots()[0].getFormationVolumeList().Count;
            Assert.IsTrue(count == 4);
            count = project.getSnapShots()[0].getVolume().getPropertyVolumeDataList().Count;
            Assert.IsTrue(count == 7);
            string name = project.getSnapShots()[0].getSurfaceList()[0].getBottomFormation().getName();
            StringAssert.Equals(name, "Erosion");
            uint num = project.getSnapShots()[0].getSurfaceList()[0].getGeometry().getNumI();
            Assert.IsTrue(num == 2);
            num = project.getSnapShots()[0].getSurfaceList()[0].getGeometry().getNumJ();
            Assert.IsTrue(num == 2);
            Assert.IsFalse(project.getSnapShots()[13].isMinorShapshot());
        }

        [TestMethod]
        public void AddToProjectNative()
        {
            Project project = ImportExport.importFromXML("../../../csharp-test/HydrostaticGL_out.xml");
            int count = project.getSnapShots().Count;
            Assert.IsTrue(count == 14);

            // Create a new volume
            string propName = "Depth";
            string unit = "m";
            Property prop = new Property(propName, propName, propName, unit, PropertyType.FormationProperty, PropertyAttribute.Continuous3DProperty);
            Geometry3D geo = new Geometry3D(2, 2, 2, 0, 100, 100, 0, 0);
            VolumeDataNative volumeData = new VolumeDataNative(geo);
            Volume volume = new Volume(SubsurfaceKind.Sediment);
            PropertyVolumeData propVol = new PropertyVolumeData(prop, volumeData);
            volume.addPropertyVolumeData(propVol);

            // Marshal data to unmanaged memory
            int nrElems = 8;
            float[] data = new float[] { 0.5f, 0.5f, 0.5f, 0.1f, 0.1f, 0.1f, 0.3f, 0.24f };
            IntPtr myData = Marshal.AllocHGlobal(sizeof(float) * nrElems);
            Marshal.Copy(data, 0, myData, nrElems);
            volumeData.setData_IJK(myData);

            SnapShot snapShot = project.getSnapShots()[0];
            snapShot.setVolume(volume);

        }

        [TestMethod]
        public void ProjectCreate()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            int xmlVersionMjr = 2;
            int xmlVersionMnr = 1;
            Project project = new Project(name, description, team, version, mode, xmlVersionMjr, xmlVersionMnr);
            StringAssert.Equals(project.getName(), name);
            StringAssert.Equals(project.getDescription(), description);
            StringAssert.Equals(project.getTeam(), team);
            StringAssert.Equals(project.getProgramVersion(), version);
            Assert.AreEqual(project.getModelingMode(), mode);
            Assert.AreEqual(project.getXmlVersionMajor(), xmlVersionMjr);
            Assert.AreEqual(project.getXmlVersionMinor(), xmlVersionMnr);
        }

        [TestMethod]
        public void ProjectAddSnapShot()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            
            Project project = new Project(name, description, team, version, mode, 2,1);
            SnapShot snap = new SnapShot(2.5, SnapShotKind.SYSTEM, true);
            SnapShotList snapShotList = project.getSnapShots();
            Assert.AreEqual(snapShotList.Count, 0);
            project.addSnapShot(snap);
            Assert.AreEqual(snapShotList.Count, 1);
        }

        [TestMethod]
        public void ProjectAddProperty()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            
            Project project = new Project(name, description, team, version, mode, 2,1);
            const string propName = "Depth";
            const string cauldronName = "cDepth";
            const string userName = "uDepth";
            const string unit = "unit";
            PropertyType type = PropertyType.FormationProperty;
            PropertyAttribute attrib = PropertyAttribute.Continuous3DProperty;
            Property property = new Property(propName, userName, cauldronName, unit, type, attrib);
            PropertyList propertyList = project.getProperties();
            Assert.AreEqual(propertyList.Count, 0);
            project.addProperty(property);
            Assert.AreEqual(propertyList.Count, 1);
        }

        [TestMethod]
        public void ProjectAddFormation()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            
            Project project = new Project(name, description, team, version, mode, 2,1);
            uint kStart = 1;
            uint kEnd = 2;
            const string formationName = "formation";
            bool isSourceRock = true;
            bool isMobileLayer = true;
            Formation formation = new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer);
            FormationList formationList = project.getFormations();
            Assert.AreEqual(formationList.Count, 0);
            project.addFormation(formation);
            Assert.AreEqual(formationList.Count, 1);
        }

        [TestMethod]
        public void ProjectAddReservoir()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            
            Project project = new Project(name, description, team, version, mode, 2,1);
            uint kStart = 1;
            uint kEnd = 2;
            const string formationName = "formation";
            bool isSourceRock = true;
            bool isMobileLayer = true;
            Formation formation = new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer);
            const string reservoirName = "reservoir";
            Reservoir reservoir = new Reservoir(reservoirName, formation);
            ReservoirList reservoirList = project.getReservoirs();
            Assert.AreEqual(reservoirList.Count, 0);
            project.addReservoir(reservoir);
            Assert.AreEqual(reservoirList.Count, 1);
        }

        [TestMethod]
        public void ProjectFindProperty()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            
            Project project = new Project(name, description, team, version, mode, 2,1);
            const string propName = "Depth";
            const string cauldronName = "cDepth";
            const string userName = "uDepth";
            const string unit = "unit";
            PropertyType type = PropertyType.FormationProperty;
            PropertyAttribute attrib = PropertyAttribute.Continuous3DProperty;
            Property property = new Property(propName, userName, cauldronName, unit, type, attrib);
            project.addProperty(property);
            Property newProp = project.findProperty(propName);
            StringAssert.Equals(newProp.getName(), propName);
            StringAssert.Equals(newProp.getCauldronName(), cauldronName);
            StringAssert.Equals(newProp.getUserName(), userName);
            StringAssert.Equals(newProp.getUnit(), unit);
            Assert.AreEqual(newProp.getType(), type);
            Assert.AreEqual(newProp.getAttribute(), attrib);

        }

        [TestMethod]
        public void ProjectFindFormation()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            
            Project project = new Project(name, description, team, version, mode, 2,1);
            uint kStart = 1;
            uint kEnd = 2;
            const string formationName = "formation";
            bool isSourceRock = true;
            bool isMobileLayer = true;
            Formation formation = new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer);
            project.addFormation(formation);
            Formation newFormation = project.findFormation(formationName);
            uint start, end;
            newFormation.getK_Range(out start, out end);
            Assert.AreEqual(start, kStart);
            Assert.AreEqual(end, kEnd);
            StringAssert.Equals(newFormation.getName(), formationName);
            Assert.AreEqual(newFormation.isMobileLayer(), isMobileLayer);
            Assert.AreEqual(newFormation.isSourceRock(), isSourceRock);
        }

        [TestMethod]
        public void ProjectFindReservoir()
        {
            const string name = "Project";
            const string description = "desc";
            const string team = "team";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            
            Project project = new Project(name, description, team, version, mode, 2,1);
            uint kStart = 1;
            uint kEnd = 2;
            const string formationName = "formation";
            bool isSourceRock = true;
            bool isMobileLayer = true;
            Formation formation = new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer);
            const string reservoirName = "reservoir";
            Reservoir reservoir = new Reservoir(reservoirName, formation);
            project.addReservoir(reservoir);
            Reservoir newReservoir = project.findReservoir(reservoirName);
            StringAssert.Equals(newReservoir.getName(), reservoirName);
            StringAssert.Equals(newReservoir.getFormation().getName(), formationName);
            Assert.AreEqual(newReservoir.getFormation().isMobileLayer(), isMobileLayer);
            Assert.AreEqual(newReservoir.getFormation().isSourceRock(), isSourceRock);
            uint start, end;
            newReservoir.getFormation().getK_Range(out start, out end);
            Assert.AreEqual(start, kStart);
            Assert.AreEqual(end, kEnd);
        }
    }

    [TestClass()]
    public class SurfaceTests
    {
        [TestMethod]
        public void SurfaceCreate()
        {
            const string surfaceName = "waterbottom";
            SubsurfaceKind kind = SubsurfaceKind.Sediment;
            Surface surface = new Surface(surfaceName, kind);
            StringAssert.Equals(surface.getName(), surfaceName);
            Assert.AreEqual(surface.getSubSurfaceKind(), kind);
        }

        [TestMethod]
        public void SurfaceAddPropertySurfaceData()
        {
            const string surfaceName = "waterbottom";
            SubsurfaceKind kind = SubsurfaceKind.Sediment;
            Surface surface = new Surface(surfaceName, kind);
            const string propName = "Depth";
            const string cauldronName = "cDepth";
            const string userName = "uDepth";
            const string unit = "unit";
            PropertyType type = PropertyType.FormationProperty;
            PropertyAttribute attrib = PropertyAttribute.Continuous3DProperty;
            Property property = new Property(propName, userName, cauldronName, unit, type, attrib);

            PropertySurfaceData propertySurfaceData = new PropertySurfaceData();
            PropertySurfaceDataList propSurfaceDataList = surface.getPropertySurfaceDataList();
            Assert.AreEqual(propSurfaceDataList.Count, 0);
            surface.addPropertySurfaceData(propertySurfaceData);
            Assert.AreEqual(propSurfaceDataList.Count, 1);
        }

        [TestMethod]
        public void SurfaceSetFormation()
        {
            const string surfaceName = "waterbottom";
            SubsurfaceKind kind = SubsurfaceKind.Sediment;
            Surface surface = new Surface(surfaceName, kind);
            uint kStart = 1;
            uint kEnd = 2;
            const string formationName = "formation";
            bool isSourceRock = true;
            bool isMobileLayer = true;
            Formation formation = new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer);
            surface.setFormation(formation,true);
            Assert.AreEqual(surface.getTopFormation().getName(), formationName);
            uint start, end;
            surface.getTopFormation().getK_Range(out start, out end);
            Assert.AreEqual(surface.getTopFormation().isMobileLayer(), isMobileLayer);
            Assert.AreEqual(surface.getTopFormation().isSourceRock(), isSourceRock);
            Assert.AreEqual(start, kStart);
            Assert.AreEqual(end, kEnd);
        }

        [TestClass()]
        public class SnapShotTests
        {
            [TestMethod]
            public void SnapShotCreate()
            {
                double age = 2.5;
                SnapShotKind kind = SnapShotKind.SYSTEM;
                bool isMinorSnapShot = true;
                SnapShot snap = new SnapShot(age, kind, isMinorSnapShot);
                Assert.AreEqual(snap.getAge(), age);
                Assert.AreEqual(snap.getKind(), kind);
                Assert.AreEqual(snap.isMinorShapshot(), isMinorSnapShot);
            }

            [TestMethod]
            public void SnapShotAddSurface()
            {
                double age = 2.5;
                SnapShotKind kind = SnapShotKind.SYSTEM;
                bool isMinorSnapShot = true;
                SnapShot snap = new SnapShot(age, kind, isMinorSnapShot);
                const string surfaceName = "waterbottom";
                SubsurfaceKind subsurfaceKind = SubsurfaceKind.Sediment;
                Surface surface = new Surface(surfaceName, subsurfaceKind);
                SurfaceList surfaceList = snap.getSurfaceList();
                Assert.AreEqual(surfaceList.Count, 0);
                snap.addSurface(surface);
                Assert.AreEqual(surfaceList.Count, 1);
            }

            [TestMethod]
            public void SnapShotSetVolume()
            {
                double age = 2.5;
                SnapShotKind kind = SnapShotKind.SYSTEM;
                bool isMinorSnapShot = true;
                SnapShot snap = new SnapShot(age, kind, isMinorSnapShot);
                SubsurfaceKind subsurfaceKind = SubsurfaceKind.Sediment;
                Geometry3D geometry = new Geometry3D(2, 2, 2, 4, 100, 100, 0, 0);
                Volume volume = new Volume(subsurfaceKind);
                snap.setVolume(volume);
                Assert.AreEqual(snap.getVolume().getSubSurfaceKind(), subsurfaceKind);
            }

            [TestMethod]
            public void SnapShotAddFormationVolume()
            {
                double age = 2.5;
                SnapShotKind kind = SnapShotKind.SYSTEM;
                bool isMinorSnapShot = true;
                SnapShot snap = new SnapShot(age, kind, isMinorSnapShot);
                uint kStart = 1;
                uint kEnd = 2;
                const string formationName = "formation";
                bool isSourceRock = true;
                bool isMobileLayer = true;
                Formation formation = new Formation(kStart, kEnd, formationName, isSourceRock, isMobileLayer);
                SubsurfaceKind subsurfaceKind = SubsurfaceKind.Sediment;
                Geometry3D geometry = new Geometry3D(2, 2, 2, 4, 100, 100, 0, 0);
                Volume volume = new Volume(subsurfaceKind);
                FormationVolume formationVolume = new FormationVolume(formation, volume);
                FormationVolumeList formationVolumeList = snap.getFormationVolumeList();
                Assert.AreEqual(formationVolumeList.Count, 0);
                snap.addFormationVolume(formationVolume);
                Assert.AreEqual(formationVolumeList.Count, 1);
            }

            [TestMethod]
            public void SnapShotAddTrapper()
            {
                double age = 2.5;
                SnapShotKind kind = SnapShotKind.SYSTEM;
                bool isMinorSnapShot = true;
                SnapShot snap = new SnapShot(age, kind, isMinorSnapShot);
                int ID = 1234;
                int persistentID = 2345;
                Trapper trapper = new Trapper(ID, persistentID);
                TrapperList trapperList = snap.getTrapperList();
                Assert.AreEqual(trapperList.Count, 0);
                snap.addTrapper(trapper);
                Assert.AreEqual(trapperList.Count, 1);
            }
        }

        [TestClass()]
        public class TrapperTests
        {
            [TestMethod]
            public void TrapperCreate()
            {
                int ID = 1234;
                int persistentID = 2345;
                Trapper trapper = new Trapper(ID, persistentID);
                Assert.AreEqual(trapper.getID(), ID);
                Assert.AreEqual(trapper.getPersistentID(), persistentID);
            }

            [TestMethod]
            public void TrapperSetReservoirName()
            {
                int ID = 1234;
                int persistentID = 2345;
                const string reservoirName = "reservoir";
                Trapper trapper = new Trapper(ID, persistentID);
                trapper.setReservoirName(reservoirName);
                StringAssert.Equals(trapper.getReservoirName(), reservoirName);

            }


            [TestMethod]
            public void TrapperSetSpillDepth()
            {
                int ID = 1234;
                int persistentID = 2345;
                float depth = 1.5f;
                Trapper trapper = new Trapper(ID, persistentID);
                trapper.setSpillDepth(depth);
                Assert.AreEqual(trapper.getSpillDepth(), depth);

            }

            [TestMethod]
            public void TrapperSetDepth()
            {
                int ID = 1234;
                int persistentID = 2345;
                float depth = 1.5f;
                Trapper trapper = new Trapper(ID, persistentID);
                trapper.setDepth(depth);
                Assert.AreEqual(trapper.getDepth(), depth);

            }

            [TestMethod]
            public void TrapperSetPosition()
            {
                int ID = 1234;
                int persistentID = 2345;
                float posX = 1.5f;
                float posY = 2.5f;
                Trapper trapper = new Trapper(ID, persistentID);
                trapper.setPosition(posX, posY);
                float checkX, checkY;
                trapper.getPosition(out checkX, out checkY);
                Assert.AreEqual(checkX, posX);
                Assert.AreEqual(checkY, posY);

            }

            [TestMethod]
            public void TrapperSetSpillPointPosition()
            {
                int ID = 1234;
                int persistentID = 2345;
                float posX = 1.5f;
                float posY = 2.5f;
                Trapper trapper = new Trapper(ID, persistentID);
                trapper.setSpillPointPosition(posX, posY);
                float checkX, checkY;
                trapper.getSpillPointPosition(out checkX, out checkY);
                Assert.AreEqual(checkX, posX);
                Assert.AreEqual(checkY, posY);

            }

            [TestMethod]
            public void TrapperSetDownStreamTrapper()
            {

                int ID = 1234;
                int persistentID = 2345;
                Trapper trapper = new Trapper(ID, persistentID);
                int downID = 3456;
                int downPersistentID = 4567;
                Trapper downstreamTrapper = new Trapper(downID, downPersistentID);
                trapper.setDownStreamTrapper(downstreamTrapper);
                Assert.AreEqual(trapper.getDownStreamTrapper().getID(), downID);
                Assert.AreEqual(trapper.getDownStreamTrapper().getPersistentID(), downPersistentID);
            }

            [TestMethod]
            public void TrapperSetDownStreamTrapperID()
            {
                int ID = 1234;
                int persistentID = 2345;
                Trapper trapper = new Trapper(ID, persistentID);
                int downID = 3456;
                int downPersistentID = 4567;
                Trapper downstreamTrapper = new Trapper(downID, downPersistentID);
                trapper.setDownStreamTrapper(downstreamTrapper);
                trapper.setDownStreamTrapperID(persistentID);
                Assert.AreEqual(trapper.getDownStreamTrapperID(), persistentID);
            }
        }

        [TestClass()]
        public class MapTests
        {
            [TestMethod]
            public void MapSetUUID()
            {

            }

            [TestMethod]
            public void MapSetData_IJ()
            {
            }
        }
    }
}

