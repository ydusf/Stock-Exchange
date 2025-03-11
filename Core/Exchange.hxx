#pragma once

#include "OrderBook.hxx"
#include "AccountManager.hxx"
#include "MarketManager.hxx"

#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mutex>

class Exchange final
{
private:
    std::mutex m_lock;

    std::unique_ptr<AccountManager> m_accountManager;
    std::unique_ptr<MarketManager> m_marketManager;
    std::unordered_map<std::string, OrderBook> m_orderBooks;

public:
    Exchange();

    MarketManager* GetMarketManager();
    AccountManager* GetAccountManager();
    OrderBook* GetOrderBook(const std::string& ticker);

    bool SendOrderRequest(std::size_t ownerId, const std::string& ticker, OrderType orderType, Side Side, double quantity, double price);
    bool SendCancelRequest(std::size_t ownerId, std::size_t orderId);
    bool SendModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice);

    bool ProcessOrderRequest(std::size_t ownerId, const std::string& ticker, OrderType orderType, Side side, double quantity, double price);
    bool ProcessCancelRequest(std::size_t ownerId, std::size_t orderId);
    bool ProcessModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice);

    void AddSeedData(std::size_t id, const std::string& ticker, double quantity);

    ~Exchange();

private:
    void RegisterOrderBookCallbacks(OrderBook& orderBook);
    bool on_order_match(const Trade& trade);
    bool on_add_order(const Order& order);
    void on_update_market_quote(const std::string& ticker, double topBid, double topAsk);

};