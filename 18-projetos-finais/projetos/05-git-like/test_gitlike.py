import tempfile,unittest
from gitlike import GitLike
class Tests(unittest.TestCase):
    def test_commits_branches_checkout_and_diff(self):
        with tempfile.TemporaryDirectory() as tmp:
            g=GitLike(tmp);g.init()
            c1=g.commit({"a.txt":"one"},"first")
            c2=g.commit({"a.txt":"two","b.txt":"new"},"second")
            self.assertEqual(g.diff(c1,c2),{"a.txt":"modified","b.txt":"added"})
            g.branch("old",c1);g.switch("old")
            self.assertEqual(g.files_at(),{"a.txt":b"one"})
            c3=g.commit({"a.txt":"branch"},"branch commit")
            self.assertEqual(g.head(),c3);self.assertNotEqual(c3,c2)
if __name__=="__main__":unittest.main()
