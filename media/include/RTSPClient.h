#ifndef RTSP_CLIENT_H
#define RTSP_CLIENT_H
#include <boost/asio.hpp>
namespace fantasy{
    class RTSPClient{
        public:
        RTSPClient(boost::asio::io_context& io_context);
        int open(const char* url);
        private:

        boost::asio::io_context& m_io_context;
        boost::asio::ip::tcp::resolver* m_resolver;
        boost::asio::ip::tcp::socket* m_sock;

        void handle_connect(const boost::system::error_code& error,const boost::asio::ip::tcp::endpoint& ep);
        void handle_resolve(const boost::system::error_code& error,const boost::asio::ip::tcp::resolver::results_type& results);

        int sendDescribeCommand();

    };

};


#endif//RTSP_CLIENT_H
