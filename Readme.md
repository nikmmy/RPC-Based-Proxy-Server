README


The server code was automatically generated from the thrift file(example_thrift_file.thrift)
by using the command:
thrift --gen cpp example_thrift_file.thrift


The server code is there in a file called “Something_server.cpp” and the client code in a file called “Something_client.cpp”. All the server code as well as the caching code is in the same file. The cache replacement algorithms have been implemented as three separate functions in the same file: fifo(), MAXS() and random() respectively.


To run a particular algorithm, we comment out the function calls corresponding to the other two algorithms. Also, the cache size has been defined by the pre-processor directive 


                                #define MAXSIZE 128*1024


and can be changed to increase or decrease the cache size.


1) To build the code, from the gen-cpp folder, simply run:
 
        make 


This should create the server and client executables. 


2) To start the server, run the command:


./something_server


3) To start the client, run the command:


./something_client


The client read URLs from a file called “URLs.txt” line by line and then sends the requests one by one to the server. All changes in workloads need to be made to this file.
