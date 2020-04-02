import os
import sys
import multiprocessing
from pathlib import Path

binary_file_root = os.path.join(os.path.dirname(__file__), "..", "bin")
bname = "clustarexamples"

def run(name, degree):
    return os.system(os.path.join(binary_file_root, str(name)+" "+str(degree)))

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
    core_num = input("Set number of parallel process(1-40): ")
    if core_num > multiprocessing.cpu_count() or core_num <= 0:
        print ('Number of Parallel Process must between 1 and 40!')
        exit(-1)
    degree = input("Set poly_modulus_degree 1024, 2048, 4096, 8192, 16384 or 32768: ")
    if (degree not in [1024, 2048, 4096, 8192, 16384, 32768]):
        print ('Poly_Modulus_Degree must be 1024, 2048, 4096, 8192, 16384 or 32768')
        exit(-1)
    run_parallel(bname, degree, core_num)

boot()