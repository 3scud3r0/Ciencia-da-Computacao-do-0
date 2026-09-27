import unittest
from automata import NFA,EPSILON
class Tests(unittest.TestCase):
 def setUp(self):self.nfa=NFA(0,{3},{(0,EPSILON):{1},(1,'a'):{1},(1,'b'):{3}})
 def test_nfa(self):
  for s in ['b','ab','aaaaab']:self.assertTrue(self.nfa.accepts(s))
  for s in ['','a','ba','bb']:self.assertFalse(self.nfa.accepts(s))
 def test_determinization(self):
  dfa=self.nfa.determinize();samples=['','a','b','ab','aaab','ba','bbbb']
  self.assertEqual([self.nfa.accepts(s) for s in samples],[dfa.accepts(s) for s in samples])
if __name__=='__main__':unittest.main()
