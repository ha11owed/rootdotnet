using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    [TestClass]
    public class t_STLObjects
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
        public void TestVectorCTor()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("vector<int>");
        }

        [TestMethod]
        public void TestVectorCTorFullSpec()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("vector<int,allocator<int> >");            
        }
    }
}
