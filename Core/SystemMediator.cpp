#include "SystemMediator.hxx"

SystemMediator::SystemMediator()
{

}

SystemMediator::~SystemMediator()
{

}

AccountManager* SystemMediator::GetAccountManager()
{
    return &m_accountManager;
}

OrderBook* SystemMediator::GetOrderBook(std::string& ticker)
{
    if (m_orderBooks.find(ticker) != m_orderBooks.end())
    {
        return &m_orderBooks.at(ticker);
    }

    return nullptr;
}

bool SystemMediator::SendOrderRequest(std::size_t ownerId, std::string& ticker, Side side, double quantity, double price)
{
    Account* account = m_accountManager.GetAccount(ownerId);

    if (!account || !(account->CanPlaceOrder(ticker, side, quantity, price)))
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

void SystemMediator::SendCancelRequest(std::size_t ownerId, std::size_t orderId)
{

}

void SystemMediator::SendModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice)
{

}

MarketQuote SystemMediator::QueryMarketData(std::string& ticker)
{
    OrderBook* orderBook = GetOrderBook(ticker);

    if (!orderBook)
        return {};

    return orderBook->GetMarketQuote();
}

void SystemMediator::RegisterOrderBookCallbacks(OrderBook& orderBook)
{
    orderBook.RegisterOrderMatchCallback(
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

bool SystemMediator::on_order_match(const Trade& trade)
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

    m_accountManager.UpdateBalances(trade);

    return true;
}

bool SystemMediator::on_add_order(const Order& order)
{
    Account* owner = m_accountManager.GetAccount(order.m_ownerId);

    if (!owner)
        return false;

    owner->AddOrder(order);

    double tradeValue = order.m_price * order.m_quantity;

    if (order.m_side == Side::Buy)
    {
        owner->UpdateReservedCash(tradeValue);
    }

    return true;
}