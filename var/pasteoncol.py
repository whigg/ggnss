#! /usr/bin/python
##
##  Given two files this function will write out matching lines; matching lines
##+ are compared using the values at column 'column'
##

from __future__ import print_function
import os, sys

if not os.path.isfile(sys.argv[1]): sys.exit(1)
if not os.path.isfile(sys.argv[2]): sys.exit(1)
column=0

def mysplit(line):
  nlst=[]
  lst = line.split()
  i=0
  while (i<len(lst)):
    if (lst[i][0]=="\"" or lst[i][0]=="\'") and (lst[i+1][-1]=="\"" or lst[i+1][-1]=="\'"):
      nlst.append(' '.join([lst[i], lst[i+1]]))
      i+=2
    else:
      nlst.append(lst[i])
      i+=1
  return nlst

lines1 = []
with open(sys.argv[1], 'r') as fin:
  for line in fin.readlines():
    if len(line)>1 and line[0]!='#':
      l=mysplit(line)
      lines1.append(l)

lines2 = []
with open(sys.argv[2], 'r') as fin:
  for line in fin.readlines():
    if len(line)>1 and line[0]!='#':
      l=mysplit(line)
      lines2.append(l)

if len(lines1) > len(lines2):
  reflines = lines1
  cmplines = lines2
else:
  reflines = lines2
  cmplines = lines1

for l in reflines:
  cmpstr=l[column]
  matching_l=None
  for ln in cmplines:
    if ln[column]==cmpstr:
      matching_l=ln
      break
  if matching_l is not None:
    print("{:} {:}".format(' '.join(l), ' '.join(matching_l)))
