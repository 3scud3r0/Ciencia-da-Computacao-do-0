import unittest
from password_store import hash_password,verify_password
class Tests(unittest.TestCase):
    def test_roundtrip(self):
        a=hash_password("correct horse battery staple");b=hash_password("correct horse battery staple")
        self.assertNotEqual(a,b)
        self.assertTrue(verify_password("correct horse battery staple",a))
        self.assertFalse(verify_password("wrong",a))
    def test_bad_format(self):self.assertFalse(verify_password("x","garbage"))
if __name__=="__main__":unittest.main()
