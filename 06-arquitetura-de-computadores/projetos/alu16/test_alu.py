import unittest
from alu import alu,s16
class Tests(unittest.TestCase):
 def test_wrap(self):
  r,f=alu(0xFFFF,1,"ADD");self.assertEqual(r,0);self.assertTrue(f["C"] and f["Z"])
 def test_overflow(self):
  r,f=alu(32767,1,"ADD");self.assertEqual(s16(r),-32768);self.assertTrue(f["V"])
 def test_bits(self):self.assertEqual(alu(10,12,"XOR")[0],6)
if __name__=="__main__":unittest.main()
