#include "common.h"
#include "FIFOreqchannel.h"
#include "BoundedBuffer.h"
#include "HistogramCollection.h"
#include <thread>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

struct server_response{ // structure to hold the response data and patient number
	double data;
	int p;

	server_response(){}
	server_response(double d, int pno) : data(d), p(pno) {}
};

void makeFile_request(int64 filesize,int buffersize, FileRequest * file_request, BoundedBuffer * requestBuffer){
	// this function makes all the file request and puts them in the request buffer
	while(filesize > 0){
		file_request->length = min<int64>(filesize, (int64) buffersize);
		vector<char> request((char*) file_request, (char*) file_request + sizeof(FileRequest));
		requestBuffer->push(request);
		filesize -= file_request->length;
		file_request->offset += file_request->length;
	}
}

FIFORequestChannel * createChannel(FIFORequestChannel * channel){
	// get a new request channel and return the pointer
	Request rq(NEWCHAN_REQ_TYPE);
	channel->cwrite(&rq,sizeof(Request));
	char buffer [30];
	channel->cread(&buffer, sizeof(buffer));
	FIFORequestChannel * new_channel = new FIFORequestChannel(buffer,FIFORequestChannel::CLIENT_SIDE);
	return new_channel;
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

void worker_thread_function(BoundedBuffer * requestBuffer,BoundedBuffer * responseBuffers, FIFORequestChannel * channel, int buffersize){
    /*
		Functionality of the worker threads	
		pop from the request buffer 
		send the request
		recieve the request
		push the response into respose buffer
    */
   vector<char> request, response;
   // create a channel
   FIFORequestChannel * new_channel = createChannel(channel);
   double resp;
   char buffer [buffersize];

   // cast a pointer of request type
   while(true){
	   // get the request from the request_buffer
	   request = requestBuffer->pop();

	   // get the command from the vector
	   char* command = request.data();

	   // get the command type
	   REQUEST_TYPE_PREFIX * type = (REQUEST_TYPE_PREFIX*)command;

	   // check to see if the request type is an quit message type 
	   if(*type == QUIT_REQ_TYPE){
		   	// send the quit type to the server to quit the connection for that channel
			new_channel->cwrite(command,sizeof(Request));
			delete new_channel;
			break;
	   }

	   if(*type == DATA_REQ_TYPE){
		   // send the request, get the data and put it in the response buffer
		   DataRequest * dr = (DataRequest*) command;
		   new_channel->cwrite(dr, sizeof(DataRequest));
		   new_channel->cread(&resp, sizeof(double));

		   // put the request data into the response struct with the patient number
		   server_response * s_r = new server_response(resp,dr->person);
		   vector<char> data((char *)s_r, (char *)s_r + sizeof(server_response));

		   // add it to the reponse buffer
		   responseBuffers->push(data);

		   //delete all the pointer to prevent memeory leak
		   delete dr, s_r;
	   }

	   if(*type == FILE_REQ_TYPE){
		   // send the request for the file and get it to work
		   FileRequest * fr = (FileRequest*) command;
		   channel->cwrite(fr,fr->length);
	   }

	   // delete the pointer to prevent memory leak
	   delete command, type;
   }

}

void histogram_thread_function (BoundedBuffer * response_buffer, HistogramCollection hc){
    /*
		Functionality of the histogram threads	
		h history for p patients
		Histogram should check the response buffer and then added it to the historgram
    */

   vector<char> response;
   // pop once to determine the patient number to handle with this thread
   int pno;

	
}

int main(int argc, char *argv[]){
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
	FIFORequestChannel * channel = new FIFORequestChannel("control",FIFORequestChannel::CLIENT_SIDE);

	// make a request buffer with the buffer size
	BoundedBuffer request_buffer(b);

	// make a responde buffer 
	BoundedBuffer response_buffer(b);

	HistogramCollection hc;

	struct timeval start, end;
    gettimeofday (&start, 0);

	if(filename!= ""){ // this mean we have a file request
		//query the size of the file
		FileRequest fm (0,0);
		int len = sizeof (FileRequest) + filename.size()+1;
		char buf [len];
		memcpy (buf, &fm, sizeof (FileRequest));
		strcpy (buf + sizeof (FileRequest), filename.c_str());
		channel->cwrite (&buf, len);  
		int64 filelen;
		channel->cread (&filelen, sizeof(int64));
		if (isValidResponse(&filelen)){
			cout << "File length is: " << filelen << " bytes" << endl;
		}

		// make a thread that makes and puts all the request into the request buffer
		thread file_worker(makeFile_request,filelen, b , (FileRequest*) buf, &request_buffer);

		file_worker.join();
	}

    /* Start all threads here */
	thread patients[p];
	for(int i = 0; i < p;++i){
		patients[i] = thread(patient_thread_function,i+1,n,&request_buffer);
	}

	thread workers[w];
	for(int i = 0; i < w; ++i){
		workers[i] = thread(worker_thread_function,&request_buffer, &response_buffer, channel, b);
	}

	thread hist[h];
	for(int i = 0; i < h;++i){
		hist[i] = thread(histogram_thread_function,&response_buffer,hc);
	}


	/* Join all threads here */
	for (int i = 0; i < p;++i){
		patients[i].join();
	}

	for(int i = 0; i < w; ++i){
		workers[i].join();
	}

	for(int i = 0; i < h;++i){
		hist[i].join();
	}

	// kill the worker thread
	for(int i  = 0; i < h; ++i){
		Request q (QUIT_REQ_TYPE);
		vector<char> rq ((char*) &q, (char*) &q  + sizeof(Request));
		request_buffer.push(rq);
	}
	
	
    gettimeofday (&end, 0);

    // print the results and time difference
	hc.print ();
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
	
	// close all the channels 
	Request q (QUIT_REQ_TYPE);
	channel->cwrite(&q, sizeof(Request));
	delete channel; // delete pointer to the channel


	// client waiting for the server process, which is the child, to terminate
	wait(0);
	cout << "Client process exited" << endl;

}
