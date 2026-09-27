import unittest
from bytecode_lang import Compiler,VM
class Tests(unittest.TestCase):
    def test_compile_and_run(self):
        code=Compiler().compile("let x=2+3*4; let y=x-5; print y; print -x;")
        vm=VM();self.assertEqual(vm.run(code),[9,-14]);self.assertEqual(vm.env["x"],14)
        self.assertTrue(any(ins[0]=="MUL" for ins in code))
    def test_name_error(self):
        with self.assertRaises(NameError):VM().run(Compiler().compile("print missing;"))
if __name__=="__main__":unittest.main()
