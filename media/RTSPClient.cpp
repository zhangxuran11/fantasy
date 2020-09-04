#include "RTSPClient.h"
#include <string.h>
#include <cstring>
#include <boost/bind/bind.hpp>
#include <iostream>
namespace fantasy{
    RTSPClient::RTSPClient( boost::asio::io_context& ioContext,const char*progName,const char* rtspUrl):mIoContext(ioContext),
        mProgName(progName),mUrl(rtspUrl),mCseq(0){
    }
    void RTSPClient::handleWrite(RequestRecord* request,const boost::system::error_code& error,std::size_t bytes_transferred){
        if(!error.failed())
        {
            std::cout<<"handle write success"<<std::endl;
            request->writtenLen(bytes_transferred+request->writtenLen());
            if(request->writtenLen() < strlen(request->bytesBuffer()))
            {

                const char* str = request->bytesBuffer()+request->writtenLen();
                boost::asio::async_write( *mSock,boost::asio::buffer(str,strlen(str)),boost::bind(&RTSPClient::handleWrite,this,request,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            }
            else{
                mSock->async_read_some(boost::asio::buffer(readBuff,sizeof(readBuff)-1),boost::bind(&RTSPClient::handleRead,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            }

            
        }
        else
        {
            std::cout<<"handle write failure"<<std::endl;
        }
        
    }   
    int RTSPClient::sendRequest(RequestRecord* request){
       std::cout<<"request :"<<std::endl<<request->bytesBuffer()<<std::endl;
       boost::asio::async_write( *mSock,boost::asio::buffer(request->bytesBuffer(),strlen(request->bytesBuffer())),boost::bind(&RTSPClient::handleWrite,this,request,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));        

    }
    int RTSPClient::sendDescribeCommand(ResponseHandler handler) {
        /*
        DESCRIBE rtsp://192.168.20.136:5000/xxx666 RTSP/1.0
        CSeq: 2
        token: 
        Accept: application/sdp
        User-Agent: VLC media player (LIVE555 Streaming Media v2005.11.10) 
        */
       return sendRequest(new RequestRecord(mCseq++,"DESCRIBE",handler));
       
    } 
    void RTSPClient::handleRead(const boost::system::error_code& error,std::size_t bytes_transferred){
        if(!error.failed()){
            std::cout<<"read :"<<std::endl<<readBuff<<std::endl;
            mSock->async_read_some(boost::asio::buffer(readBuff,sizeof(readBuff)-1),boost::bind(&RTSPClient::handleRead,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
        else{
            std::cout<<"handleRead "<<error.message()<<"("<<error.value()<<")"<<std::endl;
        }
    }
    static void continueAfterDESCRIBE(RTSPClient* rtspClient,
				 int resultCode, RTSPClient::ResponseRecord* response){

                 }
    //rtsp://192.168.199.64
    void RTSPClient::handleConnect(const boost::system::error_code& error,const boost::asio::ip::tcp::endpoint& ep){
        if(!error.failed()){
            std::cout<<"handleConnect address=" <<ep.address().to_string()<<std::endl;
            std::cout<<"handleConnect aport=" <<ep.port()<<std::endl;
            //mSock->async_read_some(boost::asio::buffer(readBuff,sizeof(readBuff)-1),boost::bind(&RTSPClient::handleRead,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            
            sendDescribeCommand(continueAfterDESCRIBE);
            
        }
        else{
            std::cout<<"handleConnect "<<error.message()<<"("<<error.value()<<")"<<std::endl;
        }
    }
    void RTSPClient::handleResolve(const boost::system::error_code& error,const boost::asio::ip::tcp::resolver::results_type& results){
        auto it = results.begin();
        for(auto it = results.begin();it != results.end();it++){
            std::cout<<"address=" <<it->endpoint().address().to_string()<<std::endl;
            std::cout<<"aport=" <<it->endpoint().port()<<std::endl;
        } 
        if(!error.failed())
        {
            if(mSock == nullptr)
                mSock = new boost::asio::ip::tcp::socket(mIoContext);
            boost::asio::async_connect(*mSock,results, boost::bind(&RTSPClient::handleConnect,this, boost::asio::placeholders::error,boost::asio::placeholders::endpoint));
        }
        else{
            std::cout<<"handleResolve "<<error.message()<<"("<<error.value()<<")"<<std::endl;
        }
    }
    int RTSPClient::play(){
        const char* url = mUrl.data();
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
        if(mResolver == nullptr)
            mResolver = new boost::asio::ip::tcp::resolver(mIoContext);
         
         std::cout<<"ready call async_resolve"<<std::endl;
         
         mResolver->async_resolve(cstr_host,"rtsp",boost::bind(&RTSPClient::handleResolve,this,boost::asio::placeholders::error,boost::asio::placeholders::results));

    }
    ////////// RTSPClient::RequestRecord implementation //////////
    RTSPClient::RequestRecord::RequestRecord(uint32_t cseq,const char* method,const char* url,ResponseHandler handler,const char* progName):mCseq(cseq),mMethod(method),mHandler(handler),mWrittenLen(0){
      const char* authenticatorStr = "";
      char const* const cmdFmt =
      "%s %s RTSP/1.0\r\n"
      "CSeq: %d\r\n"
      "%s"  //authenticatorStr
      "User-Agent: %s(fantasy media v2020.09.03)\r\n"
      "%s\r\n"  //extraHeaders for example : Accept:  application/sdp
      //"%s"
      "\r\n"
      //"%s"
      ;

    unsigned cmdSize = strlen(cmdFmt)
      + strlen(method) + strlen(url) 
      + 20 /* max int len */
      + strlen(authenticatorStr)
      + strlen(progName)
      + strlen(extraHeaders())
      //+ strlen(contentLengthHeader)
      //+ contentStrLen
      ;
      mBytesBuffer.resize(cmdSize);
    char *cmd = mBytesBuffer.data();
    sprintf(cmd, cmdFmt
	    ,method ,url
	    ,cseq
	    ,authenticatorStr
	    ,progName
        ,extraHeaders()
	    //contentLengthHeader,
	    //contentStr
        );

    }
    const char* RTSPClient::RequestRecord::extraHeaders()const{
        if(mMethod == "DESCRIBE")
            return "Accept:  application/sdp";
        else return "";
    }
}