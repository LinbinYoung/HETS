/*
    Author: Linbin Yang @ Clustar.ai
*/

#include "common.h"

int main(int argc, char** argv){
#ifdef SEAL_VERSION
    cout << "Microsoft SEAL version: " << SEAL_VERSION << endl;
#endif
    if (argc == 3){
        if (atoi(argv[2]) == 1){
            calc_bfv_basic(atoi(argv[1]));
        }else if (atoi(argv[2]) == 2){
            bfv_performance_custom(atoi(argv[1]));
        }else{
            cout << "Unknown Option (1-calculator, 2-performance)" << endl;
            return -1;
        }
    }else if (argc == 2){
        /* Main Page
        */
        bool invalid = true;
        do{
            system("clear");
            cout << clustar_flag;
            int op = 1;
            cout << endl << ">Enter Task Mode (1 ~ 2) or exit (0):";
            while (!(cin >> op));
            switch(op){
                case 1: calc_bfv_basic(atoi(argv[1])); break;
                case 2: boot_performance(); break;
                case 0:  invalid = false; break;
                default: cout << "Unknown Mode!!!\n" << endl; invalid = false; break;
            }
        }while (invalid);
    }else{
        printf("Usage:: ./clustarexamples valid_degree_size [task mode: ]");
        return -1;
    }
    return 0;
}