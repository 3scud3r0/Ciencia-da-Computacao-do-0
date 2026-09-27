from __future__ import annotations
from collections import Counter,defaultdict
from math import log
import json,re
TOKEN=re.compile(r"[A-Za-zÀ-ÿ0-9]+",re.UNICODE)

def tokenize(text):
    return [x.lower() for x in TOKEN.findall(text)]

class SearchEngine:
    def __init__(self):
        self.docs={};self.lengths={};self.inverted=defaultdict(dict)
    def add(self,doc_id,text):
        if doc_id in self.docs:self.remove(doc_id)
        terms=tokenize(text);counts=Counter(terms);self.docs[doc_id]=text;self.lengths[doc_id]=len(terms)
        for term,count in counts.items():self.inverted[term][doc_id]=count
    def remove(self,doc_id):
        if doc_id not in self.docs:return
        for term in list(self.inverted):
            self.inverted[term].pop(doc_id,None)
            if not self.inverted[term]:del self.inverted[term]
        self.docs.pop(doc_id);self.lengths.pop(doc_id)
    def search(self,query,k=10):
        terms=tokenize(query);n=len(self.docs)
        if n==0:return []
        avgdl=sum(self.lengths.values())/n;b=.75;k1=1.5;scores=defaultdict(float)
        for term in terms:
            postings=self.inverted.get(term,{})
            df=len(postings)
            if df==0:continue
            idf=log(1+(n-df+.5)/(df+.5))
            for doc_id,tf in postings.items():
                dl=self.lengths[doc_id];den=tf+k1*(1-b+b*dl/avgdl)
                scores[doc_id]+=idf*(tf*(k1+1)/den)
        return sorted(scores.items(),key=lambda x:(-x[1],x[0]))[:k]
    def save(self,path):
        payload={"docs":self.docs}
        with open(path,"w",encoding="utf-8") as f:json.dump(payload,f,ensure_ascii=False)
    @classmethod
    def load(cls,path):
        with open(path,encoding="utf-8") as f:payload=json.load(f)
        engine=cls()
        for doc_id,text in payload["docs"].items():engine.add(doc_id,text)
        return engine
