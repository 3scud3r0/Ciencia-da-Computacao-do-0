import heapq
from math import inf
def dijkstra(graph,start):
 if start not in graph:raise KeyError(start)
 for origin,edges in graph.items():
  for destination,weight in edges.items():
   if weight<0:raise ValueError(f"peso negativo: {origin}->{destination}")
 distance={node:inf for node in graph};previous={node:None for node in graph}
 distance[start]=0.0;heap=[(0.0,start)]
 while heap:
  current,node=heapq.heappop(heap)
  if current!=distance[node]:continue
  for neighbor,weight in graph[node].items():
   if neighbor not in distance:distance[neighbor]=inf;previous[neighbor]=None
   candidate=current+weight
   if candidate<distance[neighbor]:
    distance[neighbor]=candidate;previous[neighbor]=node
    heapq.heappush(heap,(candidate,neighbor))
 return distance,previous
def reconstruct_path(previous,start,target):
 if start==target:return [start]
 if target not in previous or previous[target] is None:return []
 path=[];node=target
 while node is not None:
  path.append(node)
  if node==start:return list(reversed(path))
  node=previous[node]
 return []
