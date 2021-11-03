#include "common.h"
#include "FIFOreqchannel.h"
#include "BoundedBuffer.h"
#include "HistogramCollection.h"
#include <thread>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

void createChannel(vector<FIFORequestChannel*> openChannels){
	FIFORequestChannel * channel = openChannels.front();
	Request rq (NEWCHAN_REQ_TYPE);
	channel->cwrite(&rq, sizeof(Request));
	char buffer [30];
	channel->cread(&buffer, sizeof(buffer));
	openChannels.push_back( new FIFORequestChannel (buffer, FIFORequestChannel::CLIENT_SIDE));
	channel = nullptr;
	delete channel;
}

void patient_thread_function(int p, int n, BoundedBuffer * requestBuffer){
    /* What will the patient threads do? */
	//  the patient needs to send the data to the request buffer, for all n datapoints
	// add all the arguments to the vector
	DataRequest * dt = new DataRequest(p, 0.0, 1);
	
	for (int i = 0; i < n; ++i){
		vector<char> request((char *)dt, (char *)dt + sizeof(DataRequest));
		requestBuffer->push(request);
		dt->seconds += 0.004;
	}
	
}

void worker_thread_function(BoundedBuffer * requestBuffer,BoundedBuffer * responseBuffers, vector<FIFORequestChannel*> openChannels, int p){
    /*
		Functionality of the worker threads	
		pop from the request buffer 
		send the request
		recieve the request
		push the response into respose buffer
    */
   vector<char> request, response;
   // create a channel 
   createChannel(openChannels);
   FIFORequestChannel * channel = openChannels.back(); // get the newest created channel
   double resp;

   // cast a pointer of request type
//    while(true){

// 	   //check for the message type // vector of size datarquest 
// 	   request = requestBuffer->pop();
// 	   // make the request 
// 	   DataRequest dr ( (int) request[0], (double) request[1], (int) request[2]);
	   
// 	   // send the request
// 	   channel->cwrite(&dr, sizeof(DataRequest));

// 	   // get the data
// 	   channel->cread(&resp, sizeof(double));

// 	   // put it in the response buffer
// 	   response.push_back((request[0])); // add the patient number with the reponse
// 	   response.push_back( (char) resp);

// 	   responseBuffers->push(response);
//    }

}
void histogram_thread_function (BoundedBuffer response_buffer, HistogramCollection hc){
    /*
		Functionality of the histogram threads	
		Histogram should check the response buffer and then added it to the historgram
    */

}

int main(int argc, char *argv[]){
	vector<FIFORequestChannel*> openChannels;
	int opt;
	int p = 1;
	string filename = "";
	int b = 10; // size of bounded buffer, NOTE: this is different from another variable buffercapacity/m acceptable range [100, 1000000]
	// take all the arguments first because some of these may go to the server
	int m = MAX_MESSAGE; // buffer capacity 
	int n = 1;	// [1, 15k] number of data items
	int w = 50; // [50,5k] worker threads 
	int h = 1; // histogram threads
	while ((opt = getopt(argc, argv, "f:p:b:m:n:w:h:")) != -1) {
		switch (opt) {
			case 'f':
				filename = optarg;
				break;
			case 'p':
				p =  atoi(optarg);
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
	//make the control channel 
	openChannels.push_back( new FIFORequestChannel( "control", FIFORequestChannel::CLIENT_SIDE));

	BoundedBuffer request_buffer(b);

	// also make the response buffers make the response buffers without the size
	BoundedBuffer * respose_buffer = new BoundedBuffer[p];
	for(int i = 0; i < p;++i){
		respose_buffer[i] = BoundedBuffer(b);
	}

	HistogramCollection hc;


	struct timeval start, end;
    gettimeofday (&start, 0);

    /* Start all threads here */
	thread patients[p];
	for(int i = 0; i < p;++i){
		patients[i] = thread(patient_thread_function,i+1,n,&request_buffer);
	}

	thread workers[w];
	for(int i = 0; i < w; ++i){
		workers[i] = thread(worker_thread_function,)
	}


	/* Join all threads here */
	for (int i = 0; i < p;++i){
		patients[i].join();
	}
	
	
    gettimeofday (&end, 0);

    // print the results and time difference
	hc.print ();
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
	
	// close all the channels 
	Request q (QUIT_REQ_TYPE);
	for(int i = 0; i < openChannels.size();++i){
		openChannels.back()->cwrite(&q, sizeof(Request));
		openChannels.pop_back();
	}

	// delete the pointers to the threads

	// client waiting for the server process, which is the child, to terminate
	wait(0);
	cout << "Client process exited" << endl;

}
