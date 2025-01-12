#pragma once

#include "Account.hxx"

#include <unordered_map>
#include <memory>
#include <cstddef>
#include <optional>

class AccountManager
{
private:
    std::unordered_map<std::size_t, std::shared_ptr<Account>> m_accounts;

public:
    AccountManager();

    std::unordered_map<std::size_t, std::shared_ptr<Account>> GetAccounts() const;

    std::optional<std::shared_ptr<Account>> GetAccount(std::size_t ownerId) const;
    void AddAccount(std::size_t ownerId, double cashBalance, double assetBalance);
    void UpdateBalances(std::size_t, double tradeValue);

    ~AccountManager();

private:


};