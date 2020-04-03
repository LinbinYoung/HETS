import os
import sys
from pathlib import Path
from collections import defaultdict

record_path = os.path.join(os.path.dirname(__file__), ".", "record")

def getFileList():
    filelist = []
    for root, _, log_name in os.walk(record_path):
        filelist = [os.path.join(root, log_name[i]) for i in range(len(log_name))]
    return filelist

def Calc():
    filelist = getFileList()
    total_case = 0
    dict1 = defaultdict(list)
    dict2 = defaultdict(int)
    for each_file in filelist:
        f = open(each_file)
        while 1:
            line = f.readline().strip()
            if not line:
                break
            print (line.split(':'))
            if (line.split(':')[0] == 'count'):
                total_case = total_case + int(line.split(':')[1])
            else:
                dict1[line.split(':')[0]].append(int(line.split(':')[1].split()[0]))
        f.close()
    
    f = open('./report.txt', 'ab+')
    per_core_task = (total_case + len(filelist) - 1) // len(filelist)
    for elem in dict1:
        avg_time = sum(dict1[elem]) / (len(dict1[elem]) * len(dict1[elem])) 
        str1 = elem + ": " + str(per_core_task) + " " + str(avg_time) + " " + str(per_core_task / avg_time) + "\n"
        f.write(str1.encode())
    f.close()

Calc()