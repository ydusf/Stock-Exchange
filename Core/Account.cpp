#include "Account.hxx"

#include <cassert>

Account::Account(std::size_t id, double initialCashBalance, double initialAssetBalance)
    : m_id(id), m_portfolio(Portfolio(initialCashBalance, initialAssetBalance, 0, {}))
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
    return m_portfolio.m_cashBalance + m_portfolio.m_reservedBalance + m_portfolio.m_assetBalance;
}

Portfolio Account::GetPortfolio() const
{
    return m_portfolio;
}

void Account::UpdatePortfolio(double cashBalanceDelta, double assetBalanceDelta, double reservedCashDelta)
{
    m_portfolio.m_cashBalance += cashBalanceDelta;
    m_portfolio.m_assetBalance += assetBalanceDelta;
    m_portfolio.m_reservedBalance += reservedCashDelta;
}

void Account::UpdateAssetQuantities(const std::string& ticker, double quantityDelta)
{
    std::unordered_map<std::string, double>& assets = m_portfolio.m_assets;
    
    auto itr = assets.find(ticker);
    if (itr != assets.end())
    {
        itr->second += quantityDelta;
        if (itr->second <= 0)
        {
            assets.erase(itr);
        }

        return;
    }

    assets.insert({ticker, quantityDelta}); // must be a buy order i.e. quantityDelta is +
    //assert(quantityDelta >= 0);
}

bool Account::CanPlaceOrder(const std::string& ticker, Side side, double quantity, double price) const
{
    double orderValue = quantity * price;

    const auto& [cashBalance, assetBalance, reservedBalance, assets] = m_portfolio;
    
    if (side == Side::Buy)
    {
        return (cashBalance - reservedBalance) >= orderValue;
    }

    auto itr = assets.find(ticker);
    if (itr != assets.end())
    {
        return assetBalance >= orderValue && assets.at(ticker) >= quantity;
    }

    return false;
}

void Account::AddOrder(const Order& order)
{
    if (m_orderIds.find(order.m_orderId) == m_orderIds.end())
    {        
        m_orderIds.insert(order.m_orderId);
    }
}

void Account::RemoveOrder(const Order& order)
{
    if (m_orderIds.find(order.m_orderId) != m_orderIds.end())
    {
        m_orderIds.erase(order.m_orderId);
        return;
    }
}