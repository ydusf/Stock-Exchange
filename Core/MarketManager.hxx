#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <functional>

struct MarketQuote
{
    double m_lastPrice;
    double m_topBid;
    double m_topAsk;
    std::size_t m_volume;
};

class MarketManager
{
private:
    using StockCallbackType = std::function<void(MarketQuote)>;

    std::size_t m_nextCallbackId = 0;
    std::map<std::size_t, StockCallbackType> m_callbacks;

private:
    std::unordered_map<std::string, MarketQuote> m_marketQuotes;
    std::map<std::string, std::vector<std::size_t>> m_marketObservers;
    std::map<std::string, std::vector<std::size_t>> m_marketStrategies;

public:
    MarketManager();

    MarketQuote* GetMarketQuote(const std::string& ticker);
    void AddMarketQuote(const std::string& ticker, double meanPrice, double topBid, double topAsk, std::size_t volume);
    void UpdateMarketQuote(const std::string& ticker, double topBid, double topAsk, std::size_t volume);

    void AddObserver(const std::string& ticker, StockCallbackType& callback);
    void NotifyObservers(const std::string& ticker);

    ~MarketManager() = default;
};