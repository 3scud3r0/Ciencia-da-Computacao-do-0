import unittest
from interpreter import Interpreter,Parser,tokenize
class Tests(unittest.TestCase):
 def test_program(self):
  vm=Interpreter();out=vm.run("let x=2+3*4;let y=(x-2)/3;print y;print -x;")
  self.assertEqual(out,["4","-14"]);self.assertEqual(vm.environment["x"],14)
 def test_name(self):
  with self.assertRaises(NameError):Interpreter().run("print missing;")
 def test_syntax(self):
  with self.assertRaises(SyntaxError):Parser(tokenize("let = 1")).program()
if __name__=="__main__":unittest.main()
