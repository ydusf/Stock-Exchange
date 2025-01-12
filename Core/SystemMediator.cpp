#include "SystemMediator.hxx"

SystemMediator::SystemMediator(std::unique_ptr<AccountManager> accountManager)
    : m_accountManager(std::move(accountManager))
{

}

SystemMediator::~SystemMediator()
{

}

AccountManager* SystemMediator::GetAccountManager() const
{
    return m_accountManager.get();
}

OrderBook* SystemMediator::GetOrderBook(std::string ticker) const
{
    if (m_orderBooks.find(ticker) != m_orderBooks.end())
    {
        return m_orderBooks.at(ticker).get();
    }

    return nullptr;
}

bool SystemMediator::SendOrderRequest(std::size_t ownerId, std::string ticker, Side side, double quantity, double price)
{
    std::optional<std::shared_ptr<Account>> account = m_accountManager->GetAccount(ownerId);

    if (!account)
        return false;

    double tradeValue = quantity * price;

    if (side == Side::BUY)
    {
        if (tradeValue > (*account)->GetCashBalance())
            return false;

        (*account)->SetCashBalance((*account)->GetCashBalance() - tradeValue);
        (*account)->SetAssetBalance((*account)->GetAssetBalance() + tradeValue);
    }
    else
    {
        if (tradeValue > (*account)->GetAssetBalance())
            return false;

        (*account)->SetCashBalance((*account)->GetCashBalance() + tradeValue);
        (*account)->SetAssetBalance((*account)->GetAssetBalance() - tradeValue);
    }

    OrderBook* orderBook = GetOrderBook(ticker);

    if (!orderBook) // create orderBook
    {
        std::unique_ptr<OrderBook> newOrderBook = std::make_unique<OrderBook>();
        RegisterOrderBookCallbacks(newOrderBook);

        newOrderBook->AddOrder(ownerId, ticker, side, quantity, price);

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

void SystemMediator::RegisterOrderBookCallbacks(std::unique_ptr<OrderBook>& orderBook)
{
    orderBook->RegisterOrderMatchCallback([this](const std::shared_ptr<Order>& bid, const std::shared_ptr<Order>& ask, double tradeValue)
        {
            on_order_match(bid, ask, tradeValue);
        });

    orderBook->RegisterAddOrderCallback([this](const std::shared_ptr<Order>& order)
        {
            on_add_order(order);
        });
}

void SystemMediator::on_order_match(const std::shared_ptr<Order>& bid, const std::shared_ptr<Order>& ask, double tradeValue)
{
    std::optional<std::shared_ptr<Account>> buyer = m_accountManager->GetAccount(bid->m_ownerId);
    std::optional<std::shared_ptr<Account>> seller = m_accountManager->GetAccount(ask->m_ownerId);

    if (!buyer || !seller)
        return;

    std::cout << "Trade occured between: " << bid->m_ownerId << " and " << ask->m_ownerId << " with trade value of: " << tradeValue << '\n';

    if (bid->m_status == Status::Filled)
    {
        (*buyer)->RemoveOrder(bid->m_orderId);
    }
    if (ask->m_status == Status::Filled)
    {
        (*seller)->RemoveOrder(ask->m_orderId);
    }

    m_accountManager->UpdateBalances(bid->m_ownerId, tradeValue);
    m_accountManager->UpdateBalances(ask->m_ownerId, -tradeValue);
}

void SystemMediator::on_add_order(const std::shared_ptr<Order>& order)
{
    std::optional<std::shared_ptr<Account>> owner = m_accountManager->GetAccount(order->m_ownerId);

    if (!owner)
        return;

    (*owner)->AddOrder(order->m_orderId);

    double tradeValue = order->m_price * order->m_quantity;

    m_accountManager->UpdateBalances(order->m_ownerId, order->m_side == Side::BUY ? tradeValue : -tradeValue);
}