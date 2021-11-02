#include "common.h"
#include "FIFOreqchannel.h"
#include "BoundedBuffer.h"
#include "HistogramCollection.h"
#include <sys/wait.h>
using namespace std;


void patient_thread_function(int p, int t, int e, int n, vector<FIFORequestChannel *> openChannels){
    /* What will the patient threads do? */
	//patient needs to be able to create a channel for its use
	// requestChannels(openChannels); // open the channel 
	// for(int i = 0; i < n; ++i){
	// 	// make n data request to the channel

	// }
	// close the channel
}

void worker_thread_function(/*add necessary arguments*/){
    /*
		Functionality of the worker threads	
    */
}
void histogram_thread_function (/*add necessary arguments*/){
    /*
		Functionality of the histogram threads	
    */
}

int main(int argc, char *argv[]){
	vector<FIFORequestChannel*> openChannels;
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	string filename = "";
	int b = 10; // size of bounded buffer, NOTE: this is different from another variable buffercapacity/m acceptable range [100, 1000000]
	// take all the arguments first because some of these may go to the server
	int m = MAX_MESSAGE; // buffer capacity 
	int n = 1;	// [1, 15k] number of data items
	int w = 50; // [50,5k] worker threads 
	int h = 1; // histogram threads
	while ((opt = getopt(argc, argv, "f:p:t:e:b:m:n:w:h:")) != -1) {
		switch (opt) {
			case 'f':
				filename = optarg;
				break;
			case 'p':
				p =  atoi(optarg);
				break;
			case 't':
				t = atof(optarg);
				break;
			case 'e':
				e = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 'w':
				w = atoi(optarg);
				break;
			case 'h':
				h = atoi(optarg);
				break;
		}
	}

	int pid = fork ();
	if (pid < 0){
		EXITONERROR ("Could not create a child process for running the server");
	}
	if (!pid){ // The server runs in the child process
		char* args[] = {"./server", nullptr};
		if (execvp(args[0], args) < 0){
			EXITONERROR ("Could not launch the server");
		}
	}
	FIFORequestChannel channel ("control", FIFORequestChannel::CLIENT_SIDE);
	BoundedBuffer request_buffer(b);
	HistogramCollection hc;


	struct timeval start, end;
    gettimeofday (&start, 0);

    /* Start all threads here */
	

	/* Join all threads here */
    gettimeofday (&end, 0);

    // print the results and time difference
	hc.print ();
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
	
	// close all the channels 
	Request q (QUIT_REQ_TYPE);
	channel.cwrite(&q, sizeof(Request));
	// for(int i = 0; i < openChannels.size();++i){
	// 	openChannels.back()->cwrite(&q, sizeof(Request));
	// 	openChannels.pop_back();
	// }
	// client waiting for the server process, which is the child, to terminate
	wait(0);
	cout << "Client process exited" << endl;

}
