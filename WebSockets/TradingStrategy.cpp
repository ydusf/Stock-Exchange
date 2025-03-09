#include "TradingStrategy.hxx"

#include <iostream>

TradingStrategyHHLL::TradingStrategyHHLL(Account& account, std::string ticker)
	: m_account(account), m_ticker(std::move(ticker))
{
};

void TradingStrategyHHLL::RunImpl(MarketQuote quote)
{
	std::cout << "Running strategy for: " << m_ticker << '\n';
	Execute(quote);
}

void TradingStrategyHHLL::Execute(MarketQuote quote)
{
    nlohmann::json order = {
        { "id", "0" },
        { "ticker", "NVDA" },
        { "type", "limit" },
        { "side", "buy" },
        { "quantity", "10" },
        { "price", "100" }
    };

	m_orderConnection->SendOrderRequest(order);
	std::cout << "Executing trades for: " << m_ticker << '\n';
}