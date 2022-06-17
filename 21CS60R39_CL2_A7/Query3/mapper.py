import sys

for line in sys.stdin:
    line = line.strip()
    node1,node2 = line.split()
    print('%s\t%s' % (node1,node2))
    
    