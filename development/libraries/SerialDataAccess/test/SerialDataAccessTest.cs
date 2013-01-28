using Shell.BasinModeling.DataAccess;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace Shell.BasinModeling.DataAccess.Test
{
    
    
    /// <summary>
    ///This is a test class for SerialDataAccessTest and is intended
    ///to contain all SerialDataAccessTest Unit Tests
    ///</summary>
    [TestClass()]
    public class SerialDataAccessTest
    {


        private TestContext testContextInstance;

        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        #region Additional test attributes
        // 
        //You can use the following additional attributes as you write your tests:
        //
        //Use ClassInitialize to run code before running the first test in the class
        //[ClassInitialize()]
        //public static void MyClassInitialize(TestContext testContext)
        //{
        //}
        //
        //Use ClassCleanup to run code after all tests in a class have run
        //[ClassCleanup()]
        //public static void MyClassCleanup()
        //{
        //}
        //
        //Use TestInitialize to run code before running each test
        //[TestInitialize()]
        //public void MyTestInitialize()
        //{
        //}
        //
        //Use TestCleanup to run code after each test has run
        //[TestCleanup()]
        //public void MyTestCleanup()
        //{
        //}
        //
        #endregion


		/// <summary>
        /// Open a non-existent project file 
        ///</summary>
        [TestMethod()]
        public void OpenCauldronNonExistingProjectTest()
        {
            string name = "non-existent-file.project3d"; 
            ProjectHandle expected = null; // TODO: Initialize to an appropriate value
            ProjectHandle actual;
            actual = Shell.BasinModeling.DataAccess.SerialDataAccess.OpenCauldronProject(name, "r");
            Assert.AreEqual(expected, actual);
        }
		
        /// <summary>
        /// Open an existing project file
        ///</summary>
        [TestMethod()]
        public void OpenCauldronExistingProjectTest()
        {
            string name = "${CMAKE_CURRENT_SOURCE_DIR}\\test\\Project.project3d"; 
            ProjectHandle actual;
            actual = Shell.BasinModeling.DataAccess.SerialDataAccess.OpenCauldronProject(name, "r");
            Assert.IsNotNull( actual );
        }
    }
}
