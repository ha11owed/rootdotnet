using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Operators
{
    /// <summary>
    /// Test the operator implementation. This test will only build if TLorentzVector is built
    /// as part of the monolithic stuff!
    /// </summary>
    [TestClass]
    public class t_SimpleOperators
    {
        [TestMethod]
        public void TestAdditionWithObjects()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a2 = new ROOTNET.NTLorentzVector(0.5, 0.5, 0.5, 0.5);
            var a3 = a1 + a2;
            Assert.AreEqual(1.5, a3.X(), "X");
            Assert.AreEqual(2.5, a3.Y(), "Y");
            Assert.AreEqual(3.5, a3.Z(), "Z");
            Assert.AreEqual(4.5, a3.T(), "T");
        }
    }
}
