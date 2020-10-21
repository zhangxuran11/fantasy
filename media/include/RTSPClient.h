#ifndef RTSP_CLIENT_H
#define RTSP_CLIENT_H
#include <boost/asio.hpp>
#include <cstring>
#include <vector>
namespace fantasy{
    class RTSPClient{
        public:
        class SDPMedia{
            public:
            SDPMedia();

        };
        class SDPParser{
            public:
            SDPParser(const char* sdpStr);
        };
        class ResponseRecord{
            int mStatusCode;
            uint32_t mCseq;
            std::string  mBasicRealm;
            std::string mDigestRealm;
            std::string mNonce;
            std::string mAlgorithm;
            std::string mStale;
            
            public:
            ResponseRecord(const char* responseBytesBuffer);
            uint32_t cseq()const{return mCseq;}
            int statusCode()const{return mStatusCode;}
            const char* nonce()const { return mNonce.c_str();}
            const char* realm()const { return mDigestRealm.c_str(); }
        };
        typedef void (*ResponseHandler)(RTSPClient* rtspClient,
				 int resultCode, ResponseRecord* response);
        class RequestRecord{
            uint32_t mCseq;
            std::string mMethod;
            std::string mUrl;

            uint32_t mWrittenLen;
            uint32_t mReadLen;
            std::vector<char> mRequestBytesBuffer;
            std::vector<char> mResponseBytesBuffer;
            std::string mAuthorization;

            void mkAuthorization(const char* user,const char* passwd,const char* realm,const char* nonce);
            public:
                ResponseHandler mHandler;
                RequestRecord(uint32_t cseq,const char* method,const char* url,ResponseHandler handler,const char* progName,const char* user = nullptr,
                                const char* passwd = nullptr,const char* realm = nullptr,const char* nonce = nullptr);
                const char* method() const { return mMethod.c_str(); }
                const char* extraHeaders()const;
                uint32_t cseq()const{ return mCseq;}
                uint32_t writtenLen()const{return mWrittenLen;}
                void writtenLen(uint32_t len){mWrittenLen = len;}
                uint32_t readLen()const{return mReadLen;}
                void readLen(uint32_t len){mReadLen = len; mResponseBytesBuffer[mReadLen] = '\0'; }
                const char* requestBytesBuffer()const{return mRequestBytesBuffer.data();}
                char* responseBytesBuffer() {return mResponseBytesBuffer.data();}
                size_t responseBytesBufferLen()const{return mResponseBytesBuffer.size()-1;}
                void responseBytesBufferAlloc(size_t len) { mResponseBytesBuffer.resize(mResponseBytesBuffer.size()+len); }
                void responseHandler(RTSPClient* rtspClient, int resultCode, ResponseRecord* response) { if(mHandler) mHandler(rtspClient,resultCode,response); }
                
        };
        RTSPClient(boost::asio::io_context& ioContext,const char*progName,const char* rtspUrl);
        int play();
        private:
        boost::asio::io_context& mIoContext;
        std::string mProgName;
        boost::asio::ip::tcp::resolver* mResolver;
        boost::asio::ip::tcp::socket* mSock;
        std::string mUrl;
        std::string mUserName;
        std::string mPasswd;
        std::string mServerHost;
        std::string mRealm;
        std::string mNonce;
        uint32_t mCseq;
        char mWriteBuff[1024];
        char mReadBuff[1024];

        void handleRead(RequestRecord* request,const boost::system::error_code& error,std::size_t bytesTransferred);
        void handleWrite(RequestRecord* request,const boost::system::error_code& error,std::size_t bytesTransferred);
        void handleConnect(const boost::system::error_code& error,const boost::asio::ip::tcp::endpoint& ep);
        void handleResolve(const boost::system::error_code& error,const boost::asio::ip::tcp::resolver::results_type& results);

        int asyncRequest(RequestRecord* request);
        
        int sendDescribeCommand(ResponseHandler handler);
        

    };

};


#endif//RTSP_CLIENT_H
