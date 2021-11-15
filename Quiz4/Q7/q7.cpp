#include <iostream>
#include <thread>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include "Semaphore.h"

using namespace std;
/*
Producer and consumer problem:
There are A, B, C threads trying to run
First 1 instance of A has to run, 2 instances of B have to run, and one instance of C has to run
Define the correct number of semaphores
*/

#define BNUM 2

Semaphore aDone(0), bDone(0), cDone(1), mtx(1);
int bcount = 0;

void ThreadA(){
    while(true){
        cDone.P(); // wait until c is empty

        mtx.P();
        // operation goes here
        cout << "This is thread A" << endl;
        mtx.V();

        mtx.P();
        for(int i = 0; i < BNUM;++i){
            aDone.V();
        }
        mtx.V();
    }
}

void ThreadB(){
    while(true){
        aDone.P(); // wait until A threads are done

        mtx.P();
        // consumer operation goes here
        cout << "This is thread B" << endl;
        mtx.V();

        mtx.P();
        ++bcount;
        if(bcount == 2){
            bDone.V();
            bcount = 0;
        }
        mtx.V();
    }
}

void ThreadC(){
    while(true){
        bDone.P();

        mtx.P();
        // producer operation goes here
        cout << "This is thread C" << endl;
        mtx.V();

        mtx.P();
        cDone.V();
        mtx.V();
    }
}

int main(){
    int num_threads = 100;

    thread thread_A[num_threads], thread_B[num_threads], thread_C[num_threads];
    for(int i = 0; i < num_threads;++i){
        thread_A[i] = thread (ThreadA);
        thread_B[i] = thread (ThreadB);
        thread_C[i] = thread (ThreadC);
    }

    for(int i = 0; i < num_threads;++i){
        thread_A[i].join();
    }

    for(int i = 0; i < num_threads;++i){
        thread_B[i].join();
    }

    for(int i = 0; i < num_threads;++i){
        thread_C[i].join();
    }
    return 0;
}