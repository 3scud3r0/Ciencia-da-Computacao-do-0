import unittest
from dijkstra import dijkstra,reconstruct_path
class Tests(unittest.TestCase):
 def test_paths(self):
  g={"A":{"B":4,"C":1},"B":{"D":1},"C":{"B":2,"D":5},"D":{}}
  d,p=dijkstra(g,"A");self.assertEqual(d["D"],4)
  self.assertEqual(reconstruct_path(p,"A","D"),["A","C","B","D"])
 def test_negative(self):
  with self.assertRaises(ValueError):dijkstra({"A":{"B":-1},"B":{}},"A")
if __name__=="__main__":unittest.main()
