#include "AccountManager.hxx"


AccountManager::AccountManager()
{

}

AccountManager::~AccountManager()
{

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

    m_accounts.insert({ id, Account(cashBalance, assetBalance) });
}

void AccountManager::UpdateBalances(std::size_t ownerId, double tradeValue)
{
    std::optional<Account*> account = GetAccount(ownerId);

    if (!account)
        return;

    Account* accountPtr = (*account);

    double cashBalance = accountPtr->GetCashBalance();
    double assetBalance = accountPtr->GetAssetBalance();

    accountPtr->SetCashBalance(cashBalance - tradeValue);
    accountPtr->SetAssetBalance(assetBalance + tradeValue);
}