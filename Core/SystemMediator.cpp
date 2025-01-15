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

OrderBook* SystemMediator::GetOrderBook(std::string ticker)
{
    if (m_orderBooks.find(ticker) != m_orderBooks.end())
    {
        return &m_orderBooks.at(ticker);
    }

    return nullptr;
}

bool SystemMediator::SendOrderRequest(std::size_t ownerId, std::string ticker, Side side, double quantity, double price)
{
    Account* account = m_accountManager.GetAccount(ownerId);

    if (!account || !(account->CanPlaceOrder(side, quantity * price)))
        return false;

    OrderBook* orderBook = GetOrderBook(ticker);

    if (!orderBook) // create orderBook
    {
        OrderBook newOrderBook;
        RegisterOrderBookCallbacks(newOrderBook);

        newOrderBook.AddOrder(ownerId, ticker, side, quantity, price);

        m_orderBooks.insert({ ticker, std::move(newOrderBook) });
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

void SystemMediator::RegisterOrderBookCallbacks(OrderBook& orderBook)
{
    orderBook.RegisterOrderMatchCallback([this](const Order& bid, const Order& ask, double tradeValue)
        {
            on_order_match(bid, ask, tradeValue);
        });

    orderBook.RegisterAddOrderCallback([this](const Order& order)
        {
            on_add_order(order);
        });
}

void SystemMediator::on_order_match(const Order& bid, const Order& ask, double tradeValue)
{
    Account* buyer = m_accountManager.GetAccount(bid.m_ownerId);
    Account* seller = m_accountManager.GetAccount(ask.m_ownerId);

    if (!buyer || !seller)
        return;

    if (bid.m_status == Status::Filled)
    {
        buyer->RemoveOrder(bid.m_orderId);
    }
    if (ask.m_status == Status::Filled)
    {
        seller->RemoveOrder(ask.m_orderId);
    }

    m_accountManager.UpdateBalances(bid.m_ownerId, tradeValue);
    buyer->UpdateReservedCash(tradeValue); // only update for buy orders . sell orders never reserved anything

    m_accountManager.UpdateBalances(ask.m_ownerId, -tradeValue);
}

void SystemMediator::on_add_order(const Order& order)
{
    Account* owner = m_accountManager.GetAccount(order.m_ownerId);

    if (!owner)
        return;

    owner->AddOrder(order.m_orderId);

    double tradeValue = order.m_price * order.m_quantity;

    if (order.m_side == Side::Buy)
    {
        owner->UpdateReservedCash(-tradeValue);
    }
}