import unittest
from virtual_fs import VirtualFS
class Tests(unittest.TestCase):
    def test_tree_and_files(self):
        fs=VirtualFS();fs.mkdir("/home");fs.write("/home/user/a.txt","A");fs.write("/home/user/b.txt","B")
        self.assertEqual(fs.read("/home/user/a.txt"),"A")
        self.assertEqual(fs.listdir("/home/user"),["a.txt","b.txt"])
        self.assertEqual(fs.walk("/"),["/home/user/a.txt","/home/user/b.txt"])
    def test_errors(self):
        fs=VirtualFS();fs.write("/x","data")
        with self.assertRaises(NotADirectoryError):fs.write("/x/y","bad")
if __name__=="__main__":unittest.main()
