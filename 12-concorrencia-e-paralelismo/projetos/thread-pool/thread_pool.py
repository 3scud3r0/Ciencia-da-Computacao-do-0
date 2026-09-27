from concurrent.futures import Future
from queue import Queue
from threading import Thread
_STOP=object()
class ThreadPool:
 def __init__(self,workers=4):
  if workers<=0:raise ValueError("workers deve ser positivo")
  self._queue=Queue();self._closed=False
  self._threads=[Thread(target=self._worker,daemon=True,name=f"cc0-worker-{i}") for i in range(workers)]
  for t in self._threads:t.start()
 def submit(self,fn,/,*args,**kwargs):
  if self._closed:raise RuntimeError("thread pool encerrado")
  future=Future();self._queue.put((future,fn,args,kwargs));return future
 def _worker(self):
  while True:
   item=self._queue.get()
   try:
    if item is _STOP:return
    future,fn,args,kwargs=item
    if not future.set_running_or_notify_cancel():continue
    try:future.set_result(fn(*args,**kwargs))
    except BaseException as exc:future.set_exception(exc)
   finally:self._queue.task_done()
 def shutdown(self,wait=True):
  if self._closed:return
  self._closed=True
  for _ in self._threads:self._queue.put(_STOP)
  if wait:
   for t in self._threads:t.join()
 def __enter__(self):return self
 def __exit__(self,*_):self.shutdown()
