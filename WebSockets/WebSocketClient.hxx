#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <atomic>

namespace websocket 
{

namespace beast = boost::beast;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

class WebSocketClient 
{

private:
    tcp::resolver m_resolver;
    beast::websocket::stream<tcp::socket> m_socket;
    beast::flat_buffer m_buffer;
    std::string m_host;
    std::string m_port;
    std::string m_orderMessage;
    std::atomic<bool> m_running = true;
    std::thread m_userInputThread;

public:
    WebSocketClient(asio::io_context& ioc, const std::string& host, const std::string& port);

    ~WebSocketClient();

    void start();
    void SendOrderRequest(const nlohmann::json& message);

private:
    void StartInputHandler();
    void connect(const tcp::resolver::results_type& results);
    void doHandshake();
    void readMessages();
};

}