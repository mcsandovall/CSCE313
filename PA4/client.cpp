#include "common.h"
#include "FIFOreqchannel.h"
#include "BoundedBuffer.h"
#include "HistogramCollection.h"
#include <thread>
#include <sys/wait.h>
#include <unistd.h>
#include <mutex>
#include <pthread.h>

using namespace std;

struct server_response{ // structure to hold the response data and patient number
	double data;
	int p;

	server_response(){}
	server_response(double d, int pno) : data(d), p(pno) {}
};

void makeFile_request(BoundedBuffer * request_buffer, string filename, int64 filelen, int buffer_size){
	// this function makes all the file request and puts them in the request buffer
	FileRequest fm (0,0);
	int len = sizeof (FileRequest) + filename.size() + 1;
	char buf2 [len];

	int64 rem = filelen;
	while(rem > 0){
		fm.length = min<int64>(rem, (int64) buffer_size);
		memcpy (buf2, &fm, sizeof (FileRequest));
		strcpy (buf2 + sizeof (FileRequest), filename.c_str());
		vector<char> rq (buf2, buf2 + len);
		request_buffer->push(rq);
		rem -= fm.length;
		fm.offset += fm.length;
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
	delete dt;
}

void worker_thread_function(BoundedBuffer * requestBuffer,BoundedBuffer * responseBuffers, FIFORequestChannel * channel, int buffersize, string filename, mutex * mtx){
    /*
		Functionality of the worker threads	
		pop from the request buffer 
		send the request
		recieve the request
		push the response into respose buffer
    */
   int fperson;
   int totalBytes;
   int readSize;
   if(filename !=""){
	   fperson = open(("./received/" + filename).c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
   }
   // cast a pointer of request type
   while(true){
	   
	   vector<char> req_v = requestBuffer->pop();
	   char* command = req_v.data();
	   REQUEST_TYPE_PREFIX type = *(REQUEST_TYPE_PREFIX*) command;
	   //cout << "This is the type of request " << type << endl;
	   
	   if(type == QUIT_REQ_TYPE){
		   	// send the quit type to the server to quit the connection for that channel
			Request q = *(Request*) command;
			channel->cwrite(&q,sizeof(Request));
			delete channel;
			break;
	   }
	   if(type == DATA_REQ_TYPE){
		   DataRequest dr = *(DataRequest*) command;

		   channel->cwrite(&dr, sizeof(DataRequest));
		   double value;
		   channel->cread(&value, sizeof(double));
		 
		   // put the request data into the response struct with the patient number
		   server_response * s_r = new server_response(value,dr.person);
		   vector<char> data((char *)s_r, (char *)s_r + sizeof(server_response));

		   // add it to the reponse buffer
		   responseBuffers->push(data);

		   //delete all the pointer to prevent memeory leak
	   }

		else if(type == FILE_REQ_TYPE){
			//cout << "File request " << endl;
		   FileRequest * fr = (FileRequest*) command;

		   string file_name = command + sizeof(FileRequest);

		   const int size = sizeof(FileRequest) + file_name.size() + 1;

		   channel->cwrite(command, size);
		   char buffer[buffersize];
		   mtx->lock();
		   totalBytes = 0;
		   while(totalBytes < fr->length){
			   readSize = channel->cread(&buffer,fr->length);
			   totalBytes += readSize;
			   write(fperson, buffer, readSize);
		   }
		   mtx->unlock();
	    }
   }

   if(filename != ""){
	   close(fperson);
   }
}

void histogram_thread_function (BoundedBuffer * response_buffer, HistogramCollection *hc){
    /*
		Functionality of the histogram threads	
		h history for p patients
		Histogram should check the response buffer and then added it to the historgram
    */
   vector<char> response;
   while(true){
		// get the response 
		response = response_buffer->pop();

		// get the server_response structore
		server_response rp = *(server_response*) response.data();

		// break the loop if there is a negative value in the request
		if(rp.p == -1){
			break;
		}

		// put it in the right histogram with according to the patient number
		hc->update(rp.p,rp.data);
   }
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
	mutex * mtx = new mutex;
	int pid = fork ();
	if (pid < 0){
		EXITONERROR ("Could not create a child process for running the server");
	}
	if (!pid){ // The server runs in the child process
		char* args[] = {"./server", "-m", (char*) to_string(m).c_str() ,nullptr};
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

	if(filename == ""){ // this handles the data request

		//create patient threads
		thread patients[p];
		for(int i = 0; i < p; ++i){
			patients[i] = thread (patient_thread_function, i+1, n, &request_buffer);
			Histogram * h = new Histogram(10,-2.0,2.0);
			hc.add(h);
		}

		// make worker channels
		thread workers[w];
		for(int i = 0; i < w; ++i){
			FIFORequestChannel * n_channel = createChannel(channel);
			workers[i] =  thread (worker_thread_function, &request_buffer, &response_buffer, n_channel, m, filename, mtx);
		}

		// make histogram threads 
		thread hist[h];
		for(int i = 0; i < h; ++i){
			hist[i] = thread(histogram_thread_function, &response_buffer, &hc);
		}

		/* Join the threads */

		for(int i = 0; i < p; ++i){
			patients[i].join();
		}

		Request q (QUIT_REQ_TYPE);
		for(int i = 0; i < w; ++i){
			vector<char> d ((char*) &q, (char*)&q + sizeof(Request));
			request_buffer.push(d);
		}

		for(int i = 0; i < w; ++i){
			workers[i].join();
		}

		server_response sq (-1.0,-1);
		for(int i = 0; i < h; ++i){
			vector<char> d ((char*)&sq, (char*) &sq + sizeof(server_response));
			response_buffer.push(d);
		}

		for(int i = 0; i < h; ++i){
			hist[i].join();
		}
	}else{ // handle the filerequest
		// get the filesize
		FileRequest fm (0,0);
		int len = sizeof (FileRequest) + filename.size()+1;
		char buffer [len];
		memcpy (buffer, &fm, sizeof (FileRequest));
		strcpy (buffer + sizeof (FileRequest), filename.c_str());
		channel->cwrite (&buffer, len);  
		int64 filelen;
		channel->cread (&filelen, sizeof(int64));
		if (isValidResponse(&filelen)){
			cout << "File length is: " << filelen << " bytes" << endl;
		}

		// create file request thread
		thread fr_thread(makeFile_request, &request_buffer, filename, filelen, m);

		// make worker channels
		thread workers[w];
		for(int i = 0; i < w; ++i){
			FIFORequestChannel * n_channel = createChannel(channel);
			workers[i] =  thread (worker_thread_function, &request_buffer, &response_buffer, n_channel, m, filename, mtx);
		}

		// join the threads
		fr_thread.join();

		Request q (QUIT_REQ_TYPE);
		for(int i = 0; i < w; ++i){
			vector<char> d ((char*) &q, (char*)&q + sizeof(Request));
			request_buffer.push(d);
		}

		for(int i = 0; i < w; ++i){
			workers[i].join();
		}
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
	//delete channel; // delete pointer to the channel


	// client waiting for the server process, which is the child, to terminate
	wait(0);
	cout << "Client process exited" << endl;

}
