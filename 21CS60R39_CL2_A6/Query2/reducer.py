from operator import itemgetter
import sys
import heapq as hp

current_src=None
current_count=0
h=[]
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
        hp.heappush(h,(-1*current_count,current_src))
        current_src=src
        current_count=count

if current_src is not None:
    hp.heappush(h,(-1*current_count,current_src))
    
c=0
while c<10:
    item=hp.heappop(h)
    print('%s\t%s'%(item[1],-1*item[0]))
    c+=1
