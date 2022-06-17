from operator import itemgetter
from re import I
import sys


current_src=None
current_dest=[]
INF=100000
map1= [[INF for i in range(42)] for j in range(42)]
for line in sys.stdin:
    line = line.strip()
    src,dest=line.split("\t")
    if map1[int(src)][int(dest)]==INF:
        map1[int(src)][int(dest)]=1
    else:
        map1[int(src)][int(dest)]+=1

max_degree=-1
min_degree=INF
max_degree_nodes=[]
min_degree_nodes=[]
for i in range(42):
    degree=0
    for j in range(42):
        if map1[i][j]==INF:
            continue
        degree+=1
    #print(i,degree)
    if degree<min_degree:
        min_degree=degree
        min_degree_nodes=[i]
    elif degree==min_degree:
        min_degree_nodes.append(i)
    if degree>max_degree:
        max_degree=degree
        max_degree_nodes=[i]
    elif degree==max_degree:
        max_degree_nodes.append(i)

for k in range(42):
    for i in range(42):
        for j in range(42):
            map1[i][j]=min(map1[i][j],map1[i][k]+map1[k][j])

for p in max_degree_nodes:
    for q in min_degree_nodes:
        if map1[p][q]==INF:
            print('%d\t%d\t%s'%(p,q,'Not reachable'))
        else:
            print('%d\t%d\t%d' %(p,q,map1[p][q]))
