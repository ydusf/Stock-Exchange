#include "MarketManager.hxx"

MarketManager::MarketManager()
{
}

MarketQuote* MarketManager::GetMarketQuote(const std::string& ticker)
{
    auto itr = m_marketQuotes.find(ticker);
    if (itr != m_marketQuotes.end())
    {   
        return &itr->second;
    }

    return nullptr;
}

void MarketManager::AddMarketQuote(const std::string& ticker, double price)
{
    MarketQuote* marketQuote = GetMarketQuote(ticker);

    if (!marketQuote)
    {
        m_marketQuotes.try_emplace(ticker, price);
    }
}

void MarketManager::UpdateMarketQuote(const std::string& ticker, double topBid, double topAsk)
{
    MarketQuote* marketQuote = GetMarketQuote(ticker);

    double meanPrice = topBid * 0.5 + topAsk * 0.5;

    if (marketQuote)
    {
        marketQuote->m_lastPrice = (marketQuote->m_lastPrice * 0.5) + (meanPrice * 0.5);
    }
    else
    {
        AddMarketQuote(ticker, meanPrice);
    }
}