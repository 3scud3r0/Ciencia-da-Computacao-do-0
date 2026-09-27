import unittest
from raytracer import V,hit_sphere,render,ppm
class Tests(unittest.TestCase):
    def test_hit(self):self.assertIsNotNone(hit_sphere(V(0,0,0),V(0,0,-1),V(0,0,-3),1))
    def test_render(self):
        pixels=render(16,10);self.assertEqual(len(pixels),10);self.assertEqual(len(pixels[0]),16)
        text=ppm(4,3);self.assertTrue(text.startswith("P3\n4 3\n255\n"))
if __name__=="__main__":unittest.main()
