import sys

file1 = open('network.txt', 'r')

for line in file1:
    line = line.strip()
    src,dest = line.split()
    print('%s\t%s' % (src, 1))
file1.close()