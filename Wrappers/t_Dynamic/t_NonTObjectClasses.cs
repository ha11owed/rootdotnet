using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    [TestClass]
    public class t_NonTObjectClasses
    {
        // Force a reference to the monolithic wrapper guys - so that we can
        // make sure that the wrappers get loaded into memory.
#pragma warning disable 0414
        static ROOTNET.NTObject _obj = null;
#pragma warning restore 0414

        [TestInitialize]
        public void TestInit()
        {
            ROOTNET.Utility.ROOTCreator.ResetROOTCreator();
        }

        [TestMethod]
        public void TestTAttLineCtor()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttLine");
        }

        [TestMethod]
        public void TestCallTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttLine");
            vec.SetLineWidth(1);
            Assert.AreEqual(1.0, vec.GetLineWidth(), "Line With");
        }

        [TestMethod]
        public void TestGetPropertyTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttLine");
            vec.SetLineWidth(1);
            Assert.AreEqual(1.0, vec.LineWidth, "Line With");
        }

        [TestMethod]
        public void TestSetPropertyTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttLine");
            vec.LineWidth = 2.0;
            Assert.AreEqual(2.0, vec.LineWidth, "Line With");
        }

        [TestMethod]
        public void TestObjectTableLookup()
        {
            Assert.Fail("Make sure that the void pointer two ways gets back the same thing");
        }

        [TestMethod]
        public void TestObjectTableDrop()
        {
            Assert.Fail("Make sure that the void point dropped gets a new wrapper");
        }

    }
}
