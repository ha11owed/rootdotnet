﻿using Microsoft.CSharp.RuntimeBinder;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    /// <summary>
    /// Tests to make sure method invokation works correctly, as well as argument
    /// passing and returns.
    /// </summary>
    [TestClass]
    public class t_callingMethods
    {
        // Force a reference to the monolithic wrapper guys - so that we can
        // make sure that the wrappers get loaded into memory.
        static ROOTNET.NTObject _obj = null;

        [TestInitialize]
        public void TestInit()
        {
            ROOTNET.Utility.ROOTCreator.ResetROOTCreator();
        }

        [TestMethod]
        public void TestIntegerReturns()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            Assert.AreEqual(1, c.GetAxisColor(), "Axis color reset");
        }

        [TestMethod]
        public void TestVoidReturns()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetAxisColor(2);
            Assert.AreEqual(2, c.GetAxisColor(), "Axis color reset");
        }

        [TestMethod]
        [ExpectedException(typeof(RuntimeBinderException))]
        public void TestBadMethodName()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetAxisColors(2);
        }

        [TestMethod]
        public void StringArgument()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetOption("dude");
            Assert.AreEqual("dude", c.GetOption(), "object string name");
        }

        [TestMethod]
        public void StringArgumentReturn()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TAxis();
            c.Set(10, 1.0, 11.0);
            c.SetBinLabel(1, "hi");
            Assert.AreEqual("hi", c.GetBinLabel(1), "Bin name");
        }
    }
}
