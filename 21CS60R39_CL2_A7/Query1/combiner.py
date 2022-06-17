from operator import itemgetter
import sys

current_src=None
current_text=''
prev_dest=None
for line in sys.stdin:
    line = line.strip()
    src,dest=line.split("\t")
    
    
    if  current_src is None:
        current_text='\t'+dest
        current_src=src
        prev_dest=dest
        continue
    elif current_src==src:
        if prev_dest==dest:
            continue
        current_text+='\t'+dest
        current_src=src
        prev_dest=dest
        continue
    else:
        print("%s%s" %(current_src,current_text))
        current_src=src
        current_text='\t'+dest
        prev_dest=dest

if current_src is not None:
    print("%s%s" %(current_src,current_text))