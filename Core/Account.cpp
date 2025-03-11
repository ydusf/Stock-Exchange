#include "Account.hxx"

#include <cassert>

Account::Account(std::size_t id, MarketManager& marketManager, double initialCashBalance)
    : m_id(id), m_marketManager(marketManager), m_portfolio(Portfolio(initialCashBalance, 0, 0, {}))
{

}

Account::~Account()
{

}

std::size_t Account::GetId() const
{
    return m_id;
}

double Account::GetNetworth()
{
    UpdateAssetBalance();
    return m_portfolio.m_cashBalance + m_portfolio.m_reservedBalance + m_portfolio.m_assetBalance;
}

Portfolio Account::GetPortfolio()
{
    UpdateAssetBalance();
    return m_portfolio;
}

void Account::Withdraw(double withdrawal)
{
    if (m_portfolio.m_cashBalance >= withdrawal)
    {
        m_portfolio.m_cashBalance -= withdrawal;
    }
}

void Account::Insert(double insertion)
{
    m_portfolio.m_cashBalance += insertion;
};

void Account::UpdateBalances(double cashBalanceDelta, double reservedCashDelta)
{
    m_portfolio.m_cashBalance += cashBalanceDelta;
    m_portfolio.m_reservedBalance += reservedCashDelta;
}

void Account::UpdateAssetBalance()
{
    auto& [cashBalance, reservedBalance, assetBalance, assets] = m_portfolio;

    double newAssetBalance = 0;

    for (auto& [ticker, quantity] : assets)
    {
        MarketQuote* marketQuote = m_marketManager.GetMarketQuote(ticker);
        newAssetBalance += (marketQuote->m_lastPrice * quantity);
    }

    m_portfolio.m_assetBalance = newAssetBalance;
}

void Account::UpdateAssets(const std::string& ticker, double quantityDelta)
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

    assets.insert({ ticker, quantityDelta }); // must be a buy order i.e. quantityDelta is +
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
    else
    {
        auto itr = assets.find(ticker);

        return itr != assets.end() && itr->second >= quantity;
    }
}

void Account::AddOrder(const Order& order)
{
    auto itr = m_orderIds.find(order.m_orderId);
    if (itr == m_orderIds.end())
    {
        m_orderIds.insert(order.m_orderId);
    }
}

void Account::RemoveOrder(const Order& order)
{
    auto itr = m_orderIds.find(order.m_orderId);
    if (itr != m_orderIds.end())
    {
        m_orderIds.erase(itr);
        return;
    }
}