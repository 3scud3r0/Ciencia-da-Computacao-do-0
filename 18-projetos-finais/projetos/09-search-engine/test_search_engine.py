import tempfile,unittest
from pathlib import Path
from search_engine import SearchEngine
class Tests(unittest.TestCase):
    def test_ranking_and_persistence(self):
        e=SearchEngine()
        e.add("a","banco de dados usa índice b tree")
        e.add("b","redes tcp http dns")
        e.add("c","índice invertido busca documentos busca")
        result=e.search("busca índice")
        self.assertEqual(result[0][0],"c")
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/"index.json";e.save(path);e2=SearchEngine.load(path)
            self.assertEqual(e2.search("tcp")[0][0],"b")
if __name__=="__main__":unittest.main()
