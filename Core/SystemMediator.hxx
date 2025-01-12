#pragma once

#include "OrderBook.hxx"
#include "AccountManager.hxx"

#include <memory>
#include <string>
#include <unordered_map>

class SystemMediator
{
private:
    std::unique_ptr<AccountManager> m_accountManager;
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> m_orderBooks;

public:
    SystemMediator(std::unique_ptr<AccountManager> accountManager);

    AccountManager* GetAccountManager() const;
    OrderBook* GetOrderBook(std::string ticker) const;

    bool SendOrderRequest(std::size_t id, std::string ticker, Side Side, double quantity, double price);
    void SendCancelRequest(std::size_t ownerId, std::size_t orderId);
    void SendModifyRequest(std::size_t ownerId, std::size_t orderId, double newQuantity, double newPrice);

    ~SystemMediator();

private:
    void RegisterOrderBookCallbacks(std::unique_ptr<OrderBook>& orderBook);
    void on_order_match(const std::shared_ptr<Order>& bid, const std::shared_ptr<Order>& ask, double tradeValue);
    void on_add_order(const std::shared_ptr<Order>& order);

};