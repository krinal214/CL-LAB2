from operator import itemgetter
import sys

from operator import itemgetter
import sys

dict1={}
dict2={}
dept=None
max_mails=0
for line in sys.stdin:
    line = line.strip()
    type,node,dept_or_node=line.split("\t")
    if type=='a':
        dict1[node]=dept_or_node
    if type=='b':
        dept_of_node1=dict1[node]
        dept_of_node2=dict1[dept_or_node]
        if dept_of_node1==dept_of_node2:
            continue
        if dept_of_node1 not in dict2:
            dict2[dept_of_node1]=0
        dict2[dept_of_node1]+=1
        if dept is None:
            dept=dept_of_node1
            max_mails=dict2[dept_of_node1]
        elif dict2[dept_of_node1]>max_mails:
            dept=dept_of_node1
            max_mails=dict2[dept_of_node1]

if dept is not None:
    print("%s\t%s"%(dept,max_mails))
