//
// Created by dima on 24/11/17.
//

#ifndef TCP_SHMAFKA_CLIENT_H
#define TCP_SHMAFKA_CLIENT_H

//
// Client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <deque>
#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "../Buff/BufferPool.h"
#include "../Queue/ConcurrentQueue.h"

using namespace std;
using boost::asio::ip::tcp;

class ClientBufferConteiner{
public:
    ClientBufferConteiner(SocketProtoBuffer *first) : first(first), current(first) {
        SocketProtoBuffer *next = first;
        while(next != nullptr){
            last = next;
            next = next->nextBuffer;
        }
    }

    SocketProtoBuffer* first;
    SocketProtoBuffer* last;
    SocketProtoBuffer* current;
};
class Client
{
public:
    Client(boost::asio::io_service& io_service, string &host, uint16_t port);
    std::thread spawn();
    void setShouldRun(volatile bool shouldRun);

    void run(){
        while (shouldRun){
            io_service_.poll();
            shared_ptr<ClientBufferConteiner> buff = concurentQueueToServer.try_pop();
            if(buff != nullptr){
                write(buff);
            }
        }
        close();
    }

    void send(SocketProtoBuffer* buffer){
        concurentQueueToServer.push(make_shared<ClientBufferConteiner>(ClientBufferConteiner(buffer)));
    }

    SocketProtoBuffer * recieve(){
        return concurentQueueFromServer.pop();
    }

private:

    void handle_connect(const boost::system::error_code &error);

    void handle_read_hello(const boost::system::error_code &error);

    void handle_read_len(const boost::system::error_code &error);

    void handle_body(const boost::system::error_code &error);

    void write(shared_ptr<ClientBufferConteiner> buffer);

    void handle_write(shared_ptr<ClientBufferConteiner> out_buff, const boost::system::error_code &error);

    void set_no_deley();

    void do_close() {
        socket_.close();
    }

    void close() {
        io_service_.post(boost::bind(&Client::do_close, this));
    }





private:
    ConcurrentQueue<shared_ptr<ClientBufferConteiner>> concurentQueueToServer;
    ConcurrentQueue<SocketProtoBuffer *> concurentQueueFromServer;

    string host;
    uint16_t port;

    SocketProtoBuffer * in_buff;

    volatile bool shouldRun = true;

    tcp::resolver::iterator epIterator;
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
};

//int main(int argc, char* argv[])
//{
//    try
//    {
//        if (argc != 3)
//        {
//            std::cerr << "Usage: Client <host> <port>\n";
//            return 1;
//        }
//
//        boost::asio::io_service io_service;
//
//        tcp::resolver resolver(io_service);
//        tcp::resolver::query query(argv[1], argv[2]);
//        tcp::resolver::iterator iterator = resolver.resolve(query);
//
//        Client c(io_service, iterator);
//
//        boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
//
//        char line[chat_message::max_body_length + 1];
//        while (std::cin.getline(line, chat_message::max_body_length + 1))
//        {
//            using namespace std; // For strlen and memcpy.
//            chat_message msg;
//            msg.body_length(strlen(line));
//            memcpy(msg.body(), line, msg.body_length());
//            msg.encode_header();
//            c.write(msg);
//        }
//
//        c.close();
//        t.join();
//    }
//    catch (std::exception& e)
//    {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }
//
//    return 0;
//}

#endif //TCP_SHMAFKA_CLIENT_H
