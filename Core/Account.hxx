#pragma once

#include "OrderBook.hxx"

#include <set>

struct Portfolio
{
    double m_cashBalance;
    double m_assetBalance;
    double m_reservedBalance;
    std::unordered_map<std::string, double> m_assets;
};

class Account final
{
private:
    std::size_t m_id;
    std::set<std::size_t> m_orderIds;

private:
    Portfolio m_portfolio;

public:
    Account(std::size_t id, double initialCashBalance, double initialAssetBalance);

    std::size_t GetId() const;
    double GetNetworth() const;
    Portfolio GetPortfolio() const;

    void UpdatePortfolio(double cashBalanceDelta, double assetBalanceDelta, double reservedCashDelta);
    void UpdateAssetQuantities(const std::string& ticker, double quantityDelta);

    bool CanPlaceOrder(const std::string& ticker, Side side, double quantity, double price) const;
    void AddOrder(const Order& order);
    void RemoveOrder(const Order& order);

    ~Account();

private:
};

