#include "Account.hxx"

#include <cassert>

Account::Account(double initialCashBalance, double initialAssetBalance)
    : m_cashBalance(initialCashBalance), m_assetBalance(initialAssetBalance)
{

}

Account::~Account()
{

}

double Account::GetCashBalance() const
{
    return m_cashBalance;
}

void Account::SetCashBalance(double newCashBalance)
{
    assert(newCashBalance >= 0);
    m_cashBalance = newCashBalance;
}

double Account::GetAssetBalance() const
{
    return m_assetBalance;
}

void Account::SetAssetBalance(double newAssetBalance)
{
    assert(newAssetBalance >= 0);
    m_assetBalance = newAssetBalance;
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