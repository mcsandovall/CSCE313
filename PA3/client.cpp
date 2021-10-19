#include "common.h"
#include "FIFOreqchannel.h"
#include <sys/wait.h>
using namespace std;

void gethousandredRecords(string filename, int patient);

int main(int argc, char *argv[]){
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	string testfilename = "";
	string filename = "";
	// take all the arguments first because some of these may go to the server
	while ((opt = getopt(argc, argv, "f:p:t:e:h:")) != -1) {

		switch (opt) {
			case 'f':
				filename = optarg;
				break;
			case 'p': // this is the person number
				p = atoi(optarg);
				break;
			case 't': // this is for the time
				t = atof(optarg);
				break;
			case 'e': // this is the ecg number 
				e = atoi(optarg);
				break;
			case 'h':// i will use this as the case for the 100 request
				testfilename = optarg;
				gethousandredRecords(testfilename, p);
				exit(0); // after
			case 'c': // requesting a new channel
				break;
			case 'm': // this is to change the buffer size 
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
	FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
	DataRequest d (p, t, e);
	chan.cwrite (&d, sizeof (DataRequest)); // question
	double reply;
	chan.cread (&reply, sizeof(double)); //answer
	if (isValidResponse(&reply)){
		cout << "For person " << p <<", at time " << t << ", the value of ecg "<< e <<" is " << reply << endl;
	}

	
	/* this section shows how to get the length of a file
	you have to obtain the entire file over multiple requests 
	(i.e., due to buffer space limitation) and assemble it
	such that it is identical to the original*/
	FileRequest fm (0,0);
	int len = sizeof (FileRequest) + filename.size()+1;
	char buf2 [len];
	memcpy (buf2, &fm, sizeof (FileRequest));
	strcpy (buf2 + sizeof (FileRequest), filename.c_str());
	chan.cwrite (buf2, len);  
	int64 filelen;
	chan.cread (&filelen, sizeof(int64));
	if (isValidResponse(&filelen)){
		cout << "File length is: " << filelen << " bytes" << endl;
	}
	
	
	// closing the channel    
    Request q (QUIT_REQ_TYPE);
    chan.cwrite (&q, sizeof (Request));
	// client waiting for the server process, which is the child, to terminate
	wait(0);
	cout << "Client process exited" << endl;

}

void gethousandredRecords(string filename, int patient){
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
	// create the channel for the request
	FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

	struct timeval start, end;
	gettimeofday(&start, NULL);

	// open the file 
	ofstream ofs("received/"+filename);
	if(!ofs.is_open()){
		throw invalid_argument("File could not open" + filename);
		return;
	}

	double time = 0; // from file time increase by 0.04
	double read;
	cout << "Process Started ..." << endl;
	while ( time < 40.000){ // last time is 59.996
		ofs << time << ",";

		DataRequest d1 = DataRequest(patient, time, 1);
		DataRequest d2 = DataRequest(patient, time, 2);

		char * buffer1 = new char[sizeof(double)]; // size of the reply
		char * buffer2 = new char[sizeof(double)];

		// send the request for the data 
		chan.cwrite(&d1, sizeof(DataRequest));
		chan.cread(&read, sizeof(double));

		if (isValidResponse(&read)){
			ofs << read << ",";
		}

		chan.cwrite(&d2, sizeof(DataRequest));
		chan.cread(&read, sizeof(double));

		if (isValidResponse(&read)){
			ofs << read << "\n";
		}
		time += 0.04;
	}
	ofs.close();

	// after all the request are sent and recieved get the time
	gettimeofday(&end,NULL);

	double requestTime = end.tv_sec - start.tv_sec;

	cout << "Process Ended, Time taken for 1000 request: " << requestTime << endl;

	// closing the channel    
    Request q (QUIT_REQ_TYPE);
    chan.cwrite (&q, sizeof (Request));
	// client waiting for the server process, which is the child, to terminate
	wait(0);
	cout << "Client process exited" << endl;
}

void RequestFile(string filename){
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

	FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

	FileRequest fm (0,0);
	int len = sizeof (FileRequest) + filename.size()+1;
	char buf [len];
	memcpy (buf, &fm, sizeof (FileRequest));
	strcpy (buf + sizeof (FileRequest), filename.c_str());
	chan.cwrite (buf, len);  
	int64 filelen;
	chan.cread (&filelen, sizeof(int64));
	if (isValidResponse(&filelen)){
		cout << "File length is: " << filelen << " bytes" << endl;
	}

	// start the time for the file transfer 
	struct timeval start, end;
	gettimeofday(&start,NULL);

	FILE *file = fopen(filename.c_str(),"wb");
	cout << "File transfer started ..." << endl;

	for(int i = 0; i < filelen;i += MAX_MESSAGE){
		FileRequest f (i, MAX_MESSAGE);
		memcpy (buf, &f, sizeof (FileRequest));
		strcpy (buf + sizeof (FileRequest), filename.c_str());
		chan.cwrite (buf, len);

		int64 *cont =  new int64;
		chan.cread(&cont, sizeof(int64));
		fwrite(cont,sizeof(int64), sizeof(cont), file);
	}
	fclose(file);

	// get the end time after the file trasfer
	gettimeofday(&end,NULL);
	double requestTime = end.tv_sec - start.tv_sec;
	cout << "File transfer ended, Time taken: " << requestTime << endl;

	// closing the channel    
    Request q (QUIT_REQ_TYPE);
    chan.cwrite (&q, sizeof (Request));
	// client waiting for the server process, which is the child, to terminate
	wait(0);
	cout << "Client process exited" << endl;
}