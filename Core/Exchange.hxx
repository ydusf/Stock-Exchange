#pragma once

#include "OrderBook.hxx"
#include "AccountManager.hxx"

#include <memory>
#include <string>
#include <unordered_map>

class Exchange final
{
private:
    AccountManager m_accountManager;
    std::unordered_map<std::string, OrderBook> m_orderBooks;

public:
    Exchange();

    AccountManager* GetAccountManager();
    OrderBook* GetOrderBook(std::string& ticker);

    bool SendOrderRequest(std::size_t ownerId, std::string& ticker, Side Side, double quantity, double price);
    void SendCancelRequest(std::size_t ownerId, std::size_t orderId);
    void SendModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice);

    MarketQuote QueryMarketData(std::string& ticker);

    void AddSeedData(std::size_t id, std::string& ticker, double quantity);

    ~Exchange();

private:
    void RegisterOrderBookCallbacks(OrderBook& orderBook);
    bool on_order_match(const Trade& trade);
    bool on_add_order(const Order& order);

};