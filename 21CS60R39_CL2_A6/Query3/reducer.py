from operator import itemgetter
import sys

current_node=None
current_count=0
for line in sys.stdin:
    line = line.strip()
    top_10,sender=line.split("\t")
    
    if  current_node is None:
        current_count=1
        current_node=top_10
        continue
    elif current_node==top_10:
        current_count+=1
        continue
    else:
        print("%s\t%s"%(current_node,current_count))
        current_node=top_10
        current_count=1

if current_node is not None:
    print("%s\t%s"%(current_node,current_count))