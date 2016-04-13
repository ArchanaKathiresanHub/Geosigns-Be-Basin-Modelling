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
using System.Threading;


namespace Shell.BasinModeling.CauldronIO.Test
{
    /// <summary>
    ///This is a test class for suaAPI
    ///</summary>
    ///
    public class ThreadSafeChecker
    {
        public void OpenProject()
        {
            string fileName =  "../../../csharp-test/HydrostaticGL_out.xml";
            Project project = ImportExport.importFromXML(fileName);
            
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
            Assert.IsTrue(project.getSnapShots()[0].getSurfaceList()[1].getSubSurfaceKind() == SubsurfaceKind.Sediment);
            Assert.IsFalse(project.getSnapShots()[13].isMinorShapshot());
            
            Assert.IsTrue(project.getProperties()[3].getType() == PropertyType.FormationProperty);
            Assert.IsTrue(project.getProperties()[3].getAttribute() == 0);
            StringAssert.Equals(project.getProperties()[3].getUnit(), "C");
            StringAssert.Equals(project.getProperties()[3].getCauldronName(), "Temperature");
            StringAssert.Equals(project.getProperties()[3].getUserName(), "Temperature");
            
            Assert.IsTrue(project.getSnapShots()[0].getVolume().getSubSurfaceKind() == SubsurfaceKind.None);

            Formation form = project.findFormation("Mantle");
            uint kStart, kEnd;
            form.getK_Range(out kStart, out kEnd);
            Assert.IsFalse(form.isSourceRock());
            Assert.IsFalse(form.isMobileLayer());
            Assert.IsTrue(kStart == 12);
            Assert.IsTrue(kEnd == 17);

            Property prop = project.findProperty("ErosionFactor");
            Assert.IsTrue(prop.getAttribute() == PropertyAttribute.Formation2DProperty);
            Assert.IsTrue(prop.getType() == PropertyType.FormationProperty);
            Assert.IsTrue(prop.getUnit() == "");
            Assert.IsTrue(prop.getUserName() == "ErosionFactor");
            Assert.IsTrue(prop.getCauldronName() == "ErosionFactor");
        }
    }
    [TestClass()]
    public class ThreadSafeTests 
    {

        [TestMethod]
        public void CheckThreadSafety()
        {
            ThreadSafeChecker test = new ThreadSafeChecker();
            ThreadStart job = new ThreadStart(test.OpenProject);
            Thread thread = new Thread(job);
            thread.Start();
                      
            ThreadSafeChecker test2 = new ThreadSafeChecker();
            ThreadStart job2 = new ThreadStart(test2.OpenProject);
            Thread thread2 = new Thread(job2);
            thread2.Start();
            
            thread.Join();
            thread2.Join();
            
        }
    }
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
            Assert.IsTrue(project.getSnapShots()[0].getSurfaceList()[1].getSubSurfaceKind() == SubsurfaceKind.Sediment);
            Assert.IsFalse(project.getSnapShots()[13].isMinorShapshot());
            Assert.IsTrue(project.getProperties()[3].getType() == PropertyType.FormationProperty);
            Assert.IsTrue(project.getProperties()[3].getAttribute() == 0);
            StringAssert.Equals(project.getProperties()[3].getUnit(), "C");
            StringAssert.Equals(project.getProperties()[3].getCauldronName(), "Temperature");
            StringAssert.Equals(project.getProperties()[3].getUserName(), "Temperature");
            Assert.IsTrue(project.getSnapShots()[0].getVolume().getSubSurfaceKind() == SubsurfaceKind.None);
            
            Formation form = project.findFormation("Mantle");
            uint kStart, kEnd;
            form.getK_Range(out kStart, out kEnd);
            Assert.IsFalse(form.isSourceRock());
            Assert.IsFalse(form.isMobileLayer());
            Assert.IsTrue(kStart == 12);
            Assert.IsTrue(kEnd == 17);

            Property prop = project.findProperty("ErosionFactor");
            Assert.IsTrue(prop.getAttribute() == PropertyAttribute.Formation2DProperty);
            Assert.IsTrue(prop.getType() == PropertyType.FormationProperty);
            Assert.IsTrue(prop.getUnit() == "");
            Assert.IsTrue(prop.getUserName() == "ErosionFactor");
            Assert.IsTrue(prop.getCauldronName() == "ErosionFactor");
        }
        /*
        [TestMethod]
        public void AddToProjectNative()
        {
            Project project = ImportExport.importFromXML("../../../csharp-test/HydrostaticGL_out.xml");
            int count = project.getSnapShots().Count;
            Assert.IsTrue(count == 14);

            //Add a new property
            Property property = new Property("TestProperty", "User", "Clrdrn", "u", PropertyType.ReservoirProperty, PropertyAttribute.Continuous3DProperty);
            project.addProperty(property);

            //Add a new reservoir
            Formation formation = new Formation(2, 3, "TestFormation", false, false);
            Reservoir reservoir = new Reservoir("Reservoir1", formation);
            project.addReservoir(reservoir);

            //Add a new formation
            Formation testFormation = new Formation(5, 6, "Core", false, true);
            project.addFormation(testFormation);

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
            //Add new snapshot
            SnapShot snap = new SnapShot(25.7, SnapShotKind.SYSTEM, true);

            //Add new formation volume
            Formation form = new Formation(1, 2, "formation1", true, true);
            Geometry3D geometry = new Geometry3D(1, 1, 2, 2, 10, 10, 5, 5);
            Volume vol = new Volume(SubsurfaceKind.Sediment);
            FormationVolume formVolume = new FormationVolume(form, vol);
            snap.addFormationVolume(formVolume);

            //Add new surface
            Surface surface = new Surface("TestSurface", SubsurfaceKind.Basement);
            Geometry2D geo2D = new Geometry2D(2, 2, 10, 10, 1, 1);
            surface.setHighResGeometry(geo2D);
            snap.addSurface(surface);

            //Add new trapper
            Trapper trapper = new Trapper(123, 234);
            trapper.setDepth(1.5f);
            trapper.setReservoirName("reservoir");
            trapper.setPosition(1.2f, 1.5f);
            trapper.setSpillDepth(1.6f);
            trapper.setSpillPointPosition(1.1f, 2.2f);
            Trapper trap = new Trapper(234, 456);
            trapper.setDownStreamTrapper(trap);
            trapper.setDownStreamTrapperID(789);
            snap.addTrapper(trapper);
            project.addSnapShot(snap);

            ImportExport.exportToXML(project, "..\\..\\", "..\\..\\", "testfile1.xml", 10);
        }
        [TestMethod]
        public void VerifyAddedDataNative()
        {      
            //Checking the data added
            Project project = ImportExport.importFromXML("..\\..\\testfile1.xml");
            
            Property prop = project.findProperty("TestProperty");
            StringAssert.Equals("User", prop.getUserName());
            StringAssert.Equals("Cldrn", prop.getCauldronName());
            StringAssert.Equals("u", prop.getUnit());
            Assert.Equals(PropertyType.ReservoirProperty, prop.getType());
            Assert.Equals(PropertyAttribute.Continuous3DProperty, prop.getAttribute());

            Formation formation = project.findFormation("Core");
            uint kStart,kEnd;
            formation.getK_Range(out kStart, out kEnd);
            Assert.Equals(5, kStart);
            Assert.Equals(6, kEnd);
            Assert.IsFalse(formation.isSourceRock());
            Assert.IsTrue(formation.isMobileLayer());

            Reservoir reservoir = project.findReservoir("Reservoir1");
            formation = reservoir.getFormation();
            StringAssert.Equals("TestFormation", formation.getName());
            formation.getK_Range(out kStart, out kEnd);
            Assert.Equals(2, kStart);
            Assert.Equals(3, kEnd);
            Assert.IsFalse(formation.isSourceRock());
            Assert.IsFalse(formation.isMobileLayer());
        }
        */

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

    }
}

