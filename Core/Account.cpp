#include "Account.hxx"

#include <cassert>

Account::Account(double initialCashBalance, double initialAssetBalance)
    : m_cashBalance(initialCashBalance), m_assetBalance(initialAssetBalance), m_reservedBalance(0)
{

}

Account::~Account()
{

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

void Account::SetCashBalance(double newCashBalance)
{
    m_cashBalance = newCashBalance;
}

double Account::GetAssetBalance() const
{
    return m_assetBalance;
}

void Account::SetAssetBalance(double newAssetBalance)
{
    m_assetBalance = newAssetBalance;
}

void Account::UpdateReservedCash(double reservedCashDelta)
{
    m_reservedBalance -= reservedCashDelta;
}

bool Account::CanPlaceOrder(Side side, double amount) const
{
    if (side == Side::Buy)
    {
        return (m_cashBalance - m_reservedBalance) >= amount;
    }
    else
    {
        return m_assetBalance >= amount;
    }
}

void Account::AddOrder(std::size_t orderId)
{
    if (m_orderIds.find(orderId) == m_orderIds.end())
    {
        m_orderIds.insert(orderId);
        return;
    }

    std::cerr << "Order with id: " << orderId << " already exists" << '\n';
}

void Account::RemoveOrder(std::size_t orderId)
{
    if (m_orderIds.find(orderId) != m_orderIds.end())
    {
        m_orderIds.erase(orderId);
        return;
    }

    std::cerr << "Order with id: " << orderId << " does not exist" << '\n';
}