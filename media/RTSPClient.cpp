#include "RTSPClient.h"
#include <string.h>
#include <cstring>
#include <cstdlib>
#include <boost/bind/bind.hpp>
#include <sstream>
#include <iostream>
namespace fantasy{
    ////////// RTSPClient implementation //////////
    RTSPClient::RTSPClient( boost::asio::io_context& ioContext,const char*progName,const char* rtspUrl):mIoContext(ioContext),
        mProgName(progName),mUrl(rtspUrl),mCseq(0){
    }
    void RTSPClient::handleWrite(RequestRecord* request,const boost::system::error_code& error,std::size_t bytes_transferred){
        if(!error.failed())
        {
            //std::cout<<"handle write success"<<std::endl;
            request->writtenLen(bytes_transferred+request->writtenLen());
            if(request->writtenLen() < strlen(request->requestBytesBuffer()))
            {
                const char* str = request->requestBytesBuffer()+request->writtenLen();
                boost::asio::async_write( *mSock,boost::asio::buffer(str,strlen(str)),boost::bind(&RTSPClient::handleWrite,this,request,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            }
            else{
                mSock->async_read_some(boost::asio::buffer(request->responseBytesBuffer(),request->responseBytesBufferLen()),boost::bind(&RTSPClient::handleRead,this,request,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            }
        }
        else
        {
            std::cout<<"handle write failure"<<std::endl;
        }
    }   
        void RTSPClient::handleRead(RequestRecord* request,const boost::system::error_code& error,std::size_t bytes_transferred){
        if(!error.failed()){
            std::cout<<"read :"<<std::endl<<request->responseBytesBuffer()<<std::endl;
            const char* readBuff = request->responseBytesBuffer()+request->readLen();
            const char* endOfReadBuff = request->responseBytesBuffer() +request->readLen()+bytes_transferred;
            request->readLen(bytes_transferred+request->readLen());
            while((readBuff < (endOfReadBuff - 3)) && (readBuff[0] != '\r' || readBuff[1] != '\n') || readBuff[2] != '\r' || readBuff[3] != '\n')
                readBuff++;
            if(readBuff < (endOfReadBuff - 3)){//接收一个完整response
                ResponseRecord* response = new ResponseRecord(request->responseBytesBuffer());
                if(request->cseq() != response->cseq()){
                    printf("cseq error : request-cseq=%d,response-cseq=%d\n",request->cseq() , response->cseq());
                }
                else if(response->statusCode() == 401){
                    mNonce = response->nonce();
                    mRealm = response->realm();
                    asyncRequest(new RequestRecord(mCseq++,"DESCRIBE",mUrl.c_str(), request->mHandler,mProgName.c_str(),mUserName.c_str(),mPasswd.c_str(),mRealm.c_str(),mNonce.c_str()));
                }
                else{
                    request->responseHandler(this,response->statusCode(),response);
                }
                delete request;
                delete response;
                return;
            }
            else{//继续接收
                if(request->responseBytesBufferLen() - request->readLen() < 512)
                {
                    request->responseBytesBufferAlloc(1024);
                }
                mSock->async_read_some(boost::asio::buffer(request->responseBytesBuffer(),request->responseBytesBufferLen()-request->readLen()),boost::bind(&RTSPClient::handleRead,this,request,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            }
        }
        else{
            std::cout<<"handleRead "<<error.message()<<"("<<error.value()<<")"<<std::endl;
        }
    }
    int RTSPClient::asyncRequest(RequestRecord* request){
       std::cout<<"request :"<<std::endl<<request->requestBytesBuffer()<<std::endl;
       boost::asio::async_write( *mSock,boost::asio::buffer(request->requestBytesBuffer(),strlen(request->requestBytesBuffer())),boost::bind(&RTSPClient::handleWrite,this,request,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));        
    }
    

    int RTSPClient::sendDescribeCommand(ResponseHandler handler) {
       return asyncRequest(new RequestRecord(mCseq++,"DESCRIBE",mUrl.c_str(), handler,mProgName.c_str(),mUserName.c_str(),mPasswd.c_str(),mRealm.c_str(),mNonce.c_str()));
    } 

    static void continueAfterDESCRIBE(RTSPClient* rtspClient,
				 int resultCode, RTSPClient::ResponseRecord* response){
                     std::cout<<"continueAfterDESCRIBE" <<std::endl;

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
        std::string url = mUrl;
        if(url.find("rtsp://") == -1){
            std::cout<<"url is error:"<<url<<std::endl;
            return -1;
        }
        url.erase(0,7);
        int pos = url.find('@');
        if(pos != -1){
            std::string tmp = url.substr(0,pos);
            url.erase(0,pos+1);
            pos = tmp.find(':');
            mUserName = tmp.substr(0,pos);
            mPasswd = tmp.substr(pos+1,-1);
        }
        if(mResolver == nullptr)
            mResolver = new boost::asio::ip::tcp::resolver(mIoContext);
         
         std::cout<<"ready call async_resolve"<<std::endl;
         
         mResolver->async_resolve(url.c_str(),"rtsp",boost::bind(&RTSPClient::handleResolve,this,boost::asio::placeholders::error,boost::asio::placeholders::results));

    }
    ////////// RTSPClient::RequestRecord implementation //////////
    RTSPClient::RequestRecord::RequestRecord(uint32_t cseq,const char* method,const char* url,ResponseHandler handler,const char* progName,const char* user,const char* passwd,const char* realm,const char* nonce):mCseq(cseq),mMethod(method),mHandler(handler),mWrittenLen(0),mReadLen(0),mResponseBytesBuffer(1024){
      
      char const* const cmdFmt =
      "%s %s RTSP/1.0\r\n"
      "CSeq: %d\r\n"
      "%s"  //authenticatorStr
      "User-Agent: %s(fantasy media v2020.09.03)\r\n"
      "%s"  //extraHeaders for example : Accept:  application/sdp
      //"%s"
      "\r\n"
      //"%s"
      ;

      if( user && passwd && realm && nonce
            && user[0] != '\0' && passwd[0] != '\0' && realm[0] != '\0' && nonce[0] != '\0'){
          mkAuthorization(user,passwd,realm,nonce);
      }

     unsigned cmdSize = strlen(cmdFmt)
      + strlen(method) + strlen(url) 
      + 20 /* max int len */
      + mAuthorization.length();
      + strlen(progName)
      + strlen(extraHeaders())
      //+ strlen(contentLengthHeader)
      //+ contentStrLen
      ;
      mRequestBytesBuffer.resize(cmdSize);
    char *cmd = mRequestBytesBuffer.data();
    sprintf(cmd, cmdFmt
	    ,method ,url
	    ,cseq
	    ,mAuthorization.c_str()
	    ,progName
        ,extraHeaders()
	    //contentLengthHeader,
	    //contentStr
        );

    }
    const char* RTSPClient::RequestRecord::extraHeaders()const{
        if(mMethod == "DESCRIBE")
            return "Accept:  application/sdp\r\n";
        else return "";
    }
    //Authorization: Digest username=“admin”, realm=“000102030405”, nonce=“59caf6cbb9d5dd0ae2a168059919f559”, uri=“rtsp://10.175.30.35”, response=“039837838f10192c7dcb98b0485265e9”
    void RTSPClient::RequestRecord::mkAuthorization(const char* user,const char* passwd,const char* realm,const char* nonce){



    }
    ////////// RTSPClient::ResponseRecord implementation //////////
    RTSPClient::ResponseRecord::ResponseRecord(const char* responseBytesBuffer){
        std::string line,tmp;
        std::stringstream responseStream(responseBytesBuffer);
        getline(responseStream,line);
        std::stringstream lineStream(line);
        lineStream >> tmp >> mStatusCode >> tmp;
        while(getline(responseStream,line)){
            if(line == "\r") continue;
            line.erase(line.find_last_not_of("\r") + 1); //去除尾换行
            //line.erase(0, line.find_first_not_of(" "));

            int pos = line.find(':');
            std::string k =  line.substr(0,pos);
            std::string v = line.substr(pos+1);
            v.erase(0, v.find_first_not_of(" ")); //去除头空白

            transform(k.begin(), k.end(), k.begin(), ::tolower);  
            
            if(k == "cseq"){
                mCseq = std::stoul(v);
            }else if(k == "www-authenticate")
            {
                //for realm
                pos = v.find(',');
                line = v.substr(0,pos);
                v.erase(0,pos+1);
                line.erase(0, line.find_first_not_of(" "));
                std::string realm = line.substr(0,line.find('='));
                transform(realm.begin(), realm.end(), realm.begin(), ::tolower);  
                if(realm == "basic realm" ){
                    pos = line.find('"');
                    mBasicRealm = line.substr(pos+1,line.find_last_of('"')-pos);
                }
                else
                {
                    pos = line.find('"');
                    mDigestRealm = line.substr(pos+1,line.find_last_not_of('"') - pos);

                    //for nonce
                    pos = v.find(',');
                    line = v.substr(0,pos);
                    v.erase(0,pos+1);
                    line.erase(0, line.find_first_not_of(" "));
                    pos = line.find('"');
                    mNonce = line.substr(pos+1,line.find_last_not_of('"')-pos);

                    //for algorithm or stale
                    while(v.length() > 0){
                        pos = v.find(',');
                        line = v.substr(0,pos);
                        if(pos == -1)
                            v.clear();
                        else
                            v.erase(0,pos+1);
                        line.erase(0, line.find_first_not_of(" "));
                        tmp = line.substr(0,line.find('='));
                        if(tmp == "algorithm"){
                            pos = line.find('"');
                            mAlgorithm = line.substr(pos+1,line.find_last_not_of('"')-pos);
                        }
                        else if(tmp == "stale"){
                            pos = line.find('"');
                            mStale = line.substr(pos+1,line.find_last_not_of('"')-pos);
                        }
                    }
                }
            }
        }
    }
}