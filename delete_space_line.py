#!/usr/bin/python3
import re
import shutil

print("删除多余的空行\n\n\n\n")
f_output=open('output', 'w', encoding='utf8')
with open('input', 'r', encoding='utf8') as f:
  for line in f:
    # print("line: ", ord(line.strip().replace(" ", "")))
    # print(len(line))
    find_result = re.findall('^[0-9]+$', line.replace("\n", ""))
    if len(find_result) > 0:
      continue
    find_result = re.findall('^[0-9]+\.$', line.replace("\n", ""))
    if len(find_result) > 0:
      continue
    mylist=[c for c in line if ord(c)!=10]
    if mylist:
      print("".join(mylist))
      f_output.write("".join(mylist)+'\n')
f_output.close()

shutil.move("output", "input")
