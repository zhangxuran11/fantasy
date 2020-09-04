#include "RTSPClient.h"
#include <iostream>
#include <boost/assert.hpp>
int main(int argc,const char*argv[]){
    boost::asio::io_context io_context;
    const char* url = argc >= 2 ? argv[1] : "rtsp://192.168.199.64";
    fantasy::RTSPClient* c = new fantasy::RTSPClient(io_context,argv[0],url);
    c->play();
    
    io_context.run();
    std::cout<<"bye"<<std::endl;
    return 0;
}