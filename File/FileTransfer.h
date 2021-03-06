#ifndef FILETRANSFER
#define FILETRANSFER

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/endian/arithmetic.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <mutex>
#include <string.h>
#include "TCPSocket/TCPSocket.h"
#include "define.h"
//#include "TCPSocket.h"
//#include "../../define.h"

using namespace std;


class FileTransfer {
private:
	boost::filesystem::path source;
	string ip;
	vector<uint16_t> ports;

	atomic<bool> success;

	atomic<uint64_t> overallSize; 
	atomic<uint64_t> overallSent;
	atomic<std::chrono::time_point<std::chrono::system_clock>> transferStart;

	vector<TCPSocket> fileSockets;

	mutex rdi_protection;
	boost::filesystem::recursive_directory_iterator rdi;
	boost::filesystem::recursive_directory_iterator end_rdi;

	vector<thread> transferThreads;

	/*Based on results from dirSize and exploreDir, calls the createDirectoryPacket to send the dir tree. */
	bool sendDir();
	/*To be called in a separate thread, sends a single file.
	 *If the transmission is one file only, it will send it. If it is a directory, 
	 *it will search for each file in the directory tree. */
	void sendFile(uint16_t i);

	/*Explore the dir to find the overall size, given by the sum of the file size.*/
	uint64_t dirSize();
	/*Explore the dir and all subdirectories and saves the path in a proper packet.*/
	vector<string> exploreDir();

	/*Creates the packet with all the directories according to the protocol. */
	vector<char> createDirectoryPacket(vector<string> tree);

	/*Send transfer request*/
	void sendTransferRequest( TCPSocket& s );
	
	/*Based on the hardware concurrency, trades the number of ports to be used.
	 *It asks for a certain number, if the receiver can't hold them, it will send
	 *a counter-proposal that will be accepted.*/
	void tradePort( TCPSocket& s );

	/*Both sockets will open a data connection for each traded port.*/
	void prepareSockets();

public: 
	/*ipAddress of an active host. If not active, an exception is thrown.
	 *source path of the file or of the directory. If it does not exist,
	 *an exception is thrown.*/
	FileTransfer( string ipAddress, boost::filesystem::path source );
	~FileTransfer();

	/*Returns the progress (0-99), the FT_COMPLETE or the FT_ERROR defined in define.h
	  Returns the estimated time in s and the current speed in MBps.*/
	void getStatus(transferStatus& t);
	
	/*Start the transfer. Returns 0 if any error happens or the stop() method is invoked.*/
	bool transfer();
	/*Clear and thread-safe way to stop the transfer.*/
	void stop(); 



};


#endif
