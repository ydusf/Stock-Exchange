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

void MarketManager::AddMarketQuote(const std::string& ticker, double meanPrice, double topBid, double topAsk, std::size_t volume)
{
    MarketQuote* marketQuote = GetMarketQuote(ticker);

    if (!marketQuote)
    {
        m_marketQuotes.try_emplace(ticker, meanPrice, topBid, topAsk, volume);
    }
}

void MarketManager::UpdateMarketQuote(const std::string& ticker, double topBid, double topAsk, std::size_t volume)
{
    MarketQuote* marketQuote = GetMarketQuote(ticker);

    double meanPrice = topBid * 0.5 + topAsk * 0.5;

    if (marketQuote)
    {
        marketQuote->m_lastPrice = (marketQuote->m_lastPrice * 0.5) + (meanPrice * 0.5);
        marketQuote->m_topBid = topBid;
        marketQuote->m_topAsk = topAsk;
        marketQuote->m_volume = volume;
    }
    else
    {
        AddMarketQuote(ticker, meanPrice, topBid, topAsk, volume);
    }

    NotifyObservers(ticker);
}

void MarketManager::AddObserver(const std::string& ticker, StockCallbackType& callback)
{
    auto itr = m_marketObservers.find(ticker);
    if (itr != m_marketObservers.end())
    {
        itr->second.push_back(m_nextCallbackId);
    }
    else
    {
        m_marketObservers.insert({ ticker, { m_nextCallbackId } });
    }

    m_callbacks.insert({ m_nextCallbackId, callback });

    m_nextCallbackId++;
}

void MarketManager::NotifyObservers(const std::string& ticker)
{
    auto observerItr = m_marketObservers.find(ticker);
    if (observerItr == m_marketObservers.end())
        return;

    for (std::size_t id : observerItr->second)
    {
        auto callbackItr = m_callbacks.find(id);
        if (callbackItr == m_callbacks.end())
            return;

        (callbackItr->second)(m_marketQuotes.at(ticker));
    }
}