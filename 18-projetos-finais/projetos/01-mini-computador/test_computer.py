import unittest
from computer import Assembler,CPU16
class Tests(unittest.TestCase):
    def test_arithmetic_memory_and_output(self):
        src="""
        LOADI R0, 7
        LOADI R1, 5
        ADD R0, R1
        STORE R0, 200
        LOAD R2, 200
        OUT R2
        HALT
        """
        cpu=CPU16();cpu.load_program(Assembler().assemble(src))
        self.assertEqual(cpu.run(),[12]);self.assertEqual(cpu.mem[200],12)
    def test_labels_and_loop(self):
        src="""
        LOADI R0, 3
        LOADI R1, 1
        loop:
        SUB R0, R1
        JNZ R0, loop
        OUT R0
        HALT
        """
        cpu=CPU16();cpu.load_program(Assembler().assemble(src))
        self.assertEqual(cpu.run(),[0])
if __name__=="__main__":unittest.main()
