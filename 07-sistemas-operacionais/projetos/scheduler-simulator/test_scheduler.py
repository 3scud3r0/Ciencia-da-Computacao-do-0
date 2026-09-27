import unittest
from scheduler import Job,fcfs,sjf,round_robin
class Tests(unittest.TestCase):
 def setUp(self):self.jobs=[Job('A',0,5),Job('B',1,2),Job('C',2,1)]
 def test_fcfs(self):
  t,m=fcfs(self.jobs);self.assertEqual([(x.job,x.start,x.end) for x in t],[('A',0,5),('B',5,7),('C',7,8)]);self.assertEqual(m['C']['waiting'],5)
 def test_sjf(self):
  t,m=sjf(self.jobs);self.assertEqual([x.job for x in t],['A','C','B']);self.assertEqual(m['C']['waiting'],3)
 def test_rr(self):
  t,m=round_robin(self.jobs,2);self.assertEqual(t[0].job,'A');self.assertEqual(m['A']['completion'],8);self.assertEqual(m['B']['completion'],4)
if __name__=='__main__':unittest.main()
