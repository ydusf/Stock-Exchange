#pragma once

#include "OrderBook.hxx"

#include <set>

class Account final
{
private:
    std::size_t m_id;
    std::set<std::size_t> m_orderIds;

private:
    std::unordered_map<std::string, double> m_assets;

    double m_cashBalance;
    double m_assetBalance;
    double m_reservedBalance;

public:
    Account(std::size_t id, double initialCashBalance, double initialAssetBalance);

    std::size_t GetId() const;
    double GetNetworth() const;
    double GetCashBalance() const;
    double GetAssetBalance() const;
    double GetReservedBalance() const;

    void UpdateCashBalance(double cashBalanceDelta);
    void UpdateAssetBalance(double assetBalanceDelta);
    void UpdateReservedCash(double reservedCashDelta);
    void UpdateAssetQuantities(const std::string& ticker, double quantityDelta);

    bool CanPlaceOrder(const std::string& ticker, Side side, double quantity, double price) const;
    void AddOrder(const Order& order);
    void RemoveOrder(const Order& order);

    ~Account();

private:
};

