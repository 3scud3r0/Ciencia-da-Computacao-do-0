import unittest
from server import handle,parse_request
class Tests(unittest.TestCase):
 def test_health(self):
  req=parse_request(b"GET /health HTTP/1.1\r\nHost: localhost\r\n\r\n")
  self.assertEqual(req.headers["host"],"localhost")
  payload=handle(req);self.assertTrue(payload.startswith(b"HTTP/1.1 200 OK"));self.assertTrue(payload.endswith(b"ok\n"))
 def test_invalid(self):
  with self.assertRaises(ValueError):parse_request(b"GET / HTTP/1.1\r\nHost: x\r\n")
if __name__=="__main__":unittest.main()
