from operator import itemgetter
import sys

current_src=None
current_count=0
for line in sys.stdin:
    line = line.strip()
    src,count=line.split("\t")
    try:
        count = int(count)
    except ValueError:
        continue
    
    if  current_src is None:
        current_count=count
        current_src=src
        continue
    elif current_src==src:
        current_count+=count
        continue
    else:
        print("%s\t%s" %(current_src,current_count))
        current_src=src
        current_count=count

if current_src is not None:
    print("%s\t%s" %(current_src,current_count))