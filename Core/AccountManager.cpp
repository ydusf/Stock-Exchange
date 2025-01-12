#include "AccountManager.hxx"


AccountManager::AccountManager()
{

}

AccountManager::~AccountManager()
{

}

std::unordered_map<std::size_t, std::shared_ptr<Account>> AccountManager::GetAccounts() const
{
    return m_accounts;
}

std::optional<std::shared_ptr<Account>> AccountManager::GetAccount(std::size_t id) const
{
    std::optional<std::shared_ptr<Account>> account = std::nullopt;

    return (m_accounts.find(id) != m_accounts.end()) ? m_accounts.at(id) : account;
}

void AccountManager::AddAccount(std::size_t id, double cashBalance, double assetBalance)
{
    if (m_accounts.find(id) != m_accounts.end()) // user already added;
        return;

    m_accounts.insert({ id, std::make_shared<Account>(cashBalance, assetBalance) });
}

void AccountManager::UpdateBalances(std::size_t ownerId, double tradeValue)
{
    std::optional<std::shared_ptr<Account>> account = GetAccount(ownerId);

    if (account == std::nullopt)
        return;

    double cashBalance = (*account)->GetCashBalance();
    double assetBalance = (*account)->GetAssetBalance();

    (*account)->SetCashBalance(cashBalance - tradeValue);
    (*account)->SetAssetBalance(assetBalance + tradeValue);
}