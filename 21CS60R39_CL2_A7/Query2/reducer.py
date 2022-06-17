from operator import itemgetter
import sys

current_node1=None
current_node2=None
current_count=0
for line in sys.stdin:
    line = line.strip()
    node1,node2,count=line.split("\t")
    
    
    if  current_node1 is None and current_node2 is None:
        current_node1=node1
        current_node2=node2
        current_count=int(count)
        continue
    elif current_node1==node1 and current_node2==node2:
        current_count+=int(count)
        continue
    else:
        if current_count>=50:
            print("%s\t%s" %(current_node1,current_node2))
        current_node1=node1
        current_node2=node2
        current_count=int(count)

if current_node1 is not None and current_node2 is not None and current_count>=50:
    print("%s\t%s" %(current_node1,current_node2))