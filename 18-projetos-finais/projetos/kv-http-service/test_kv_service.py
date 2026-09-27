import tempfile,unittest
from pathlib import Path
from kv_service import WALStore,App
class Tests(unittest.TestCase):
    def test_persistence_and_api(self):
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/"db.wal";store=WALStore(path);app=App(store)
            self.assertEqual(app.put("/kv/name",'{"value":"Ada"}')[0],200)
            self.assertEqual(app.get("/kv/name"),(200,{"key":"name","value":"Ada"}))
            recovered=WALStore(path);self.assertEqual(recovered.get("name"),"Ada")
if __name__=="__main__":unittest.main()
