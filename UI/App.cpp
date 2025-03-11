#include "App.hxx"
#include "MainFrame.hxx"

#include "WebSockets/WebSocketServer.hxx"
#include "WebSockets/WebSocketClient.hxx"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include <iostream>
#include <string>
#include <vector>

using namespace websocket;

static void StartExchangeServer(Exchange& exchange)
{
    unsigned short port = 8080;
    asio::io_context ioc;
    tcp::endpoint endpoint(tcp::v4(), port);
    WebSocketServer server(ioc, endpoint, exchange);

    std::cout << "WebSocket Server running on ws://localhost:" << port << '\n';
    server.start();
    ioc.run();
}

bool App::OnInit()
{
    m_exchangeServer = std::thread([&]() { StartExchangeServer(m_exchange); });

    MainFrame* mainFrame = new MainFrame(m_exchange);
    mainFrame->Show(true);
    return true;
}

App::~App()
{
    if (m_exchangeServer.joinable())
    {
        m_exchangeServer.join();
    }
}

wxIMPLEMENT_APP(App);
