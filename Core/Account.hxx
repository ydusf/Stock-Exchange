#pragma once

#include "OrderBook.hxx"

#include <set>

class Account final
{
private:
    std::set<std::size_t> m_orderIds;

private:
    double m_cashBalance;
    double m_assetBalance;
    double m_reservedBalance;

public:
    Account(double initialCashBalance, double initialAssetBalance);

    double GetNetworth() const;
    double GetCashBalance() const;
    double GetAssetBalance() const;
    double GetReservedBalance() const;

    void SetCashBalance(double newCashBalance);
    void SetAssetBalance(double newAssetBalance);
    void UpdateReservedCash(double reservedCashDelta);

    bool CanPlaceOrder(Side side, double amount) const;
    void AddOrder(std::size_t orderId);
    void RemoveOrder(std::size_t orderId);

    ~Account();

private:
};

