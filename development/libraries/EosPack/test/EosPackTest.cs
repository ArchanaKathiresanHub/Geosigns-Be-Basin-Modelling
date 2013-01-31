using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Runtime.InteropServices;

namespace EosPack.Test
{
    
    
    /// <summary>
    ///This is a test class for SerialDataAccessTest and is intended
    ///to contain all SerialDataAccessTest Unit Tests
    ///</summary>
    [TestClass()]
    public class EosPackTest
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


        [DllImport("EosPack.dll", EntryPoint = "SetPvtPropertiesConfigFile")]
        private static extern void EosPackSetPvtPropertiesConfigFile(string fileName);

        [TestMethod]
        public void EosPackSetPropertiesConfigFileTest()
        {
            string pvtPropertiesConfigFile = "../geocase/misc/eospack/PVT_properties.cfg";
            EosPackSetPvtPropertiesConfigFile(pvtPropertiesConfigFile);
        }

    }
}
