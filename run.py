import os
import sys
import multiprocessing
from pathlib import Path

binary_file_root = os.path.join(os.path.dirname(__file__), "..", "bin")
bname = "clustarexamples"

def run(name, degree):
    command_str = os.path.join(os.path.abspath(binary_file_root), str(name))+" "+str(degree) + " " + "2"
    return os.system(command_str)

def run_parallel(name, degree, core_num):
    '''
        Run on multi cpu cores
    '''
    procs = []
    for _ in range(core_num):
        procs.append(multiprocessing.Process(target=run, args=(name, degree)))
 
    for proc in procs:
        proc.start()
 
    for proc in procs:
        proc.join()

def boot():
    valid = 0
    while (valid == 0):
        print("+---------------------------------------------------------+\n")
        print("| Performance Test                                        |\n")
        print("+---------------------------------------------------------+\n")
        print ('\n')
        core_num = int(input(">Enter number of parallel process(1-40) or exit(0): "))
        if core_num > multiprocessing.cpu_count() or core_num < 0:
            print ('Number of Parallel Process must between 1 and 40!')
            exit(-1)
        elif core_num == 0:
            valid = 1
            continue
        degree = input(">Enter poly_modulus_degree 1024, 2048, 4096, 8192, 16384 or 32768: ")
        if (degree not in ["1024", "2048", "4096", "8192", "16384", "32768"]):
            print ('Poly_Modulus_Degree must be 1024, 2048, 4096, 8192, 16384 or 32768')
            exit(-1)
        if not os.listdir(os.path.join('..', 'clustar', 'record')):
            pass
        else:
            os.system("rm ../clustar/record/*")
        run_parallel(bname, degree, core_num)
        os.system("python3 ../clustar/logAn.py")
        print ('Ok, Done. Please Check the report in the clustar folder.\n\n')

boot()