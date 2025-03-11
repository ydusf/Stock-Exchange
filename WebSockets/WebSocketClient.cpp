#include "WebSocketClient.hxx"

using namespace websocket;

WebSocketClient::WebSocketClient(asio::io_context& ioc, const std::string& host, const std::string& port)
    : m_resolver(ioc), m_socket(ioc), m_host(host), m_port(port)
{}

WebSocketClient::~WebSocketClient() 
{
    m_running = false;
    if (m_userInputThread.joinable())
    {
        m_userInputThread.join();
    }
};

void WebSocketClient::StartInputHandler()
{
    m_userInputThread = std::thread([this]()
    {
        while (m_running.load())
        {
            std::cout << "Press Enter to send a message..." << '\n';
            std::cin.get();

            if (!m_running.load()) 
                break;

            nlohmann::json order = 
            {
                { "id", "0" },
                { "ticker", "NVDA" },
                { "type", "limit" },
                { "side", "buy" },
                { "quantity", "10" },
                { "price", "100" }
            };

            SendOrderRequest(order);
        }
    });
}

void WebSocketClient::start()
{
    m_resolver.async_resolve(m_host, m_port,
    [this](boost::system::error_code ec, tcp::resolver::results_type results)
    {
        if (ec)
        {
            std::cout << "Failed to start: " << ec.message() << '\n';
            return;
        }
            
        connect(results);
    });
}


void WebSocketClient::connect(const tcp::resolver::results_type& results)
{
    asio::async_connect(m_socket.next_layer(), results.begin(), results.end(),
    [this](boost::system::error_code ec, tcp::resolver::iterator)
    {
        if (ec)
        {
            std::cout << "Failed to connect: " << ec.message() << '\n';
            return;
        }
            
        doHandshake();
    });
}

void WebSocketClient::doHandshake()
{
    m_socket.async_handshake(m_host, "/",
    [this](beast::error_code ec)
    {
        if (ec)
        {
            std::cerr << "Handshake failed: " << ec.message() << '\n';
            return;
        }

        StartInputHandler();
        readMessages();
    });
}

void WebSocketClient::SendOrderRequest(const nlohmann::json& message)
{
    m_orderMessage = message.dump();
    m_socket.async_write(boost::asio::buffer(m_orderMessage),
    [this](boost::system::error_code ec, std::size_t)
    {
        if (ec)
        {
            std::cerr << "Write failed: " << ec.message() << '\n';
        }
    });
}

void WebSocketClient::readMessages()
{
    m_socket.async_read(m_buffer,
    [this](boost::system::error_code ec, std::size_t bytes_transferred)
    {
        if (!ec)
        {
            m_orderMessage = beast::buffers_to_string(m_buffer.data());
            std::cout << "Received message: " << m_orderMessage << '\n';
            m_buffer.consume(bytes_transferred);
            readMessages();
        }
    });
}