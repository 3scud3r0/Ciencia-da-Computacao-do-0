import unittest

from recursao import soma_iterativa, soma_recursiva


class TestSoma(unittest.TestCase):
    def test_mesmo_resultado_em_entradas_pequenas(self):
        for n in range(100):
            with self.subTest(n=n):
                esperado = n * (n + 1) // 2
                self.assertEqual(soma_recursiva(n), esperado)
                self.assertEqual(soma_iterativa(n), esperado)

    def test_entrada_negativa(self):
        for funcao in (soma_recursiva, soma_iterativa):
            with self.subTest(funcao=funcao.__name__):
                with self.assertRaises(ValueError):
                    funcao(-1)


if __name__ == "__main__":
    unittest.main()
