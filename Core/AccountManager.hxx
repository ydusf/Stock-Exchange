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

    std::size_t m_nextId = 0;

public:
    AccountManager();

    std::size_t GetNextAvailableId() const;
    std::unordered_map<std::size_t, Account> GetAccounts() const;
    Account* GetAccount(std::size_t ownerId);

    void AddAccount(std::size_t ownerId, double cashBalance, double assetBalance);
    void UpdateAccounts(const Trade& trade);

    ~AccountManager();

private:


};