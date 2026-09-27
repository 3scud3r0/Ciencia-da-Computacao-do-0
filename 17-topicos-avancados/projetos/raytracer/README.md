# Ray tracer mínimo

Cada pixel lança um raio a partir da câmera. A interseção raio–esfera vira uma equação quadrática; o discriminante determina se há colisão. No ponto atingido, o produto escalar entre normal e direção da luz produz iluminação difusa.

Execute `python raytracer.py`: o arquivo `sphere.ppm` pode ser aberto por vários visualizadores/conversores.

Depois implemente várias esferas, sombras, materiais e reflexão.
