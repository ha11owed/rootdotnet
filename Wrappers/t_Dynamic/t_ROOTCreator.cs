﻿using Microsoft.CSharp.RuntimeBinder;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    /// <summary>
    /// Make sure various things with ctor and dtor are working. For a real test
    /// the monilithic wrapper guy should be pretty minimal.
    /// </summary>
    [TestClass]
    public class t_ROOTCreator
    {
        // Force a reference to the monolithic wrapper guys - so that we can
        // make sure that the wrappers get loaded into memory.
        static ROOTNET.NTObject _obj = null;

        [TestMethod]
        public void CreateTH1FNoArgs()
        {
            dynamic c = new ROOTNET.Utility.ROOTCreator();
            var h = c.TH1F();
            Assert.AreEqual(1, h.GetNbinsX(), "# of bins"); // 1 is the default
        }

        [TestMethod]
        public void CreateTH1FNoArgsByName()
        {
            dynamic h = ROOTNET.Utility.ROOTCreator.CreateByName("TH1F", new object[0]);
            Assert.AreEqual(1, h.GetNbinsX(), "# of bins");
        }

        [TestMethod]
        [ExpectedException(typeof(RuntimeBinderException))]
        public void TestCreatorCTorBadClass()
        {
            dynamic c = new ROOTNET.Utility.ROOTCreator();
            dynamic h = c.TH1FF();
        }

        [TestMethod]
        [ExpectedException(typeof(RuntimeBinderException))]
        public void NoNonTObjectClasses()
        {
            // We can't deal with non-TObject classes. Make sure we bomb correctly here!
            Assert.Inconclusive();
        }
    }
}
