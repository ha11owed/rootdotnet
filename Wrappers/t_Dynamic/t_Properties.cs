using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    /// <summary>
    /// Test properties on an object
    /// </summary>
    [TestClass]
    public class t_Properties
    {
        [TestMethod]
        public void TestGetStrippedPropertyName()
        {
            var tlz = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var dtlz = (dynamic)tlz;
            Assert.AreEqual(1.0, dtlz.X, "X property");
        }

        [TestMethod]
        public void TestSetStrippedPropertyName()
        {
            var tlz = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var dtlz = (dynamic)tlz;
            dtlz.X = 5.0;
            Assert.AreEqual(5.0, dtlz.X, "X property");
        }

        [TestMethod]
        public void TestGetSetPropertyName()
        {
            var h1 = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F ("hi", "there", 100, 0.0, 10.0);
            Assert.AreEqual(100, h1.Nbins, "# of bins");
            h1.Fill(5.0);
            Assert.AreEqual(1.0, h1.Maximum, "Maximum");
            h1.Maximum = 3.0;
            Assert.AreEqual(3.0, h1.Maximum, "Maximum after being set");
        }
    }
}
