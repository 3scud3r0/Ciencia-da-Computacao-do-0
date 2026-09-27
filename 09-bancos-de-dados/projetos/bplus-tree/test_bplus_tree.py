import random,unittest
from bplus_tree import BPlusTree
class Tests(unittest.TestCase):
 def test_insert_search_update_range(self):
  tree=BPlusTree(5);keys=list(range(1000));random.Random(7).shuffle(keys)
  for k in keys:tree.insert(k,str(k))
  self.assertTrue(tree.validate())
  for k in range(1000):self.assertEqual(tree.search(k),str(k))
  tree.insert(50,'updated');self.assertEqual(tree.search(50),'updated')
  self.assertEqual([k for k,_ in tree.range(100,120)],list(range(100,120)))
  self.assertEqual([k for k,_ in tree.range(None,5)],list(range(5)))
if __name__=='__main__':unittest.main()
