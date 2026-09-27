import unittest
from distributed_kv import Cluster,Node
class Tests(unittest.TestCase):
    def test_quorum_failure_and_read_repair(self):
        nodes=[Node("a"),Node("b"),Node("c")]
        c=Cluster(nodes,3,2,2);c.put("k","v1")
        c.set_alive("c",False);c.put("k","v2")
        c.set_alive("c",True)
        self.assertEqual(c.get("k"),"v2")
        self.assertEqual(nodes[2].data["k"][1],"v2")
    def test_write_quorum_failure(self):
        nodes=[Node("a"),Node("b"),Node("c")];c=Cluster(nodes,3,2,2)
        c.set_alive("a",False);c.set_alive("b",False)
        with self.assertRaises(RuntimeError):c.put("x","y")
if __name__=="__main__":unittest.main()
