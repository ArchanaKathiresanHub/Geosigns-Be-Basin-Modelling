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
        public void CreateProject()
        {
            Project project = ImportExport.importFromXML("../../cauldron_outputs.xml");
            int count = project.getSnapShots().Count;
            Assert.IsTrue(count == 3);
        }
    }
}
