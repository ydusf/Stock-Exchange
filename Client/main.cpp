#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "WebSockets/WebSocketServer.hxx"
#include "WebSockets/WebSocketClient.hxx"

using namespace websocket;

int main(int argc, char* argv[])
{
    try
    {
        std::string host = argv[1];
        unsigned short port = std::stoi(argv[2]);

        // Start the client
        asio::io_context ioc;
        WebSocketClient client(ioc, host, std::to_string(port));

        std::cout << "Connecting to WebSocket server at ws://" << host << ":" << port << '\n';
        client.start();
        ioc.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
