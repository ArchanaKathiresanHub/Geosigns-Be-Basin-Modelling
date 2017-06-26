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
using Shell.BasinModeling.DataAccess;

namespace Shell.BasinModeling.DataAccess.Test
{
    [TestClass]
    public class SerialDataAccessTest
    {
        [TestMethod]
        public void OpenProject3DTest()
        {
            string pathToProject3D = @"..\..\..\..\..\Project.project3d";
            ObjectFactory factory = new ObjectFactory();
            ProjectHandle projectHandle = DataAccessAPI.OpenCauldronProject(pathToProject3D.Replace("\\", "/"), "r", factory);

            Assert.IsNotNull(projectHandle, "Could not open project file: "+pathToProject3D);

            FormationList formations = projectHandle.getFormations();
            Assert.IsTrue(formations.Count == 1, "Not all formations loaded, expected 1 but loaded: " + formations.Count.ToString() );

            SurfaceList surfaces = projectHandle.getSurfaces();
            Assert.IsTrue(surfaces.Count == 2, "Not all surfaces loaded, expected 2 but loaded: " + surfaces.Count.ToString() );

            // Get all possible properties
            PropertyList properties = projectHandle.getProperties(true);
            
            Assert.IsTrue(properties.Count >= 395, "Not all possible properties loaded, expected > 395 but loaded: "+properties.Count.ToString() );

            // Get all defined properties
            properties = projectHandle.getProperties();
            Assert.IsTrue(properties.Count == 0, "An project without results doesn't contain any properties");
        }
    }
}
