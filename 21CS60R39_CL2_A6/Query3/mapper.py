import sys
import os

parent_dir=os.path.dirname(os.getcwd())
path=os.path.join(parent_dir,'Query2','result.txt')
file1 = open(path, 'r')

list1=[]
for line in file1:
    line=line.strip()
    node,_=line.split()
    list1.append(node)
file1.close()

file2=open('network.txt','r')
for line in file2:
    line = line.strip()
    src,dest = line.split()
    if dest in list1:
        print('%s\t%s' % (dest, src))
file2.close()