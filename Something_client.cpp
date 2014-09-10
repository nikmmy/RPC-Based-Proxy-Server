#include "Something.h"  // As an example
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace Test;

int main(int argc, char **argv) {
	boost::shared_ptr<TSocket> socket(new TSocket("localhost", 9090));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SomethingClient client(protocol);
	transport->open();
	string str, url;
	/*cout<<"Enter the url: ";
	cin>>url;
	cout<<"URL: "<<url;*/
  	ifstream myfile ("URLs.txt");
  	if (myfile.is_open())
  	{
    		while ( getline (myfile,url) )
    		{
		      cout << "URL:"<<url << endl;
		      client.ping(str,url);
		      cout<<"\nData:"<<endl<<str<<endl;
		}
		myfile.close();
	}

  	else 
		cout << "Unable to open file"; 
	

	transport->close();
	return 0;
 }
