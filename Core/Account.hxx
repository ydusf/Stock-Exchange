#pragma once

#include "OrderBook.hxx"

#include <set>

class Account
{
private:
    std::set<std::size_t> m_orderIds;

private:
    double m_cashBalance;
    double m_assetBalance;

public:
    Account(double initialCashBalance, double initialAssetBalance);

    double GetCashBalance() const;
    double GetAssetBalance() const;

    void SetCashBalance(double newCashBalance);
    void SetAssetBalance(double newAssetBalance);

    void AddOrder(std::size_t orderId);
    void RemoveOrder(std::size_t orderId);

    ~Account();

private:
};

