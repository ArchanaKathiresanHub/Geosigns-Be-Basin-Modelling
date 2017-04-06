/*******************************************************************
 * 
 * Copyright (C) 2017 Shell International Exploration & Production.
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
using Shell.BasinModeling.Project3dAPI;


namespace Shell.BasinModeling.Project3dAPI.Test
{
   [TestClass()]
   public partial class Project3dAPITests
   {
      [TestMethod]
      public void CheckCSharpExceptions()
      {
         bool exceptionThrown = false;
         try
         {
            ProjectIoAPI api = new ProjectIoAPI("SomeAbsentProject3DName.project3d");
            Assert.AreEqual( api.saveToProjectFile( "SomeProject3DName.project3d" ), true );
         }
         catch ( System.Exception ex )
         {
            Console.WriteLine( "SystemException: " + ex.ToString() );
            exceptionThrown = true;
         }
         Assert.IsTrue( exceptionThrown );
      }

      [TestMethod]
      public void Project3dRoundtrip()
      {
         bool exceptionThrown = false;
         try
         {
            ProjectIoAPI api = new ProjectIoAPI( "..\\..\\..\\Reference.project3d" );
            Assert.AreEqual( api.saveToProjectFile( "Reference_csharpbmapi.project3d" ), true );
         }
         catch ( System.Exception ex )
         {
            Console.WriteLine( "SystemException: " + ex.ToString() );
            exceptionThrown = true;
         }
         Assert.IsFalse( exceptionThrown );
      }

      [TestMethod]
      public void CreateProject3dFromScratch()
      {
         bool exceptionThrown = false;
         ProjectIoAPI ph = new ProjectIoAPI();
         try
         {
            FillAllTables( ph );
         }
         catch ( System.Exception ex )
         {
            Console.WriteLine( "SystemException: " + ex.ToString() );
            exceptionThrown = true;
         }
         Assert.IsFalse( exceptionThrown );
         Assert.IsTrue( ph.saveToProjectFile( "Reference_cmbapiFromScratch.project3d" ) );
      }
   }
}

