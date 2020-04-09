/*
    Author: Linbin Yang @ Clustar.ai
*/

#include "common.h"

int main(int argc, char** argv){
#ifdef SEAL_VERSION
    cout << "Microsoft SEAL version: " << SEAL_VERSION << endl;
#endif
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
            case 1: calc_bfv_basic(); break;
            case 2: muti_core_runner(); break;
            case 0:  invalid = false; break;
            default: cout << "Unknown Mode!!!\n" << endl; invalid = false; break;
        }
    }while (invalid);
    return 0;
}