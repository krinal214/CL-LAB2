import sys

file1 = open('network.txt', 'r')

for line in file1:
    line=line.strip()
    node,count=line.split()
    print('%s\t%s\t%s' % ('b',node, count))
file1.close()


file2=open('dept_labels.txt','r')
for line in file2:
    line = line.strip()
    node,dept = line.split()
    print('%s\t%s\t%s' % ('a',node, dept))
file2.close()