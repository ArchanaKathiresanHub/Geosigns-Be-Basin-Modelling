/*******************************************************************
 * 
 * Copyright (C) 2013 Shell International Exploration & Production.
 * All rights reserved.
 * 
 * Developed under licence for Shell by PDS BV.
 * 
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell.
 * 
 *******************************************************************/

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Shell.BasinModeling.Cauldron;


namespace BasinModelingLinkTest
{
    [TestClass]
    public class SerialDataAccessTest
    {
        [TestMethod]
        public void OpenProject3DTest()
        {
            string pathToProject3D = @"..\..\..\csharp-test\Project.project3d";
            ProjectHandle projectHandle = CauldronAPI.OpenCauldronProject(pathToProject3D.Replace("\\", "/"), "r");

            Assert.IsNotNull(projectHandle, "Could not open project file");

            FormationList formations = projectHandle.getFormations();
            Assert.IsTrue(formations.Count == 1, "Not all formations loaded");

            SurfaceList surfaces = projectHandle.getSurfaces();
            Assert.IsTrue(surfaces.Count == 2, "Not all surfaces loaded");

            // Get all possible properties
            PropertyList properties = projectHandle.getProperties(true);
            Assert.IsTrue(properties.Count == 402, "Not all possible properties loaded");

            // Get all defined properties
            properties = projectHandle.getProperties();
            Assert.IsTrue(properties.Count == 0, "An project without results doesn't contain any properties");
        }
    }
}
