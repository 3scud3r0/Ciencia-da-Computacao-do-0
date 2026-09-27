import unittest
from redis_like import RedisLike,encode_resp
class Clock:
    def __init__(self):self.t=100.0
    def __call__(self):return self.t
class Tests(unittest.TestCase):
    def test_commands_and_ttl(self):
        c=Clock();r=RedisLike(c)
        self.assertEqual(r.execute("SET","a","1"),"OK")
        self.assertEqual(r.execute("INCR","a"),2)
        self.assertEqual(r.execute("SET","temp","x","EX",5),"OK")
        self.assertEqual(r.execute("GET","temp"),"x")
        c.t=106;self.assertIsNone(r.execute("GET","temp"));self.assertEqual(r.execute("TTL","temp"),-2)
        self.assertEqual(encode_resp("OK"),b"$2\r\nOK\r\n")
if __name__=="__main__":unittest.main()
