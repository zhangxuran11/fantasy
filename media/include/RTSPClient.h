#ifndef RTSP_CLIENT_H
#define RTSP_CLIENT_H
#include <boost/asio.hpp>
#include <cstring>
namespace fantasy{
    class RTSPClient{
        public:
        
        class ResponseRecord{

        };
        typedef void (ResponseHandler)(RTSPClient* rtspClient,
				 int resultCode, ResponseRecord* response);
        class RequestRecord{
            public:
                RequestRecord(uint32_t cseq,const char* method,ResponseHandler handler);
        };
        RTSPClient(boost::asio::io_context& io_context,const char*progName,const char* rtsp_url);
        int play();
        private:
        std::string mProgName;
        boost::asio::io_context& mIoContext;
        boost::asio::ip::tcp::resolver* mResolver;
        boost::asio::ip::tcp::socket* mSock;
        std::string mUrl;
        int mCseq;
        char mWriteBuff[1024];
        char mReadBuff[1024];



        void handleRead(const boost::system::error_code& error,std::size_t bytesTransferred);
        void handleWrite(const boost::system::error_code& error,std::size_t bytesTransferred);
        void handleConnect(const boost::system::error_code& error,const boost::asio::ip::tcp::endpoint& ep);
        void handleResolve(const boost::system::error_code& error,const boost::asio::ip::tcp::resolver::results_type& results);

        int sendDescribeCommand();

    };

};


#endif//RTSP_CLIENT_H
