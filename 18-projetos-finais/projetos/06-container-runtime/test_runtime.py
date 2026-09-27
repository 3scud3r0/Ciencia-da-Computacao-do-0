import unittest
from unittest.mock import patch
from runtime import ContainerRuntime,ContainerSpec
class Tests(unittest.TestCase):
    def test_command_contains_namespaces_and_env(self):
        r=ContainerRuntime()
        with patch.object(r,"supported",return_value=True):
            cmd=r.command_for(ContainerSpec(["python","-V"],hostname="demo",env={"X":"1"}))
        self.assertEqual(cmd[0],"unshare")
        for flag in r.NAMESPACE_FLAGS:self.assertIn(flag,cmd)
        self.assertIn("CC0_HOSTNAME=demo",cmd);self.assertIn("X=1",cmd)
        self.assertEqual(cmd[-2:],["python","-V"])
    def test_proc_namespace_introspection(self):
        ids=ContainerRuntime().namespace_ids()
        if ids:self.assertIn("pid",ids)
if __name__=="__main__":unittest.main()
