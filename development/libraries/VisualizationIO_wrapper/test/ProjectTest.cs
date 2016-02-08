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
            Project project = ImportExport.importFromXML("../../../csharp-test/cauldron_outputs.xml");
            int count = project.getSnapShots().Count;
            Assert.IsTrue(count == 14);
            count = project.getSurfaceNames().Count;
            Assert.IsTrue(count == 5);
        }

        [TestMethod]
        public void AddToProjectNative()
        {
            Project project = ImportExport.importFromXML("../../../csharp-test/cauldron_outputs.xml");
            int count = project.getSnapShots().Count;
            Assert.IsTrue(count == 14);

            // Create a new volume
            string propName = "Depth";
            string unit = "m";
            Property prop = new Property(propName, propName, propName, unit, PropertyType.FormationProperty, PropertyAttribute.Continuous3DProperty);
            Geometry3D geo = new Geometry3D(2, 2, 2, 0, 100, 100, 0, 0);
            VolumeDataNative volumeData = new VolumeDataNative(geo);
            Volume volume = new Volume(SubsurfaceKind.Sediment, geo);
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
        public void RetrieveRelease()
        {
            Project project = ImportExport.importFromXML("../../../csharp-test/cauldron_outputs.xml");

            project.retrieve();
            project.release();
            project.retrieve();
        }
    }
}
