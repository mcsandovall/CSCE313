#include "common.h"
#include "FIFOreqchannel.h"
#include <sys/wait.h>
using namespace std;

void gethousandredRecords(string filename, int patient);
string charToString(char array[]);
void fileTransfer(string filename, int buffercapacity);

int main(int argc, char *argv[]){

	int opt;
	int bufferSize = MAX_MESSAGE;
	int p = 1;
	double t = 0.0;
	int e = 1;
	bool thousandRequest = false;
	bool filetransfer = false;
	bool requestChan = false;
	vector<FIFORequestChannel*> OpenChannel;
	string Tfile = "";
	string filename = "";
	// take all the arguments first because some of these may go to the server
	while ((opt = getopt(argc, argv, "f:p:t:e:h:cm:")) != -1) {
		switch (opt) {
			case 'f':
				filename = optarg;
				filetransfer = true;
				break;
			case 'p':
				p = atoi(optarg);
				break;
			case 't':
				t = atof(optarg);
				break;
			case 'e':
				e = atoi(optarg);
				break;
			case 'h':
				thousandRequest = true;
				Tfile = optarg;
				break;
			case 'c':
				requestChan = true;
				break;
			case 'm':
				bufferSize = atoi(optarg);
				break;
		}
	}

	// Request for thousand Data Points - --- - - - - - - - -- - -
	if(thousandRequest){
		gethousandredRecords(Tfile,p);
		exit(0);
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

	if(filetransfer){
		fileTransfer(filename,bufferSize);
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
	// FileRequest fm (0,0);
	// int len = sizeof (FileRequest) + filename.size()+1;
	// char buf2 [len];
	// memcpy (buf2, &fm, sizeof (FileRequest));
	// strcpy (buf2 + sizeof (FileRequest), filename.c_str());
	// chan.cwrite (buf2, len);  
	// int64 filelen;
	// chan.cread (&filelen, sizeof(int64));
	// if (isValidResponse(&filelen)){
	// 	cout << "File length is: " << filelen << " bytes" << endl;
	// }
	
	
	// closing the channel    
    Request q (QUIT_REQ_TYPE);
	// for(int i = 0; i < OpenChannel.size();++i){
	// 	OpenChannel[i]->cwrite(&q, sizeof(Request));
	// }
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

string charToString(char array[]){
	int len =  sizeof(array) / sizeof(char);
	string s;
	for (int i = 0; i < len;++i){
		s+= array[i];
	}
	return s;
}

void fileTransfer(string filename, int buffercapacity){
	// get the filesize
	FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

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
	filename = filename;

	struct timeval start, end;
	gettimeofday(&start,NULL);

	ofstream ofs("received/"+filename, fstream::binary);
	int offset = 0;
	
	while (offset < filelen){
		FileRequest fr(0,buffercapacity);
		int package = sizeof(FileRequest) + filename.size() + 1;
		char buff[package];
		memcpy (buff, &fr, sizeof(FileRequest));
		strcpy (buff  + sizeof(FileRequest), filename.c_str() + '\0');
		chan.cwrite(buff,package);

		char data[buffercapacity];
		chan.cread(&data,buffercapacity);
		ofs.write(data,buffercapacity);
		offset += buffercapacity;
		if(offset - package < buffercapacity){
			buffercapacity = offset - package;
		}
		//cout << "Progress.. " << (ceil(offset/filelen))*100 <<"%" << endl;
	}
	ofs.close();
	gettimeofday(&end,NULL);
	double Processtime =  end.tv_sec - start.tv_sec;
	cout << "File transfer finished time: " << Processtime << endl;
	Request q (QUIT_REQ_TYPE);
	chan.cwrite(&q, sizeof(Request));
}

