#pragma once

#include <string>
#include <unordered_map>

struct MarketQuote
{
    double m_lastPrice;
    //double m_topBid;
    //double m_topAsk;
    //double m_volume;
};

class MarketManager
{
private:
    std::unordered_map<std::string, MarketQuote> m_marketQuotes;

public:
	MarketManager();

    MarketQuote* GetMarketQuote(const std::string& ticker);

    void AddMarketQuote(const std::string& ticker, double price);

    void UpdateMarketQuote(const std::string& ticker, double topBid, double topAsk);

	~MarketManager();

private:


};