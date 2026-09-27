import unittest
from thread_pool import ThreadPool
class Tests(unittest.TestCase):
 def test_results(self):
  with ThreadPool(3) as pool:
   fs=[pool.submit(lambda x:x*x,i) for i in range(30)]
   self.assertEqual([f.result(timeout=2) for f in fs],[i*i for i in range(30)])
   bad=pool.submit(lambda:1/0)
   with self.assertRaises(ZeroDivisionError):bad.result(timeout=2)
if __name__=="__main__":unittest.main()
