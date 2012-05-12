using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.CSharp.RuntimeBinder;

namespace t_Dynamic
{
    /// <summary>
    /// Make sure various things with ctor and dtor are working. For a real test
    /// the monilithic wrapper guy should be pretty minimal.
    /// </summary>
    [TestClass]
    public class t_ROOTCreator
    {
        [TestMethod]
        public void TestCreatorCTor()
        {
            dynamic c = new ROOTNET.Utility.ROOTCreator();
            var h = c.TH1F();
            Assert.Inconclusive();
        }

        [TestMethod]
        [ExpectedException(typeof(RuntimeBinderException))]
        public void TestCreatorCTorBadClass()
        {
            dynamic c = new ROOTNET.Utility.ROOTCreator();
            dynamic h = c.TH1FF();
        }
    }
}
