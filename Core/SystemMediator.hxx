#pragma once

#include "OrderBook.hxx"
#include "AccountManager.hxx"

#include <memory>
#include <string>
#include <unordered_map>

class SystemMediator final
{
private:
    AccountManager m_accountManager;
    std::unordered_map<std::string, OrderBook> m_orderBooks;

public:
    SystemMediator();

    AccountManager* GetAccountManager();
    OrderBook* GetOrderBook(std::string ticker);

    bool SendOrderRequest(std::size_t id, std::string ticker, Side Side, double quantity, double price);
    void SendCancelRequest(std::size_t ownerId, std::size_t orderId);
    void SendModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice);

    ~SystemMediator();

private:
    void RegisterOrderBookCallbacks(OrderBook& orderBook);
    void on_order_match(const Order& bid, const Order& ask, double tradeValue);
    void on_add_order(const Order& order);

};