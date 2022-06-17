from operator import itemgetter
import sys

current_node1=None
current_node2=None
current_count=0
for line in sys.stdin:
    line = line.strip()
    node1,node2=line.split("\t")
    
    
    if  current_node1 is None and current_node2 is None:
        current_node1=node1
        current_node2=node2
        current_count=1
        continue
    elif current_node1==node1 and current_node2==node2:
        current_count+=1
        continue
    else:
        print("%s\t%s\t%s" %(current_node1,current_node2,current_count))
        current_node1=node1
        current_node2=node2
        current_count=1

if current_node1 is not None and current_node2 is not None:
    print("%s\t%s\t%s" %(current_node1,current_node2,current_count))