using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
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
        [TestMethod]
        public void TestCreatorCTor()
        {
            var c = new ROOTNET.Utility.ROOTCreator();
        }
    }
}
