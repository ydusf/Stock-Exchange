#pragma once

#include <iostream>
#include <ctime>
#include <vector>
#include <queue>
#include <cstddef>
#include <unordered_map>
#include <optional>
#include <string>
#include <memory>
#include <functional>

enum class Side
{
    Buy,
    Sell
};

enum class Status
{
    New,
    Filled,
    PartiallyFilled,
    Modified,
    Cancelled
};

struct Order
{
    std::string m_ticker;
    Side m_side;
    Status m_status;
    double m_quantity;
    double m_price;
    std::time_t m_timestamp;
    std::size_t m_orderId;
    std::size_t m_ownerId;

    Order(std::string ticker, Side side, double quantity, double price, std::size_t orderId, std::size_t ownerId)
        : m_ticker(ticker),
        m_side(side),
        m_status(Status::New),
        m_quantity(quantity),
        m_price(price),
        m_orderId(orderId),
        m_ownerId(ownerId)
    {
        UpdateTime();
    };

    void UpdateTime()
    {
        m_timestamp = std::time(nullptr);
    }
};

class OrderBook
{
    struct BuyOrderComparator
    {
        bool operator()(const std::shared_ptr<Order>& lhs, const std::shared_ptr<Order>& rhs) const
        {
            return (lhs->m_price != rhs->m_price) ? lhs->m_price < rhs->m_price : lhs->m_timestamp < rhs->m_timestamp;
        }
    };

    struct SellOrderComparator
    {
        bool operator()(const std::shared_ptr<Order>& lhs, const std::shared_ptr<Order>& rhs) const
        {
            return (lhs->m_price != rhs->m_price) ? lhs->m_price > rhs->m_price : lhs->m_timestamp < rhs->m_timestamp;
        }
    };

    typedef std::priority_queue<std::shared_ptr<Order>, std::vector<std::shared_ptr<Order>>, BuyOrderComparator> BuyOrderQueue;
    typedef std::priority_queue<std::shared_ptr<Order>, std::vector<std::shared_ptr<Order>>, SellOrderComparator> SellOrderQueue;

    typedef std::function<void(const std::shared_ptr<Order>& bid, const std::shared_ptr<Order>& ask, double tradeValue)> OrderMatchCallback;
    typedef std::function<void(const std::shared_ptr<Order>& order)> AddOrderCallback;

private: // attributes
    std::size_t m_currOrderId = 0;

    OrderMatchCallback m_orderMatchCallback;
    AddOrderCallback m_addOrderCallback;

    std::unordered_map<std::size_t, std::shared_ptr<Order>> m_orders;
    BuyOrderQueue m_bids;
    SellOrderQueue m_asks;

public: // methods
    OrderBook();

    std::optional<std::shared_ptr<Order>> GetOrder(std::size_t id) const;
    std::unordered_map<std::size_t, std::shared_ptr<Order>> GetOrders() const;

    void AddOrder(std::size_t ownerId, std::string ticker, Side side, double quantity, double price);
    void ModifyOrder(std::size_t orderId, double newQuantity, double newPrice);
    void CancelOrder(std::size_t orderId);

    void RegisterOrderMatchCallback(OrderMatchCallback orderMatchCallback);
    void RegisterAddOrderCallback(AddOrderCallback addOrderCallback);

    ~OrderBook();

private: // methods
    void MatchOrders();
    void ResetQueue();
};