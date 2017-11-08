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
using Bpa2.Basin.CauldronIO;
using System.Threading;


namespace Shell.BasinModeling.CauldronIO.Test
{
    [TestClass()]
    public class ProjectTests
    {
        [TestMethod]
        public void CheckThreadSafety()
        {
            // TODO: create a class loading data from one project using multiple threads
        }

        [TestMethod]
        public void LoadProjectNative()
        {
            Project project = ImportFromXML.importFromXML("../../../csharp-test/HydrostaticGL_out.xml");
            int count = project.getSnapShots().Count;
            Assert.IsTrue(count == 14);
            count = project.getProperties().Count;
            Assert.IsTrue(count == 23);
            count = project.getFormations().Count;
            Assert.IsTrue(count == 4);
            count = project.getReservoirs().Count;
            Assert.IsTrue(count == 0);
            count = project.getSnapShots()[0].getFormationVolumeList().Count;
            Assert.IsTrue(count == 4);
            count = project.getSnapShots()[0].getVolume().getPropertyVolumeDataList().Count;
            Assert.IsTrue(count == 10);
            string name = project.getSnapShots()[0].getSurfaceList()[0].getBottomFormation().getName();
            StringAssert.Equals(name, "Erosion");
            Assert.IsTrue(project.getSnapShots()[0].getSurfaceList()[1].getSubSurfaceKind() == SubsurfaceKind.Sediment);
            Assert.IsFalse(project.getSnapShots()[13].isMinorShapshot());
            Assert.IsTrue(project.getProperties()[3].getType() == PropertyType.FormationProperty);
            Assert.IsTrue(project.getProperties()[3].getAttribute() == 0);
            StringAssert.Equals(project.getProperties()[3].getUnit(), "C");
            StringAssert.Equals(project.getProperties()[3].getCauldronName(), "Temperature");
            StringAssert.Equals(project.getProperties()[3].getUserName(), "Temperature");
            Assert.IsTrue(project.getSnapShots()[0].getVolume().getSubSurfaceKind() == SubsurfaceKind.None);

            Formation form = project.findFormation("Mantle");
            int kStart, kEnd;
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

        [TestMethod]
        public void ProjectCreate()
        {
            const string name = "Project";
            const string description = "desc";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            int xmlVersionMjr = 2;
            int xmlVersionMnr = 1;
            Project project = new Project(name, description, version, mode, xmlVersionMjr, xmlVersionMnr);
            StringAssert.Equals(project.getName(), name);
            StringAssert.Equals(project.getDescription(), description);
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
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;

            Project project = new Project(name, description, version, mode, 2, 1);
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
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;

            Project project = new Project(name, description, version, mode, 2, 1);
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
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;

            Project project = new Project(name, description, version, mode, 2, 1);
            int kStart = 1;
            int kEnd = 2;
            const string formationName = "formation";
            Formation formation = new Formation(kStart, kEnd, formationName);
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
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;

            Project project = new Project(name, description, version, mode, 2, 1);
            int kStart = 1;
            int kEnd = 2;
            const string formationName = "formation";
            Formation formation = new Formation(kStart, kEnd, formationName);
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
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;

            Project project = new Project(name, description, version, mode, 2, 1);
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
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;

            Project project = new Project(name, description, version, mode, 2, 1);
            int kStart = 1;
            int kEnd = 2;
            const string formationName = "formation";
            Formation formation = new Formation(kStart, kEnd, formationName);
            project.addFormation(formation);
            Formation newFormation = project.findFormation(formationName);
            int start, end;
            newFormation.getK_Range(out start, out end);
            Assert.AreEqual(start, kStart);
            Assert.AreEqual(end, kEnd);
            StringAssert.Equals(newFormation.getName(), formationName);
        }

        [TestMethod]
        public void ProjectFindReservoir()
        {
            const string name = "Project";
            const string description = "desc";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;

            Project project = new Project(name, description, version, mode, 2, 1);
            int kStart = 1;
            int kEnd = 2;
            const string formationName = "formation";
            Formation formation = new Formation(kStart, kEnd, formationName);
            const string reservoirName = "reservoir";
            Reservoir reservoir = new Reservoir(reservoirName, formation);
            project.addReservoir(reservoir);
            Reservoir newReservoir = project.findReservoir(reservoirName);
            StringAssert.Equals(newReservoir.getName(), reservoirName);
            StringAssert.Equals(newReservoir.getFormation().getName(), formationName);
            int start, end;
            newReservoir.getFormation().getK_Range(out start, out end);
            Assert.AreEqual(start, kStart);
            Assert.AreEqual(end, kEnd);
        }

        [TestMethod]
        public void ProjectAddHistoryAndMassBalanceFile()
        {
            const string name = "Project";
            const string description = "desc";
            const string version = "vers";
            ModellingMode mode = ModellingMode.MODE1D;
            int xmlVersionMjr = 2;
            int xmlVersionMnr = 1;
            Project project = new Project(name, description, version, mode, xmlVersionMjr, xmlVersionMnr);

            const string historyFileName = "HistoryFile.dat";
            project.addGenexHistoryRecord(historyFileName);
            StringVector filesList = project.getGenexHistoryList();
            Assert.AreEqual(1, filesList.Count);
            StringAssert.Equals(historyFileName, filesList[0]);

            const string burialHistoryFileName = "MyWell.BHF";
            project.addBurialHistoryRecord(burialHistoryFileName);
            StringVector burialFilesList = project.getBurialHistoryList();
            Assert.AreEqual(1, burialFilesList.Count);
            StringAssert.Equals(burialHistoryFileName, burialFilesList[0]);

            const string massBalanceFileName = "project_MassBalance";
            project.setMassBalance(massBalanceFileName);
            StringAssert.Equals(project.getMassBalance(), massBalanceFileName);
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
            int kStart = 1;
            int kEnd = 2;
            const string formationName = "formation";
            Formation formation = new Formation(kStart, kEnd, formationName);
            surface.setFormation(formation, true);
            Assert.AreEqual(surface.getTopFormation().getName(), formationName);
            int start, end;
            surface.getTopFormation().getK_Range(out start, out end);
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
                int kStart = 1;
                int kEnd = 2;
                const string formationName = "formation";
                Formation formation = new Formation(kStart, kEnd, formationName);
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

            [TestMethod]
            public void SnapShotNegativeAge()
            {
                double age = -2.5;
                SnapShotKind kind = SnapShotKind.SYSTEM;
                bool isMinorSnapShot = true;

                // This should give an exception
                try
                {
                    SnapShot snap = new SnapShot(age, kind, isMinorSnapShot);
                    Assert.Fail();
                }
                catch (System.ApplicationException e)
                {
                    Assert.IsTrue(e.ToString().Contains("C++ CauldronIO::CauldronIOException exception thrown"));
                }
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

            [TestMethod]
            public void TrapperSetSolutionMass()
            {
                int ID = 1234;
                int persistentID = 2345;
                Trapper trapper = new Trapper(ID, persistentID);
                double solutionMassC6_14Aro = 2682893.23;
                trapper.setSolutionGasMass(solutionMassC6_14Aro, SpeciesNamesId.C6_MINUS_14ARO);
                Assert.AreEqual(trapper.getSolutionGasMass(SpeciesNamesId.C6_MINUS_14ARO), solutionMassC6_14Aro);
                Assert.AreEqual(trapper.getSolutionGasMass(SpeciesNamesId.C3), 0.0);
            }
        }
    }
    [TestClass]
    public class TrapTests
    {
        [TestMethod]
        public void TrapCreate()
        {
            int ID = 12345;
            Trap trap = new Trap(ID);
            Assert.AreEqual(trap.getID(), ID);
        }

        [TestMethod]
        public void TrapSetReservoirName()
        {
            int ID = 1234;
            const string reservoirName = "reservoir";
            Trap trap = new Trap(ID);
            trap.setReservoirName(reservoirName);
            StringAssert.Equals(trap.getReservoirName(), reservoirName);

        }


        [TestMethod]
        public void TrapSetSpillDepth()
        {
            int ID = 1234;
            float depth = 1.5f;
            Trap trap = new Trap(ID);
            trap.setSpillDepth(depth);
            Assert.AreEqual(trap.getSpillDepth(), depth);

        }

        [TestMethod]
        public void TrapSetDepth()
        {
            int ID = 1234;
            float depth = 1.5f;
            Trap trap = new Trap(ID);
            trap.setDepth(depth);
            Assert.AreEqual(trap.getDepth(), depth);

        }

        [TestMethod]
        public void TrapSetPressure()
        {
            int ID = 1234;
            float pressure = 30.511699f;
            Trap trap = new Trap(ID);
            trap.setPressure(pressure);
            Assert.AreEqual(trap.getPressure(), pressure);
        }

        [TestMethod]
        public void TrapSetNetToGross()
        {
            int ID = 1234;
            float net2gross = 100.0f;
            Trap trap = new Trap(ID);
            trap.setNetToGross(net2gross);
            Assert.AreEqual(trap.getNetToGross(), net2gross);
        }

        [TestMethod]
        public void TrapSetMass()
        {
            int ID = 1234;
            Trap trap = new Trap(ID);
            double C1mass = 0.005678;
            trap.setMass(C1mass, SpeciesNamesId.C1);
            Assert.AreEqual(trap.getMass(SpeciesNamesId.C1), C1mass);
        }

        [TestMethod]
        public void TrapSetSpillPointPosition()
        {
            int ID = 1234;

            float posX = 1.5f;
            float posY = 2.5f;
            Trap trap = new Trap(ID);
            trap.setSpillPointPosition(posX, posY);
            float checkX, checkY;
            trap.getSpillPointPosition(out checkX, out checkY);
            Assert.AreEqual(checkX, posX);
            Assert.AreEqual(checkY, posY);

        }

    }

    [TestClass]
    public class Tables1DTest
    {
        [TestMethod]
        public void DisplayContourTest()
        {
            const string propertyName = "Temperature";
            DisplayContour dp = new DisplayContour();
            dp.setPropertyName(propertyName);
            StringAssert.Equals(dp.getPropertyName(), propertyName);

            float val = 122.5f;
            dp.setContourValue(val);
            Assert.AreEqual(val, dp.getContourValue());
        }
        [TestMethod]
        public void BiomarkermTest()
        {
            Biomarkerm biom = new Biomarkerm();
            biom.setDepthIndex(345.0f);
            Assert.AreEqual(345.0f, biom.getDepthIndex());
            biom.setOptimization(true);
            Assert.IsTrue(biom.getOptimization());
        }
        [TestMethod]
        public void IsoEntryTest()
        {
            IsoEntry iso = new IsoEntry();
            iso.setAge(50.0f);
            Assert.AreEqual(50.0f, iso.getAge());
            iso.setSum(6789.01);
            Assert.AreEqual(6789.01, iso.getSum());
        }
        [TestMethod]
        public void FtSampleTest()
        {
            FtSample fiss = new FtSample();

            StringAssert.Equals("", fiss.getFtSampleId());
            const string sampleId = "sampleID";

            fiss.setFtSampleId(sampleId);
            StringAssert.Equals(sampleId, fiss.getFtSampleId());

            fiss.setFtMeanRatio(0.54f);
            Assert.AreEqual(0.54f, fiss.getFtMeanRatio());

        }
        [TestMethod]
        public void FtGrainTest()
        {
            FtGrain fgrain = new FtGrain();

            StringAssert.Equals("", fgrain.getFtSampleId());
            const string sampleId = "sampleID";

            fgrain.setFtSampleId(sampleId);
            StringAssert.Equals(sampleId, fgrain.getFtSampleId());

            fgrain.setFtInducedTrackNo(23);
            Assert.AreEqual(23, fgrain.getFtInducedTrackNo());
        }
        [TestMethod]
        public void FtPredLengthCountsHistTest()
        {
            FtPredLengthCountsHist fpred = new FtPredLengthCountsHist();

            Assert.AreEqual(0, fpred.getFtPredLengthHistId());
            fpred.setFtPredLengthHistId(21);
            Assert.AreEqual(21, fpred.getFtPredLengthHistId());

            fpred.setFtPredLengthBinWidth(45.0f);
            Assert.AreEqual(45.0f, fpred.getFtPredLengthBinWidth());
        }
        [TestMethod]
        public void FtPredLengthCountsHistDataTest()
        {
            FtPredLengthCountsHistData fdata = new FtPredLengthCountsHistData();

            Assert.AreEqual(0.0f, fdata.getFtPredLengthBinCount());
            fdata.setFtPredLengthBinCount(222.0f);
            Assert.AreEqual(222.0f, fdata.getFtPredLengthBinCount());

            fdata.setFtPredLengthBinIndex(3);
            Assert.AreEqual(3, fdata.getFtPredLengthBinIndex());

            fdata.setFtPredLengthHistId(-1);
            Assert.AreEqual(-1, fdata.getFtPredLengthHistId());
        }
        [TestMethod]
        public void FtClWeightPercBinsTest()
        {
            FtClWeightPercBins ftcl = new FtClWeightPercBins();

            ftcl.setFtClWeightBinStart(45.5768);
            Assert.AreEqual(45.5768, ftcl.getFtClWeightBinStart());
            ftcl.setFtClWeightBinWidth(0.9456976);
            Assert.AreEqual(0.9456976, ftcl.getFtClWeightBinWidth());
        }
        [TestMethod]
        public void SmectiteIlliteTest()
        {
            SmectiteIllite smect = new SmectiteIllite();

            StringAssert.Equals("", smect.getLabel());
            const string label = "Label";
            smect.setLabel(label);
            StringAssert.Equals(label, smect.getLabel());

            smect.setDepthIndex(356.078f);
            Assert.AreEqual(356.078f, smect.getDepthIndex());

            Assert.IsFalse(smect.getOptimization());
            smect.setOptimization(true);
            Assert.IsTrue(smect.getOptimization());

        }
        [TestMethod]
        public void DepthIoTest()
        {
            DepthIo depth = new DepthIo();

            StringAssert.Equals("", depth.getPropertyName());
            const string propertyName = "Vr";
            depth.setPropertyName(propertyName);
            StringAssert.Equals(propertyName, depth.getPropertyName());

            depth.setMinimum(0.00000123f);
            Assert.AreEqual(0.00000123f, depth.getMinimum());

            Assert.AreEqual(0.0f, depth.getMaximum());

            depth.setP85(345.098f);
            Assert.AreEqual(345.098f, depth.getP85());

            depth.setNP(34);
            Assert.AreEqual(34, depth.getNP());

            depth.setKurtosis(0.5f);
            Assert.AreEqual(0.5f, depth.getKurtosis());
        }
        [TestMethod]
        public void TimeIo1DTest()
        {
            TimeIo1D time1d = new TimeIo1D();

            const string name = "Surface";
            time1d.setSurfaceName(name);
            StringAssert.Equals(name, time1d.getSurfaceName());

            const string propertyName = "Temperature";
            StringAssert.Equals("", time1d.getPropertyName());
            time1d.setPropertyName(propertyName);
            StringAssert.Equals(propertyName, time1d.getPropertyName());

            time1d.setValue(123.04f);
            Assert.AreEqual(123.04f, time1d.getValue());
        }
    }

    [TestClass]
    public class DataDrillerTest
    {
        [TestMethod]
        public void DrillData()
        {
            const string name = "Project";
            const string description = "desc"; 
            const string version = "vers";

            ModellingMode mode = ModellingMode.MODE3D;
            int xmlVersionMjr = 2;
            int xmlVersionMnr = 1;
            Project project = new Project(name, description, version, mode, xmlVersionMjr, xmlVersionMnr);
            SnapShot snap = new SnapShot(2.5, SnapShotKind.SYSTEM, true);
            project.addSnapShot(snap);

            string propName = "Depth";
            string cauldronName = "cDepth";
            string userName = "uDepth";
            string unit = "m";
            PropertyType type = PropertyType.FormationProperty;
            PropertyAttribute attrib = PropertyAttribute.Continuous3DProperty;
            Property depthProperty = new Property(propName, userName, cauldronName, unit, type, attrib);
            project.addProperty(depthProperty);

            propName = "Temperature";
            cauldronName = propName;
            userName = propName;
            unit = "C";
            type = PropertyType.FormationProperty;
            Property tempProperty = new Property(propName, userName, cauldronName, unit, type, attrib);
            project.addProperty(tempProperty);

            propName = "Vr";
            cauldronName = propName;
            userName = propName;
            unit = "%";
            type = PropertyType.FormationProperty;
            attrib = PropertyAttribute.Discontinuous3DProperty;
            Property vrProperty = new Property(propName, userName, cauldronName, unit, type, attrib);
            project.addProperty(vrProperty);

            string surfaceName = "waterbottom";
            SubsurfaceKind kind = SubsurfaceKind.Sediment;
            Surface surface = new Surface(surfaceName, kind);

            surfaceName = "s2";
            kind = SubsurfaceKind.Sediment;
            Surface surface2 = new Surface(surfaceName, kind);

            int kStart = 0;
            int kEnd = 4;
            string formationName = "formation1";
            Formation formation = new Formation(kStart, kEnd, formationName);
            formation.setTopSurface(surface);
            formation.setBottomSurface(surface2);
            project.addFormation(formation);
 
            surfaceName = "s3";
            kind = SubsurfaceKind.Sediment;
            Surface surface3 = new Surface(surfaceName, kind);
 
            kStart = 4;
            kEnd = 6;
            formationName = "formation2";
            Formation formation2 = new Formation(kStart, kEnd, formationName);
            formation2.setTopSurface(surface2);
            formation2.setBottomSurface(surface3);
            project.addFormation(formation2);

            SubsurfaceKind subsurfaceKind = SubsurfaceKind.Sediment;
            Geometry3D geometry = new Geometry3D(2, 2, 6, 0, 100, 100, 0, 0);

            // add formation volume Vr
            Volume volume = new Volume(subsurfaceKind);
            FormationVolume formationVolume = new FormationVolume(formation, volume);
            Geometry3D geometry1 = new Geometry3D(2, 2, 4, 0, 100, 100, 0, 0);

            VolumeData propertyVrData = new VolumeDataNative(geometry1);
            float [] vrvalues = new float[16];
            uint kk;
            for(uint i = 0; i < 2; ++i) {
                for (uint j = 0; j < 2; ++j) {
                    for (uint k = 0; k < 4; ++k) {
                        kk = propertyVrData.computeIndex_IJK(i, j, k);
                        vrvalues[kk] = 0.5F;
                    }
                }
            }

            IntPtr vrvec = Marshal.AllocHGlobal(4 * 16);
            Marshal.Copy(vrvalues, 0, vrvec, vrvalues.Length);

            propertyVrData.setData_IJK( vrvec );
            PropertyVolumeData pdata = new PropertyVolumeData(vrProperty, propertyVrData);
            volume.addPropertyVolumeData(pdata);
            snap.addFormationVolume(formationVolume);

            // add formation volume Vr
            Volume volume2 = new Volume(subsurfaceKind);
            FormationVolume formationVolume2 = new FormationVolume(formation2, volume2);
            Geometry3D geometry2 = new Geometry3D(2, 2, 6, 4, 100, 100, 0, 0);

            VolumeData propertyVrData2 = new VolumeDataNative(geometry2);
            float[] vrvalues2 = new float[8];

            for (uint i = 0; i < 2; ++i)
            {
                for (uint j = 0; j < 2; ++j)
                {
                    for (uint k = 4; k < 6; ++k)
                    {
                        kk = propertyVrData2.computeIndex_IJK(i, j, k);
                        vrvalues2[kk] = 0.6F;
                    }
                }
            }

            IntPtr vrvec2 = Marshal.AllocHGlobal(4 * 8);
            Marshal.Copy(vrvalues2, 0, vrvec2, vrvalues2.Length);

            propertyVrData2.setData_IJK(vrvec2);
            PropertyVolumeData pdata2 = new PropertyVolumeData(vrProperty, propertyVrData2);
            volume2.addPropertyVolumeData(pdata2);
            snap.addFormationVolume(formationVolume2);

            // add snapshot volume Depth
            Volume snapshotVolume = new Volume(kind);
            VolumeData snapshotDepthData = new VolumeDataNative(geometry);
            float[] depthvalues = new float[24];

            float v = 0;
            for (uint i = 0; i < 2; ++i) {
                for (uint j = 0; j < 2; ++j)  {
                    for (uint k = 0; k < 6; ++k)
                    {
                        kk = snapshotDepthData.computeIndex_IJK(i, j, k);

                        depthvalues[kk] = v;
                        v = v + 100;
                    }
                }
            }

            IntPtr depthvec = Marshal.AllocHGlobal(4 * 24);
            Marshal.Copy(depthvalues, 0, depthvec, depthvalues.Length);
            snapshotDepthData.setData_IJK(depthvec);
            PropertyVolumeData volumeDepthPropertyData = new PropertyVolumeData(depthProperty, snapshotDepthData);
            snapshotVolume.addPropertyVolumeData(volumeDepthPropertyData);

            // add snapshot volume Temperature
            VolumeData snapshotDataTemp = new VolumeDataNative(geometry);
            float[] tempvalues = new float[24];

            v = 20;
            for (uint i = 0; i < 2; ++i)  {
                for (uint j = 0; j < 2; ++j) {
                    for (uint k = 0; k < 6; ++k)  {
                        kk = snapshotDataTemp.computeIndex_IJK(i, j, k);

                        tempvalues[kk] = v;
                        v = v + 100;
                    }
                }
            }

            IntPtr tempvec = Marshal.AllocHGlobal(4 * 24);
            Marshal.Copy(tempvalues, 0, tempvec, tempvalues.Length);
            snapshotDataTemp.setData_IJK(tempvec);
            PropertyVolumeData volumePropertyTempData = new PropertyVolumeData(tempProperty, snapshotDataTemp);
            snapshotVolume.addPropertyVolumeData(volumePropertyTempData);
            
            snap.setVolume(snapshotVolume);
 
            float value = project.getPropertyAtLocation(2.5, "Temperature", 0, 0, 0, "", "", "");
            Assert.AreEqual(20.0, value);
            value = project.getPropertyAtLocation(2.5, "Temperature", 0, 0, 500, "", "", "");
            Assert.AreEqual(520.0, value);
            
            value = project.getPropertyAtLocation(2.5, "Temperature", 0, 0, Bpa2.Basin.CauldronIO.VisualizationIOAPI.DefaultUndefinedScalarValue, "", "s2", "");
            Assert.AreEqual(420.0, value);
            value = project.getPropertyAtLocation(2.5, "Vr", 0, 0, Bpa2.Basin.CauldronIO.VisualizationIOAPI.DefaultUndefinedScalarValue, "", "s2", "");
            Assert.AreEqual(0.6F, value);
        }

      }
}



