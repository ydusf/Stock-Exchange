#include "Exchange.hxx"

Exchange::Exchange()
{

}

Exchange::~Exchange()
{

}

AccountManager* Exchange::GetAccountManager()
{
    return &m_accountManager;
}

OrderBook* Exchange::GetOrderBook(std::string& ticker)
{
    if (m_orderBooks.find(ticker) != m_orderBooks.end())
    {
        return &m_orderBooks.at(ticker);
    }

    return nullptr;
}

bool Exchange::SendOrderRequest(std::size_t ownerId, std::string& ticker, Side side, double quantity, double price)
{
    Account* account = m_accountManager.GetAccount(ownerId);

    if (!account)
        return false;

    bool validOrder = account->CanPlaceOrder(ticker, side, quantity, price);

    if (!validOrder)
        return false;

    OrderBook* orderBook = GetOrderBook(ticker);

    if (!orderBook)
    {
        auto [it, inserted] = m_orderBooks.try_emplace(ticker);
        if (inserted) 
        {
            RegisterOrderBookCallbacks(it->second);
        }

        it->second.AddOrder(ownerId, ticker, side, quantity, price);
    }
    else
    {
        orderBook->AddOrder(ownerId, ticker, side, quantity, price);
    }

    return true;
}

void Exchange::SendCancelRequest(std::size_t ownerId, std::size_t orderId)
{

}

void Exchange::SendModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice)
{

}

MarketQuote Exchange::QueryMarketData(std::string& ticker)
{
    OrderBook* orderBook = GetOrderBook(ticker);

    if (!orderBook)
        return {};

    return orderBook->GetMarketQuote();
}

void Exchange::AddSeedData(std::size_t id, std::string& ticker, double quantity)
{
    Account* account = m_accountManager.GetAccount(id);
    account->UpdateAssetQuantities(ticker, quantity);
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
}

bool Exchange::on_order_match(const Trade& trade)
{
    const auto& [bid, ask, price, quantity] = trade;

    Account* buyer = m_accountManager.GetAccount(bid.m_ownerId);
    Account* seller = m_accountManager.GetAccount(ask.m_ownerId);

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

    m_accountManager.UpdateAccounts(trade);

    return true;
}

bool Exchange::on_add_order(const Order& order)
{
    Account* owner = m_accountManager.GetAccount(order.m_ownerId);

    if (!owner)
        return false;

    owner->AddOrder(order);

    double tradeValue = order.m_price * order.m_quantity;

    if (order.m_side == Side::Buy)
    {
        owner->UpdatePortfolio(0, 0, tradeValue);
    }

    return true;
}