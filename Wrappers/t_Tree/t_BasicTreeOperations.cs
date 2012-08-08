using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using ROOTNET;
using System.IO;

namespace t_Tree
{
    /// <summary>
    /// Summary description for UnitTest1
    /// </summary>
    [TestClass]
    public class t_BasicTreeOperations
    {
        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestCountEntriesForEach()
        {
            // Open and get the tree
            var tree = OpenAndGet("btag-slim.root", "vtuple");
            int count = 0;
            foreach (var evt in tree)
            {
                count++;
            }
            Assert.AreEqual(300, count, "# of events in the tree");
        }

        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestCountEntriesLINQ()
        {
            // Open and get the tree
            var tree = OpenAndGet("btag-slim.root", "vtuple");
            int count = tree.Count();
            Assert.AreEqual(300, count, "# of events in the tree");
        }

        /// <summary>
        /// Open and get an ntuple
        /// </summary>
        /// <param name="rootFileName"></param>
        /// <param name="treeName"></param>
        /// <returns></returns>
        private NTTree OpenAndGet(string rootFileName, string treeName)
        {
            Assert.IsTrue(File.Exists(rootFileName), string.Format("File {0} can't be found", rootFileName));
            var f = NTFile.Open(rootFileName, "READ");
            Assert.IsTrue(f.IsOpen(), "File open");
            var to = f.Get(treeName);
            Assert.IsNotNull(to, string.Format("{0} not in the file", treeName));
            var t = to as ROOTNET.NTTree;
            Assert.IsNotNull(t, string.Format("{0} is not a TTree object", treeName));
            return t;
        }
    }
}
