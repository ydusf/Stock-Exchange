#include "WebSocketServer.hxx"

using namespace websocket;

WebSocketServer::WebSocketServer(asio::io_context& ioc, tcp::endpoint endpoint, Exchange& exchange)
    : m_acceptor(ioc, endpoint), m_exchange(exchange)
{}

WebSocketServer::~WebSocketServer()
{};

void WebSocketServer::start()
{
    acceptConnections();
}
void WebSocketServer::acceptConnections()
{
    m_acceptor.async_accept(
    asio::make_strand(m_acceptor.get_executor()),
    [this](boost::system::error_code ec, tcp::socket socket) 
    {
        if (!ec) 
        {
            std::make_shared<WebSocketConnection>(std::move(socket), m_exchange)->start();
        }

        acceptConnections();
    });
}
