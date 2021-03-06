//
// Created by dima on 24/11/17.
//

#ifndef TCP_SHMAFKA_TCPCONNECTION_H
#define TCP_SHMAFKA_TCPCONNECTION_H

#include <boost/asio.hpp>
#include <string>
#include <boost/bind.hpp>
#include <cstdint>

#include "../DefinedMessages.h"
#include "TcpServerOutcomeMessage.h"
#include "../Msg/MsgBuilder.h"
#include "../Queue/AsyncLimitedQueue.h"


using namespace boost::asio::ip;
using namespace std;


class ServerHandler;
class TcpServerConnection :
        public std::enable_shared_from_this<TcpServerConnection>{

public:
    static std::shared_ptr<TcpServerConnection> create(boost::asio::io_service& io_service, ServerHandler *serverHandler)
    {
        return std::make_shared<TcpServerConnection>(TcpServerConnection(io_service,serverHandler));
    }
    TcpServerConnection() = default;
    TcpServerConnection(boost::asio::io_service& io_service, ServerHandler *serverHandler);
    tcp::socket * socket();

    bool sendBulk(SocketProtoBuffer *buffer);
    void set_no_delay();
    void register_and_send_server_welcome();
    void send_server_goodbye();
    void close();

    bool errorOccured = false;
    bool writeInProgress = false;
    bool duringReading = false;

    void tryRead();
    void tryWrite();

private:
    void handle_send_welcome_message(const boost::system::error_code &errorCode, size_t);
    void handle_send_server_goodbye(const boost::system::error_code &errorCode, size_t size);


    void handle_read_len(const boost::system::error_code &errorCode, size_t size);
    void handle_read_data(const boost::system::error_code &errorCode, size_t size);
    void handle_send_data(SocketProtoBuffer *buffer, const boost::system::error_code &errorCode, size_t size);


    static uint32_t client_id;
    ServerHandler * serverHandler;
    tcp::socket * tcpServerSocket = nullptr;
    uint32_t len_in{};
    SocketProtoBuffer * in{};
    MsgBuilder *builder{};
    AsyncLimitedQueue<SocketProtoBuffer *> sendQueue;


    uint32_t id{};
    bool sendingData;
    bool appConnected;

    void doSendBuffer(SocketProtoBuffer *buffer);
};


#endif //TCP_SHMAFKA_TCPCONNECTION_H
