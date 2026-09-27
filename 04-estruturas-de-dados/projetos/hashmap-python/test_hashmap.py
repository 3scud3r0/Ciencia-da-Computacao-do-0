import unittest
from hashmap import HashMap
class Tests(unittest.TestCase):
 def test_insert_delete_resize(self):
  m=HashMap(4)
  for i in range(200):m[i]=i*i
  self.assertEqual(len(m),200)
  for i in range(200):self.assertEqual(m[i],i*i)
  for i in range(0,200,2):del m[i]
  self.assertEqual(len(m),100)
  for i in range(1000,1100):m[i]=i
  self.assertEqual(len(m),200)
if __name__=="__main__":unittest.main()
