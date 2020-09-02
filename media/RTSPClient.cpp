#include "RTSPClient.h"
#include <string.h>
#include <cstring>
#include <boost/bind/bind.hpp>
#include <iostream>
namespace fantasy{
    RTSPClient::RTSPClient( boost::asio::io_context& io_context):m_io_context(io_context){

    }
       
    int RTSPClient::sendDescribeCommand() {
    } 
    //rtsp://192.168.199.64
    void RTSPClient::handle_connect(const boost::system::error_code& error,const boost::asio::ip::tcp::endpoint& ep){
        if(!error.failed()){
            std::cout<<"handle_connect address=" <<ep.address().to_string()<<std::endl;
            std::cout<<"handle_connect aport=" <<ep.port()<<std::endl;
            sendDescribeCommand();
        }
        else{
            std::cout<<"handle_resolve "<<error.message()<<"("<<error.value()<<")"<<std::endl;
        }
        
    }
    void RTSPClient::handle_resolve(const boost::system::error_code& error,const boost::asio::ip::tcp::resolver::results_type& results){
        auto it = results.begin();
        for(auto it = results.begin();it != results.end();it++){
            std::cout<<"address=" <<it->endpoint().address().to_string()<<std::endl;
            std::cout<<"aport=" <<it->endpoint().port()<<std::endl;
        } 
        if(!error.failed())
        {
            if(m_sock == nullptr)
                m_sock = new boost::asio::ip::tcp::socket(m_io_context);
            boost::asio::async_connect(*m_sock,results, boost::bind(&RTSPClient::handle_connect,this, boost::asio::placeholders::error,boost::asio::placeholders::endpoint));
        }
        else{
            std::cout<<"handle_resolve "<<error.message()<<"("<<error.value()<<")"<<std::endl;
        }
    }
    int RTSPClient::open(const char* url){
        
        if(strncmp(url,"rtsp://",7) != 0)
            return -1;
        url+=7;
        const char* cstr_host = strstr(url,"@");
        if(cstr_host){
            const char* cstr_password =strstr(url,":");
            std::string user_name( url,cstr_password-url);
            cstr_password++;
            std::string password(cstr_password,cstr_host-cstr_password);
            cstr_host++;
        }
        else
        {
            cstr_host = url;
        }
        if(m_resolver == nullptr)
            m_resolver = new boost::asio::ip::tcp::resolver(m_io_context);
         //boost::asio::ip::tcp::resolver::results_type endpoints = 
         std::cout<<"ready call async_resolve"<<std::endl;
         boost::system::error_code ec;
         /*
         boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(cstr_host,"rtsp",ec);
         std::cout<<"resolve "<<strerror(ec.value())<<"("<<ec.value()<<")"<<std::endl;
         boost::asio::ip::tcp::socket socket(m_io_context);
         boost::asio::connect(socket, endpoints,ec);
         std::cout<<"connect "<<strerror(ec.value())<<"("<<ec.value()<<")"<<std::endl;
         */
         m_resolver->async_resolve(cstr_host,"rtsp",boost::bind(&RTSPClient::handle_resolve,this,boost::asio::placeholders::error,boost::asio::placeholders::results));

    }
}