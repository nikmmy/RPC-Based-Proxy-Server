
#include "Something.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <unistd.h>
#include <sys/select.h>
#include <curl/curl.h>
#include <time.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <queue>

#define MAXSIZE 128*1024

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace std;

using boost::shared_ptr;

using namespace  ::Test;

struct cache_size
{
	string u;
	int size;

	friend bool operator==(const cache_size& lhs, const cache_size& rhs)
	{
		return lhs.size == rhs.size && lhs.u == rhs.u;
	}
};

struct Comp
{
	bool operator()(const cache_size& s1, const cache_size& s2)
	{
		return s1.size<s2.size;
	}
};

map <string,string> cache;
vector <string> cache_index;
int totalsize=0;
double c=0,hit=0;
vector<cache_size> vec_size;
queue<string> fifoq;


class SomethingHandler : virtual public SomethingIf {
 public:


  SomethingHandler() {

  }
  
	static void random(int s)
	{
		int r,t; 
		string data,url;
		srand(time(NULL));
		while(totalsize+s>MAXSIZE)
		{
			if(cache.empty()!=true)
			{
				t=rand();
				cout<<"rand(): "<<t<<endl;
				cout<<"\nSize of cache: "<<cache.size()<<endl;
				r=t%cache.size();
				cout<<"\nRandom index: "<<r<<endl;
				url=cache_index[r];
				cout<<"\nRemoving url: "<<url<<endl;
				data=cache[url];
				cache.erase(url);
				cache_index.erase(cache_index.begin() + r);
				totalsize=totalsize-data.length();
				cout<<"\nTotal size after evicting: "<<totalsize<<endl;
			}
			else
				break;
			
		}
	}

	static void MAXS(int s)
	{
		make_heap(vec_size.begin(),vec_size.end(),Comp());
		string data,url;
		cache_size max;
		while(totalsize+s>MAXSIZE)
		{
			if(cache.empty()!=true)
			{
				max=vec_size.front();
				url=max.u;
				pop_heap(vec_size.begin(),vec_size.end(),Comp());
				cout<<"\nRemoving url: "<<url<<endl;
				data=cache[url];
				cache.erase(url);
				vec_size.pop_back();
				push_heap(vec_size.begin(),vec_size.end(),Comp());
				totalsize=totalsize-data.length();
				cout<<"\nTotal size after evicting: "<<totalsize<<endl;
			}
			else
				break;
		}
		
	}

	static void fifo(int s)
	{
		//Declare a queue to store sequence of urls
		string to_be_deleted, temp;
		map<string,string>::iterator deletei;
		//cout<<"Total cache size"<< totalsize << endl;
		
		//adjusting s
		s = s - (MAXSIZE - totalsize);
			 
		//Delete entries in queue and cache till s is not met
		while(s>0){
				if(!fifoq.empty()){
					to_be_deleted = fifoq.front();
					cout<< "Deleting : " << to_be_deleted<<endl << "to create " ;
					fifoq.pop();
					
					//Now delete same entry from cache
					deletei = cache.find(to_be_deleted);
					temp = deletei -> second;
					cout << temp.length() <<"bytes" <<endl;
					cache.erase(deletei);
					
					//update total cache size
					s = s- temp.length();
					totalsize = totalsize - temp.length();

				}	
		}
		//return 1;
		
	} 
 
  static int write_data(char *buffer, size_t size, 
                         size_t nmemb, string *userp) {
  
	int s=0;
	if(userp!=NULL)
	{
		userp->append(buffer,size*nmemb);
		s=size*nmemb;
					
	}
	return s;
	
  }


  void ping(std::string& _return, const std::string& url)
{

  printf("ping\n");
  cout<<url<<endl;
  CURL *curl;
  CURLcode res;
  //char url[] = "http://www.google.com";
  string buf;
  time_t start,end;
  double seconds;

  time(&start);
  c++;

  /* Get a curl handle.  Each thread will need a unique handle. */
  if(cache.find(url) != cache.end())
  {
	cout<<"\nPage is in cache"<<endl;
	hit++;
  }

  else
  {

  curl = curl_easy_init();

  if(NULL != curl) {

    /* Set the URL for the operation. */
   
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	
    /* "write_data" function to call with returned data. */
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	curl_easy_setopt(curl,CURLOPT_WRITEDATA, &buf);

   
    /* Actually perform the query. */
    res = curl_easy_perform(curl);
	
	//cout<<"res: "<<res<<endl;

    /* Clean up after ourselves. */
    curl_easy_cleanup(curl);

	int s=buf.length();
	cout<<"\nTotal Size: "<<totalsize<<endl;
	cout<<"\nSize of current url: "<<s<<endl;
	if(s>MAXSIZE)
	{
		cout<<"\nPage is bigger than cache size"<<endl;		
		_return=buf;
	}
	else
	{
		if(totalsize+s<=MAXSIZE)
		{
			totalsize+=s;
			cout<<"\nPage fits in cache"<<endl;
		}
		else
		{
			cout<<"\nPage replacememnt algo called"<<endl;
			//random(s);
			MAXS(s);
			//fifo(s);
			if(totalsize+s<=MAXSIZE)
				totalsize+=s;
		}

		cache[url]=buf;
	   	cache_index.push_back(url);
		cache_size cs;
		cs.u=url;
		cs.size=s;
		vec_size.push_back(cs);
		fifoq.push(url);
		_return=cache[url];
	}
  }
  else {
    fprintf(stderr, "Error: could not get CURL handle.\n");
    exit(EXIT_FAILURE);
  }
  
}
	
	//cout<<"\nSize of cache: "<<cache.size()<<endl;
	time(&end);
	seconds=difftime(end,start);
	cout<<"\nTime to access "<<url<<" = "<<seconds<<endl;

	hit_rate();
  }

  static void hit_rate()
  {
	cout<<"\nNo. of hits: "<<hit<<endl;
	cout<<"\nNo. of cache accesses: "<<c<<endl;
	cout<<"\nHit Rate: "<<(hit/c)<<endl;
  }
};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<SomethingHandler> handler(new SomethingHandler());
  shared_ptr<TProcessor> processor(new SomethingProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

