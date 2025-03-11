#pragma once

#include "Account.hxx"

#include <unordered_map>
#include <memory>
#include <cstddef>
#include <optional>
#include <atomic>

class AccountManager final
{
private:
    std::unordered_map<std::size_t, Account> m_accounts;

    std::atomic<std::size_t> m_nextId = 0;

public:
    AccountManager();

    std::size_t GetNextAvailableId();
    std::unordered_map<std::size_t, Account> GetAccounts();
    Account* GetAccount(std::size_t ownerId);

    void AddAccount(std::size_t ownerId, MarketManager& marketManager, double cashBalance);
    void UpdateAccounts(const Trade& trade, OrderBook* orderBook);

    ~AccountManager() = default;

private:


};