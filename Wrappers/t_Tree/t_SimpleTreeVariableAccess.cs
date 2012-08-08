using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Tree
{
    [TestClass]
    public class t_SimpleTreeVariableAccess
    {
        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestSimpleInt()
        {
            // Open and get the tree
            var tree = Utils.OpenAndGet("btag-slim.root", "vtuple");
            int count = 0;
            foreach (dynamic evt in tree)
            {
                var r = evt.run;
                Assert.IsInstanceOfType(r, typeof(int), "Run number type");
                Assert.AreEqual(157037, r, "Run number");
            }
            Assert.AreEqual(100, count, "# of events in the tree");
        }
    }
}
