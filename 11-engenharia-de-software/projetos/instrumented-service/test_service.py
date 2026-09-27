import unittest
from service import UserRepository,UserService
class FakeClock:
    def __init__(self):self.t=0
    def __call__(self):self.t+=0.01;return self.t
class Tests(unittest.TestCase):
    def test_service_and_metrics(self):
        service=UserService(UserRepository(),FakeClock())
        self.assertEqual(service.register("1","A@EXAMPLE.COM")["email"],"a@example.com")
        with self.assertRaises(ValueError):service.register("1","b@example.com")
        self.assertEqual(service.metrics.calls,2);self.assertEqual(service.metrics.errors,1)
        self.assertAlmostEqual(service.metrics.total_seconds,0.02,places=8)
if __name__=="__main__":unittest.main()
