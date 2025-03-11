#pragma once

#include "Core/Exchange.hxx"

#include <nlohmann/json.hpp>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace websocket 
{

namespace beast = boost::beast;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection>
{
private:
	Exchange& m_exchange;

	beast::websocket::stream<tcp::socket> m_socket;
	beast::flat_buffer m_buffer;
	std::string m_messageBuffer;

public:
	WebSocketConnection(tcp::socket, Exchange& m_exchange);

	void start();
	
	~WebSocketConnection();

private:
	void acceptWebSocket();
	void onAccept(beast::error_code ec);
	void readMessages();
};

}