#include "WebSocketConnection.hxx"

#include <iostream>

using namespace websocket;

WebSocketConnection::WebSocketConnection(tcp::socket socket, Exchange& exchange)
	: m_socket(std::move(socket)), m_exchange(exchange)
{}

WebSocketConnection::~WebSocketConnection()
{}

void WebSocketConnection::start()
{
    acceptWebSocket();
}

void WebSocketConnection::acceptWebSocket() 
{
    m_socket.async_accept(
        beast::bind_front_handler(
            &WebSocketConnection::onAccept,
            shared_from_this()
        )
    );
}

void WebSocketConnection::onAccept(beast::error_code ec)
{
    if (ec)
    {
        std::cerr << "Accept failed: " << ec.message() << '\n';
        return;
    }

    readMessages();
}

void WebSocketConnection::readMessages() 
{
    m_socket.async_read(m_buffer,
    [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred)
    {
        if (ec)
        {
            std::cerr << "Read failed: " << ec.message() << '\n';
            return;
        }

        self->m_messageBuffer = beast::buffers_to_string(self->m_buffer.data());
        std::cout << "Received: " << self->m_messageBuffer << '\n';

        // ----------------------- PROCESS MESSAGE -------------------------------

        auto json = nlohmann::json::parse(self->m_messageBuffer);

        std::size_t id = std::stoi(json["id"].get<std::string>());
        std::string ticker = json["ticker"].get<std::string>();
        std::string typeStr = json["type"].get<std::string>();
        OrderType type = typeStr == "limit" ? OrderType::LimitOrder : OrderType::MarketOrder;
        std::string sideStr = json["side"].get<std::string>();
        Side side = sideStr == "buy" ? Side::Buy : Side::Sell;
        double quantity = std::stod(json["quantity"].get<std::string>());
        double price = std::stod(json["price"].get<std::string>());

        self->m_exchange.SendOrderRequest(id, ticker, type, side, quantity, price);

        // Echo the message back
        self->m_socket.async_write(
            boost::asio::buffer(self->m_messageBuffer),
            [self](beast::error_code ec, std::size_t)
            {
                if (ec)
                {
                    std::cerr << "Write failed: " << ec.message() << '\n';
                }
            }
        );

        self->m_buffer.consume(bytes_transferred);
        self->readMessages();
    });
}

