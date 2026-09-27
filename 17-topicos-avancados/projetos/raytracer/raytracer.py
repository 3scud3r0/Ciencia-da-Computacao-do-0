from dataclasses import dataclass
from math import sqrt
@dataclass(frozen=True)
class V:
    x:float;y:float;z:float
    def __add__(self,o):return V(self.x+o.x,self.y+o.y,self.z+o.z)
    def __sub__(self,o):return V(self.x-o.x,self.y-o.y,self.z-o.z)
    def __mul__(self,k):return V(self.x*k,self.y*k,self.z*k)
    def dot(self,o):return self.x*o.x+self.y*o.y+self.z*o.z
    def norm(self):
        length=sqrt(self.dot(self));return self*(1/length)
def hit_sphere(origin,direction,center,radius):
    oc=origin-center;a=direction.dot(direction);b=2*oc.dot(direction);c=oc.dot(oc)-radius*radius;disc=b*b-4*a*c
    if disc<0:return None
    t=(-b-sqrt(disc))/(2*a)
    return t if t>0 else None
def render(width=64,height=40):
    origin=V(0,0,0);center=V(0,0,-3);light=V(-1,1,-1).norm();pixels=[]
    aspect=width/height
    for y in range(height):
        row=[]
        for x in range(width):
            px=(2*(x+.5)/width-1)*aspect;py=1-2*(y+.5)/height;direction=V(px,py,-1).norm()
            t=hit_sphere(origin,direction,center,1)
            if t is None:shade=20
            else:
                point=origin+direction*t;normal=(point-center).norm();shade=int(40+215*max(0,normal.dot(light)))
            row.append((shade,shade,shade))
        pixels.append(row)
    return pixels
def ppm(width=64,height=40):
    pixels=render(width,height);body="\n".join(" ".join(f"{r} {g} {b}" for r,g,b in row) for row in pixels)
    return f"P3\n{width} {height}\n255\n{body}\n"
if __name__=="__main__":open("sphere.ppm","w",encoding="ascii").write(ppm())
