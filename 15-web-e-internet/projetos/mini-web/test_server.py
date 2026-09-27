import json,unittest
from server import route
class Tests(unittest.TestCase):
    def test_routes(self):
        status,ctype,body=route("/api/status");self.assertEqual(status,200);self.assertIn("application/json",ctype)
        self.assertEqual(json.loads(body)["message"],"Web do navegador ao backend")
        self.assertEqual(route("/missing")[0],404)
if __name__=="__main__":unittest.main()
