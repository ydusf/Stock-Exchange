#pragma once

#include "Account.hxx"

#include <unordered_map>
#include <memory>
#include <cstddef>
#include <optional>

class AccountManager final
{
private:
    std::unordered_map<std::size_t, Account> m_accounts;

public:
    AccountManager();

    std::unordered_map<std::size_t, Account> GetAccounts() const;

    Account* GetAccount(std::size_t ownerId);
    void AddAccount(std::size_t ownerId, double cashBalance, double assetBalance);
    void UpdateBalances(const Trade& trade);

    ~AccountManager();

private:


};