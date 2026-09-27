import tempfile,unittest
from pathlib import Path
from minidb import MiniDB
class Tests(unittest.TestCase):
    def test_transaction_index_and_recovery(self):
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/"db.wal";db=MiniDB(path)
            db.begin();db.put(2,{"name":"B"});db.put(1,{"name":"A"});db.commit()
            db.begin();db.put(3,{"name":"C"});db.rollback()
            self.assertEqual([k for k,_ in db.scan()],[1,2])
            db2=MiniDB(path);self.assertEqual(db2.get(1)["name"],"A")
            self.assertEqual(db2.scan(2,3),[(2,{"name":"B"})])
if __name__=="__main__":unittest.main()
