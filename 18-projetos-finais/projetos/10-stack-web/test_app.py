import unittest
from app import App
class Clock:
    def __init__(self):self.t=100.0
    def __call__(self):return self.t
class Tests(unittest.TestCase):
    def test_full_flow_and_auth(self):
        clock=Clock();tokens=iter(["token-1"]);app=App(clock=clock,token_factory=lambda:next(tokens))
        status,payload=app.route("POST","/register",json_body={"email":"a@example.com","password":"12345678"});self.assertEqual(status,201)
        status,payload=app.route("POST","/login",json_body={"email":"a@example.com","password":"12345678"});self.assertEqual(status,200)
        token=payload["token"];headers={"Authorization":"Bearer "+token}
        self.assertEqual(app.route("POST","/notes",headers,{"body":"estudar TCP"})[0],201)
        status,payload=app.route("GET","/notes",headers);self.assertEqual(status,200);self.assertEqual(payload["notes"][0]["body"],"estudar TCP")
        clock.t=5000;self.assertEqual(app.route("GET","/notes",headers)[0],401)
if __name__=="__main__":unittest.main()
