#pragma once

#include "Core/Exchange.hxx"
#include "WebSocketConnection.hxx"

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include <string>
#include <thread>
#include <iostream>

namespace websocket
{

namespace beast = boost::beast;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

class WebSocketServer
{
private:
	Exchange& m_exchange;
	tcp::acceptor m_acceptor;

public:
	WebSocketServer(asio::io_context& ioc, tcp::endpoint endpoint, Exchange& exchange);

	void start();
	void acceptConnections();
	//void broadcastMessage();
	//void close();

	~WebSocketServer();

private:

};

}