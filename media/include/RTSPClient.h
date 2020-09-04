#ifndef RTSP_CLIENT_H
#define RTSP_CLIENT_H
#include <boost/asio.hpp>
#include <cstring>
#include <vector>
namespace fantasy{
    class RTSPClient{
        public:
        
        class ResponseRecord{

        };
        typedef void (ResponseHandler)(RTSPClient* rtspClient,
				 int resultCode, ResponseRecord* response);
        class RequestRecord{
            uint32_t mCseq;
            std::string mMethod;
            ResponseHandler mHandler;
            std::vector<char> mBytesBuffer;
            uint32_t mWrittenLen;
            public:
                RequestRecord(uint32_t cseq,const char* method,const char* url,ResponseHandler handler,const char* progName);
                const char* method() const { return mMethod.c_str(); }
                const char* extraHeaders()const;
                uint32_t cseq()const{ return mCseq;}
                uint32_t writtenLen()const{return mWrittenLen;}
                void writtenLen(uint32_t len){mWrittenLen = len;}
                const char* bytesBuffer()const{return mBytesBuffer.data()};
        };
        RTSPClient(boost::asio::io_context& ioContext,const char*progName,const char* rtspUrl);
        int play();
        private:
        std::string mProgName;
        boost::asio::io_context& mIoContext;
        boost::asio::ip::tcp::resolver* mResolver;
        boost::asio::ip::tcp::socket* mSock;
        std::string mUrl;
        uint32_t mCseq;
        char mWriteBuff[1024];
        char mReadBuff[1024];

        void handleRead(const boost::system::error_code& error,std::size_t bytesTransferred);
        void handleWrite(RequestRecord* request,const boost::system::error_code& error,std::size_t bytesTransferred);
        void handleConnect(const boost::system::error_code& error,const boost::asio::ip::tcp::endpoint& ep);
        void handleResolve(const boost::system::error_code& error,const boost::asio::ip::tcp::resolver::results_type& results);

        int sendRequest(RequestRecord* request);
        
        int sendDescribeCommand(ResponseHandler handler);
        

    };

};


#endif//RTSP_CLIENT_H
