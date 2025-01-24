#include "AccountManager.hxx"


AccountManager::AccountManager()
{

}

AccountManager::~AccountManager()
{

}

std::size_t AccountManager::GetNextAvailableId() const
{
    return m_nextId;
}

std::unordered_map<std::size_t, Account> AccountManager::GetAccounts() const
{
    return m_accounts;
}

Account* AccountManager::GetAccount(std::size_t id)
{
    if (m_accounts.find(id) != m_accounts.end())
    {
        return &m_accounts.at(id);
    }
    
    return nullptr;
}

void AccountManager::AddAccount(std::size_t id, double cashBalance, double assetBalance)
{
    if (m_accounts.find(id) != m_accounts.end()) // user already added;
        return;

    m_accounts.insert({ id, Account(id, cashBalance, assetBalance) });
    m_nextId++;
}

void AccountManager::UpdateAccounts(const Trade& trade)
{
    const auto& [bid, ask, price, quantity] = trade;
    double tradeValue = price * quantity;

    Account* buyer = GetAccount(trade.m_buyer.m_ownerId);
    Account* seller = GetAccount(trade.m_seller.m_ownerId);

    if (!buyer || !seller)
        return;

    buyer->UpdatePortfolio(-tradeValue, tradeValue, -tradeValue);
    buyer->UpdateAssetQuantities(bid.m_ticker, quantity);

    seller->UpdatePortfolio(tradeValue, -tradeValue, 0);
    seller->UpdateAssetQuantities(ask.m_ticker, -quantity);
}
