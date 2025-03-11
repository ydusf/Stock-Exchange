#pragma once

#include "Core/MarketManager.hxx"
#include "Core/Account.hxx"
#include "WebSocketClient.hxx"

#include <nlohmann/json.hpp>

#include <string>

using namespace websocket;

template<typename TStrategy>
class TradingStrategy
{
protected:
	std::unique_ptr<WebSocketClient> m_orderConnection;

public:
	TradingStrategy()
	{
		//asio::io_context ioc;
		//std::string host = "localhost";
		//unsigned short port = 8080;
		//m_orderConnection = std::make_unique<WebSocketClient>(ioc, host, std::to_string(port));
		//m_orderConnection->start();

		//ioc.run();

	};

	void Run(MarketQuote quote)
	{
		static_cast<TStrategy*>(this)->RunImpl(quote);
	}
};


class TradingStrategyHHLL : public TradingStrategy<TradingStrategyHHLL>
{
private:
	Account& m_account;
	std::string m_ticker;

public:
	TradingStrategyHHLL(Account& account, std::string ticker);

	void RunImpl(MarketQuote quote);

	~TradingStrategyHHLL() = default;

private:
	void Execute(MarketQuote quote);

};