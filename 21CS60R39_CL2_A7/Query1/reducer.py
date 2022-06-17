from operator import itemgetter
import sys

def merge_list(list1,list2):
    set1=set(list1)
    set2=set(list2)
    list3=list(set1.union(set2))
    return list3


current_src=None
current_dest=[]
dict1={}
for line in sys.stdin:
    line = line.strip()
    src,dests=line.split("\t",1)
    dests=dests.split("\t")
    if  current_src is None:
        current_dest=dests
        current_src=src
        continue
    elif current_src==src:
        current_dest=merge_list(current_dest,dests)
        current_src=src
        continue
    else:
        length=len(current_dest)
        if length not in dict1:
            dict1[length]=0
        dict1[length]+=1
        current_src=src
        current_dest=dests

if current_src is not None:
    length=len(current_dest)
    if length not in dict1:
        dict1[length]=0
    dict1[length]+=1

for i in dict1:
    print('%s\t%s' %(i,dict1[i]))
    