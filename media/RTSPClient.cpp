#include "RTSPClient.h"
#include <string.h>
#include <cstring>
#include <boost/bind/bind.hpp>
#include <iostream>
namespace fantasy{
    RTSPClient::RTSPClient( boost::asio::io_context& ioContext,const char*progName,const char* rtspUrl):mIoContext(ioContext),
        mProgName(progame),mUrl(rtspUrl),mCseq(0){
    }
    void RTSPClient::handle_write(const boost::system::error_code& error,std::size_t bytes_transferred){
        if(!error.failed())
        {
            std::cout<<"handle write success"<<std::endl;
        }
        else
        {
            std::cout<<"handle write failure"<<std::endl;
        }
        
    }   
    int RTSPClient::sendDescribeCommand() {
        /*
        DESCRIBE rtsp://192.168.20.136:5000/xxx666 RTSP/1.0
        CSeq: 2
        token: 
        Accept: application/sdp
        User-Agent: VLC media player (LIVE555 Streaming Media v2005.11.10) 
        */
       sprintf(write_buff,
       "%s %s RTSP/1.0\r\n"
       "CSeq: %d\r\n"
       "Accept:  application/sdp\r\n"
       "User-Agent: fantasy media (v2020.09.03)\r\n\r\n",
       "DESCRIBE",m_url.c_str(),++m_cseq
       );
       BOOST_ASSERT(strlen(write_buff)<sizeof(write_buff));
       std::cout<<"request :"<<std::endl<<write_buff<<std::endl;
       boost::asio::async_write( *m_sock,boost::asio::buffer(write_buff,strlen(write_buff)),boost::bind(&RTSPClient::handle_write,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));        
    } 
    void RTSPClient::handle_read(const boost::system::error_code& error,std::size_t bytes_transferred){
        if(!error.failed()){
            std::cout<<"read :"<<std::endl<<read_buff<<std::endl;
            m_sock->async_read_some(boost::asio::buffer(read_buff,sizeof(read_buff)-1),boost::bind(&RTSPClient::handle_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
        else{
            std::cout<<"handle_read "<<error.message()<<"("<<error.value()<<")"<<std::endl;
        }
    }
    //rtsp://192.168.199.64
    void RTSPClient::handle_connect(const boost::system::error_code& error,const boost::asio::ip::tcp::endpoint& ep){
        if(!error.failed()){
            std::cout<<"handle_connect address=" <<ep.address().to_string()<<std::endl;
            std::cout<<"handle_connect aport=" <<ep.port()<<std::endl;
            m_sock->async_read_some(boost::asio::buffer(read_buff,sizeof(read_buff)-1),boost::bind(&RTSPClient::handle_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            //boost::asio::async_read_some(*m_sock,boost::asio::buffer(read_buff,1),boost::bind(&RTSPClient::handle_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            sendDescribeCommand();
            
        }
        else{
            std::cout<<"handle_connect "<<error.message()<<"("<<error.value()<<")"<<std::endl;
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
    int RTSPClient::play(){
        const char* url = m_url.data();
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
         
         std::cout<<"ready call async_resolve"<<std::endl;
         
         m_resolver->async_resolve(cstr_host,"rtsp",boost::bind(&RTSPClient::handle_resolve,this,boost::asio::placeholders::error,boost::asio::placeholders::results));

    }
}