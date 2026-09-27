import unittest
from discrete_lab import truth_table,bfs,count_binary_strings,recurrence_fibonacci
class Tests(unittest.TestCase):
    def test_logic(self):
        rows=truth_table(["p","q"],lambda p,q:(not p) or q)
        self.assertEqual([r for _,r in rows],[True,True,False,True])
    def test_graph(self):
        self.assertEqual(bfs({"A":["B","C"],"B":["D"],"C":[],"D":[]},"A"),["A","B","C","D"])
    def test_combinatorics(self):self.assertEqual(count_binary_strings(5,2),10)
    def test_recurrence(self):self.assertEqual(recurrence_fibonacci(10),55)
if __name__=="__main__":unittest.main()
