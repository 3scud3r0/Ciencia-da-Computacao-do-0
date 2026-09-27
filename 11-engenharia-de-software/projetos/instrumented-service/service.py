from dataclasses import dataclass
from time import perf_counter
@dataclass
class Metrics:
    calls:int=0;errors:int=0;total_seconds:float=0.0
class UserRepository:
    def __init__(self):self._users={}
    def save(self,user):self._users[user["id"]]=dict(user)
    def get(self,user_id):return self._users.get(user_id)
class UserService:
    def __init__(self,repository,clock=perf_counter):
        self.repository=repository;self.clock=clock;self.metrics=Metrics()
    def register(self,user_id,email):
        started=self.clock();self.metrics.calls+=1
        try:
            if not user_id:raise ValueError("id obrigatório")
            if "@" not in email:raise ValueError("email inválido")
            if self.repository.get(user_id) is not None:raise ValueError("usuário já existe")
            user={"id":user_id,"email":email.lower()}
            self.repository.save(user);return user
        except Exception:
            self.metrics.errors+=1;raise
        finally:
            self.metrics.total_seconds+=self.clock()-started
