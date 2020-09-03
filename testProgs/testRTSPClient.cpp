#include "RTSPClient.h"
#include <iostream>
int main(int argc,const char*argv[]){
    boost::asio::io_context io_context;
    fantasy::RTSPClient* c = new fantasy::RTSPClient(io_context,argv[0],"rtsp://192.168.10.64");
    c->play();
    
    io_context.run();
    std::cout<<"bye"<<std::endl;
    return 0;
}