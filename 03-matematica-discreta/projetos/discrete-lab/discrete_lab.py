from collections import deque
from itertools import product
from math import comb
def truth_table(variables,expression):
    rows=[]
    for values in product([False,True],repeat=len(variables)):
        env=dict(zip(variables,values));rows.append((env,bool(expression(**env))))
    return rows
def bfs(graph,start):
    seen={start};order=[];queue=deque([start])
    while queue:
        node=queue.popleft();order.append(node)
        for neighbor in graph.get(node,()):
            if neighbor not in seen:seen.add(neighbor);queue.append(neighbor)
    return order
def count_binary_strings(n,k):
    if not 0<=k<=n:return 0
    return comb(n,k)
def recurrence_fibonacci(n):
    if n<0:raise ValueError("n >= 0")
    a,b=0,1
    for _ in range(n):a,b=b,a+b
    return a
