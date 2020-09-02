#include "RTSPClient.h"
#include <iostream>
int main(){
    boost::asio::io_context io_context;
    fantasy::RTSPClient* c = new fantasy::RTSPClient(io_context);
    c->open("rtsp://192.168.10.64");
    
    io_context.run();
    std::cout<<"bye"<<std::endl;
    return 0;
}