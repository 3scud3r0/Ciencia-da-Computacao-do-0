import tempfile,unittest
from minigit import MiniGit
class Tests(unittest.TestCase):
 def test_content_addressed_objects(self):
  with tempfile.TemporaryDirectory() as tmp:
   g=MiniGit(tmp);g.init();a=g.hash_blob(b'hello');b=g.hash_blob(b'hello');self.assertEqual(a,b)
   self.assertEqual(g.read_object(a),('blob',b'hello'))
   tree=g.write_tree({'hello.txt':a});commit=g.commit(tree,'primeiro commit')
   self.assertEqual(g.head(),commit);kind,payload=g.read_object(commit);self.assertEqual(kind,'commit');self.assertIn(tree.encode(),payload)
if __name__=='__main__':unittest.main()
