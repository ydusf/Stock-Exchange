#include "Account.hxx"

#include <cassert>

Account::Account(std::size_t id, double initialCashBalance, double initialAssetBalance)
    : m_id(id), m_cashBalance(initialCashBalance), m_assetBalance(initialAssetBalance), m_reservedBalance(0)
{

}

Account::~Account()
{

}

std::size_t Account::GetId() const
{
    return m_id;
}

double Account::GetNetworth() const
{
    return m_cashBalance + m_reservedBalance + m_assetBalance;
}

double Account::GetReservedBalance() const
{
    return m_reservedBalance;
}

double Account::GetCashBalance() const
{
    return m_cashBalance;
}

void Account::UpdateCashBalance(double cashBalanceDelta)
{
    m_cashBalance += cashBalanceDelta;
}

double Account::GetAssetBalance() const
{
    return m_assetBalance;
}

void Account::UpdateAssetBalance(double assetBalanceDelta)
{
    m_assetBalance += assetBalanceDelta;
}

void Account::UpdateReservedCash(double reservedCashDelta)
{
    m_reservedBalance += reservedCashDelta;
}

void Account::UpdateAssetQuantities(const std::string& ticker, double quantityDelta)
{
    auto itr = m_assets.find(ticker);
    if (itr != m_assets.end())
    {
        itr->second += quantityDelta;
        if (itr->second <= 0)
        {
            m_assets.erase(itr);
        }

        return;
    }

    m_assets.insert({ticker, quantityDelta}); // must be a buy order i.e. quantityDelta is +
    assert(quantityDelta >= 0);
}

bool Account::CanPlaceOrder(const std::string& ticker, Side side, double quantity, double price) const
{
    double orderValue = quantity * price;
    
    if (side == Side::Buy)
    {
        return (m_cashBalance - m_reservedBalance) >= orderValue;
    }

    auto itr = m_assets.find(ticker);
    if(itr != m_assets.end())
    {
        return m_assetBalance >= orderValue && m_assets.at(ticker) >= quantity;
    }

    return false;
}

void Account::AddOrder(const Order& order)
{
    if (m_orderIds.find(order.m_orderId) == m_orderIds.end())
    {        
        m_orderIds.insert(order.m_orderId);
    }

    m_assets.insert_or_assign(order.m_ticker, order.m_quantity);
}

void Account::RemoveOrder(const Order& order)
{
    if (m_orderIds.find(order.m_orderId) != m_orderIds.end())
    {
        m_orderIds.erase(order.m_orderId);
        return;
    }
}