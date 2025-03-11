#include "Exchange.hxx"

Exchange::Exchange()
    : m_accountManager(std::make_unique<AccountManager>()),
    m_marketManager(std::make_unique<MarketManager>())
{
}

Exchange::~Exchange()
{

}

MarketManager* Exchange::GetMarketManager()
{
    return m_marketManager.get();
}

AccountManager* Exchange::GetAccountManager()
{
    return m_accountManager.get();
}

OrderBook* Exchange::GetOrderBook(const std::string& ticker)
{
    if (m_orderBooks.find(ticker) != m_orderBooks.end())
    {
        return &m_orderBooks.at(ticker);
    }

    return nullptr;
}

bool Exchange::ProcessOrderRequest(std::size_t ownerId, const std::string& ticker, OrderType orderType, Side side, double quantity, double price)
{
    Account* account = m_accountManager->GetAccount(ownerId);
    if (!account)
        return false;

    bool validOrder = account->CanPlaceOrder(ticker, side, quantity, price);
    if (!validOrder)
        return false;

    MarketQuote* quote = m_marketManager->GetMarketQuote(ticker);
    if (quote && orderType == OrderType::LimitOrder)
    {
        bool withinRange = price >= quote->m_lastPrice * 0.92 && price <= quote->m_lastPrice * 1.08;
        if (!withinRange)
            return false;
    }

    OrderBook* orderBook = GetOrderBook(ticker);

    if (!orderBook)
    {
        auto [it, inserted] = m_orderBooks.try_emplace(ticker, ticker);
        if (inserted)
        {
            RegisterOrderBookCallbacks(it->second);
        }

        it->second.AddOrder(ownerId, orderType, side, quantity, price);
    }
    else
    {
        orderBook->AddOrder(ownerId, orderType, side, quantity, price);
    }

    return true;
}

bool Exchange::ProcessCancelRequest(std::size_t ownerId, std::size_t orderId)
{
    return true;
}

bool Exchange::ProcessModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice)
{
    return true;
}

bool Exchange::SendOrderRequest(std::size_t ownerId, const std::string& ticker, OrderType orderType, Side side, double quantity, double price)
{
    ProcessOrderRequest(ownerId, ticker, orderType, side, quantity, price);
    return true;
}

bool Exchange::SendCancelRequest(std::size_t ownerId, std::size_t orderId)
{
    return true;
}

bool Exchange::SendModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice)
{
    return true;
}

void Exchange::AddSeedData(std::size_t id, const std::string& ticker, double quantity)
{
    Account* account = m_accountManager->GetAccount(id);
    if (!account)
        return;

    account->UpdateAssets(ticker, quantity);
}

void Exchange::RegisterOrderBookCallbacks(OrderBook& orderBook)
{
    orderBook.RegisterOrderMatchCallback
    (
        [this](const Trade& trade)
        {
            return on_order_match(trade);
        }
    );

    orderBook.RegisterAddOrderCallback
    (
        [this](const Order& order)
        {
            return on_add_order(order);
        }
    );

    orderBook.RegisterUpdateMarketQuoteCallback
    (
        [this](const std::string& ticker, double topBid, double topAsk)
        {
            return on_update_market_quote(ticker, topBid, topAsk);
        }
    );
}

bool Exchange::on_order_match(const Trade& trade)
{
    std::lock_guard<std::mutex> lock(m_lock);
    const auto& [ticker, bid, ask, price, quantity] = trade;

    Account* buyer = m_accountManager->GetAccount(bid.m_ownerId);
    Account* seller = m_accountManager->GetAccount(ask.m_ownerId);

    if (!buyer || !seller)
        return false;

    if (bid.m_status == Status::Filled)
    {
        buyer->RemoveOrder(bid);
    }
    if (ask.m_status == Status::Filled)
    {
        seller->RemoveOrder(ask);
    }

    OrderBook* orderBook = GetOrderBook(ticker);

    m_accountManager->UpdateAccounts(trade, orderBook);

    return true;
}

bool Exchange::on_add_order(const Order& order)
{
    Account* owner = m_accountManager->GetAccount(order.m_ownerId);

    if (!owner)
        return false;

    owner->AddOrder(order);

    double tradeValue = order.m_price * order.m_quantity;

    if (order.m_side == Side::Buy)
    {
        owner->UpdateBalances(0, tradeValue);
    }

    return true;
}

void Exchange::on_update_market_quote(const std::string& ticker, double topBid, double topAsk)
{
    OrderBook* orderBook = GetOrderBook(ticker);
    std::size_t volume = orderBook->GetVolume();

    m_marketManager->UpdateMarketQuote(ticker, topBid, topAsk, volume);
}
