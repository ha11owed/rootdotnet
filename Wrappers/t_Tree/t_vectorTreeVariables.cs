using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Tree
{
    /// <summary>
    /// Test out tree's with vectors in them.
    /// </summary>
    [TestClass]
    public class t_vectorTreeVariables
    {
        [TestMethod]
        [DeploymentItem("vectorintonly_5.root")]
        public void TestVectorMethodSize()
        {
            var t = Utils.OpenAndGet("vectorintonly_5.root", "dude");
            Assert.AreEqual(5, t.Count(), "Count");
            Assert.AreEqual(5, t.Cast<dynamic>().Where(evt => evt.myvectorofint.size() == 10).Count(), "# of entries with 10 items in the vector");
        }

        [TestMethod]
        [DeploymentItem("vectorintonly_5.root")]
        public void TestVectorIndexer()
        {
            var t = Utils.OpenAndGet("vectorintonly_5.root", "dude");
            Assert.AreEqual(5, t.Count(), "Count");
            Assert.AreEqual(5, t.Cast<dynamic>().Where(evt => evt.myvectorofint[2] == 2).Count(), "# of entries with 10 items in the vector");
        }
    }
}
