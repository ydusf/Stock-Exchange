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
#include <mutex>

enum class Side
{
    Buy,
    Sell,
};

enum class Status
{
    New,
    Filled,
    PartiallyFilled,
    Modified,
    Cancelled,
};

enum class OrderType
{
    MarketOrder,
    LimitOrder,
    StopLossOrder,
};

struct Order
{
    std::string m_ticker;
    OrderType m_orderType;
    Side m_side;
    Status m_status;
    double m_quantity;
    double m_price;
    std::time_t m_timestamp;
    std::size_t m_orderId;
    std::size_t m_ownerId;

    Order(std::string ticker, OrderType orderType, Side side, double quantity, double price, std::size_t orderId, std::size_t ownerId)
        : m_ticker(ticker),
          m_orderType(orderType),
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

struct Trade
{
    const std::string& ticker;
    const Order& m_buyer;
    const Order& m_seller;
    double m_price;
    double m_quantity;
};

namespace Comparator
{
    struct BuyOrderComparator
    {
        bool operator()(const Order& lhs, const Order& rhs) const
        {
            return (lhs.m_price != rhs.m_price) ? lhs.m_price < rhs.m_price : lhs.m_timestamp < rhs.m_timestamp;
        }
    };

    struct SellOrderComparator
    {
        bool operator()(const Order& lhs, const Order& rhs) const
        {
            return (lhs.m_price != rhs.m_price) ? lhs.m_price > rhs.m_price : lhs.m_timestamp < rhs.m_timestamp;
        }
    };
}

class OrderBook final
{
    typedef std::priority_queue<Order, std::vector<Order>, Comparator::BuyOrderComparator> BuyOrderQueue;
    typedef std::priority_queue<Order, std::vector<Order>, Comparator::SellOrderComparator> SellOrderQueue;

    typedef std::function<bool(const Trade& trade)> OrderMatchCallback;
    typedef std::function<bool(const Order& order)> AddOrderCallback;
    typedef std::function<void(const std::string& ticker, double topBid, double topAsk)> UpdateMarketQuoteCallback;

private: // attributes
    std::size_t m_nextId = 0;

    std::string m_ticker;

    OrderMatchCallback m_orderMatchCallback;
    AddOrderCallback m_addOrderCallback;
    UpdateMarketQuoteCallback m_updateMarketQuoteCallback;

    std::unordered_map<std::size_t, Order> m_orders;
    BuyOrderQueue m_bids;
    SellOrderQueue m_asks;

public: // methods
    OrderBook(std::string ticker);
    std::optional<Order> GetOrder(std::size_t id) const;
    std::unordered_map<std::size_t, Order> GetOrders() const;

    void AddOrder(std::size_t ownerId, const std::string& ticker, OrderType orderType, Side side, double quantity, double price);
    void ModifyOrder(std::size_t orderId, double newQuantity, double newPrice);
    void CancelOrder(std::size_t orderId);

    void RegisterOrderMatchCallback(OrderMatchCallback orderMatchCallback);
    void RegisterAddOrderCallback(AddOrderCallback addOrderCallback);
    void RegisterUpdateMarketQuoteCallback(UpdateMarketQuoteCallback updateMarketQuoteCallback);

    ~OrderBook();

private: // methods
    void MatchOrders();
    void ResetQueue();
};