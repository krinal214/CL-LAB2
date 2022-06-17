import sys

for line in sys.stdin:
    line = line.strip()
    src,dest = line.split()
    if src==dest:
        continue
    print('%s\t%s' % (min(src, dest),max(src,dest)))
    
    