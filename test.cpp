#include <iostream>
#include <unistd.h>
#include <random>
#include <fstream>

using namespace std;

int main(int argv, char **argc){
    cout << argc[1] << endl;
    string path = "./";
    path = path + to_string(getpid());
    ofstream LogVVV(path);
    int count = 5;
    while (count -- > 0){
        random_device rd;
        LogVVV << rd() << endl;
        sleep(1);
    }
    LogVVV.close();
}