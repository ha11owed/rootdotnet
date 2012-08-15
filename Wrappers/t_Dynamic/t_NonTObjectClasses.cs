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
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            // This is not a known class. Make sure that we've not accidentally put this in.
            Assert.AreNotEqual("NTAxis", vec.GetType().Name, "wrapper object class");
        }

        [TestMethod]
        public void TestCallTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            vec.SetTextAngle(45.0);
            Assert.AreEqual(45.0, vec.GetTextAngle(), "Text Angle");
        }

        [TestMethod]
        public void TestGetPropertyTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            vec.SetTextAngle(50.0);
            Assert.AreEqual(50.0, vec.TextAngle, "Text Angle");
        }

        [TestMethod]
        public void TestSetPropertyTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            vec.TextAngle = 2.0;
            Assert.AreEqual(2.0, vec.GetTextAngle, "Text Angle");
        }

        [TestMethod]
        public void TestPassByReference()
        {
            dynamic a1 = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            a1.TextAngle = 2.0;
            dynamic a2 = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText", a1);
            Assert.AreEqual(2.0, a2.TextAngle, "Text Angle");
        }

        [TestMethod]
        public void TestCTorWithKnownClass()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttFill");
            Assert.AreEqual("NTAttFill", vec.GetType().Name, "wrapper object class");
        }

    }
}
