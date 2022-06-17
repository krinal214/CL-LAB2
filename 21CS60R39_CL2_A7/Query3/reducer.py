from operator import itemgetter
import sys

dict1={}
for line in sys.stdin:
    line = line.strip()
    src,dest=line.split("\t")
    if dest not in dict1:
        dict1[dest]=[]
    if src not in dict1:
        dict1[src]=[]
    dict1[src].append(dest)
    dict1[dest].append(src)

key=list(dict1.keys())
for i in range(0,len(key)-1):
    for j in range(i+1,len(key)):
        c=0
        for k in dict1[key[i]]:
            if k in dict1[key[j]]:
                c+=1
        if c==0:
            continue
        print("%s\t%s\t%s"%(key[i],key[j],c))