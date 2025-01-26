#pragma once

#include "OrderBook.hxx"
#include "MarketManager.hxx"

#include <set>
#include <unordered_map>

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

    MarketManager& m_marketManager;
    
    Portfolio m_portfolio;

public:
    Account(std::size_t id, MarketManager& marketManager, double initialCashBalance);

    std::size_t GetId() const;
    double GetNetworth();
    Portfolio GetPortfolio();

    void Withdraw(double withdrawal);
    void Insert(double insertion);

    void UpdateBalances(double cashBalanceDelta, double reservedCashDelta);
    void UpdateAssets(const std::string& ticker, double quantityDelta);

    bool CanPlaceOrder(const std::string& ticker, Side side, double quantity, double price) const;
    void AddOrder(const Order& order);
    void RemoveOrder(const Order& order);

    ~Account();

private:
    void UpdateAssetBalance();
};

