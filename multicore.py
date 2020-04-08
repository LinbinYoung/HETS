import os
import multiprocessing

def run(name):
    return os.system('./'+str(name) + " " + "2")

def run_parallel(name):
 
    '''
        Run on all cpu cores
    '''

    n_cpu = multiprocessing.cpu_count()
    procs = []

    for _ in range(n_cpu):
        procs.append(multiprocessing.Process(target=run, args=(name,)))
 
    for proc in procs:
        proc.start()
 
    for proc in procs:
        proc.join()
 
if __name__ == "__main__":
    run_parallel('a.out')
