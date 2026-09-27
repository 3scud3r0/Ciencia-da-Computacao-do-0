import unittest
from kernel import Kernel
class Tests(unittest.TestCase):
    def test_scheduler_and_io(self):
        k=Kernel(quantum=2)
        a=k.spawn([("CPU",3),("IO",3),("CPU",2)])
        b=k.spawn([("CPU",4)])
        total=k.run()
        self.assertEqual(k.processes[a].cpu_time,5)
        self.assertEqual(k.processes[b].cpu_time,4)
        self.assertEqual(k.processes[a].state,"done")
        self.assertGreaterEqual(total,9)
        self.assertTrue(any("io(3)" in x for x in k.processes[a].trace))
if __name__=="__main__":unittest.main()
