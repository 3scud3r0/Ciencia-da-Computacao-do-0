import unittest
from consistent_hash import ConsistentHashRing
class Tests(unittest.TestCase):
 def test_remapping(self):
  ring=ConsistentHashRing(64)
  for n in "abc":ring.add_node(n)
  keys=[f"key-{i}" for i in range(2000)];before={k:ring.locate(k) for k in keys}
  ring.add_node("d");after={k:ring.locate(k) for k in keys}
  moved=sum(before[k]!=after[k] for k in keys)
  self.assertGreater(moved,0);self.assertLess(moved,len(keys)//2)
  ring.remove_node("d");self.assertEqual(before,{k:ring.locate(k) for k in keys})
if __name__=="__main__":unittest.main()
