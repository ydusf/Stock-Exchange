#include "OrderBook.hxx"

OrderBook::OrderBook(std::string ticker) : m_ticker(ticker)
{

}

OrderBook::~OrderBook()
{

}

std::optional<Order> OrderBook::GetOrder(std::size_t id)
{
    auto itr = m_orders.find(id);
    return (itr != m_orders.end()) ? std::optional<Order>{itr->second} : std::nullopt;
}

std::unordered_map<std::size_t, Order> OrderBook::GetOrders()
{
    return m_orders;
}

std::size_t OrderBook::GetVolume()
{
    return m_orders.size();
}

void OrderBook::AddOrder(std::size_t ownerId, OrderType orderType, Side side, double quantity, double price)
{
    std::lock_guard<std::mutex> lock(m_lock);

    if (orderType == OrderType::MarketOrder && !m_asks.empty() && !m_bids.empty())
    {
        price = side == Side::Buy ? m_asks.top().m_price : m_bids.top().m_price;
    }

    Order order = Order(m_ticker, orderType, side, quantity, price, m_nextId, ownerId);
    side == Side::Buy ? m_bids.push(order) : m_asks.push(order);
    m_orders.insert({ m_nextId, order });

    m_addOrderCallback(order);

    m_nextId++;
    MatchOrders();
}

void OrderBook::ModifyOrder(std::size_t orderId, double newQuantity, double newPrice)
{
    std::lock_guard<std::mutex> lock(m_lock);

    auto itr = m_orders.find(orderId);
    if (itr == m_orders.end())
        return;

    Order& order = itr->second;

    if (order.m_status != Status::New && order.m_status != Status::Modified)
        return;

    order.m_quantity = newQuantity;
    order.m_price = newPrice;
    order.m_status = Status::Modified;
    order.UpdateTime();

    ResetQueue();
    MatchOrders();
}

void OrderBook::CancelOrder(std::size_t orderId)
{
    std::lock_guard<std::mutex> lock(m_lock);

    Order& order = m_orders.at(orderId);

    if (order.m_status != Status::New && order.m_status != Status::Modified)
        return;

    order.m_status = Status::Cancelled;
    order.UpdateTime();

    m_orders.erase(orderId);

    ResetQueue();
    MatchOrders();
}

void OrderBook::RegisterOrderMatchCallback(OrderMatchCallback orderMatchCallback)
{
    m_orderMatchCallback = std::move(orderMatchCallback);
}

void OrderBook::RegisterAddOrderCallback(AddOrderCallback addOrderCallback)
{
    m_addOrderCallback = std::move(addOrderCallback);
}

void OrderBook::RegisterUpdateMarketQuoteCallback(UpdateMarketQuoteCallback updateMarketQuoteCallback)
{
    m_updateMarketQuoteCallback = std::move(updateMarketQuoteCallback);
}

void OrderBook::MatchOrders()
{
    // no need to lock because calling funcs lock

    while (!m_bids.empty() && !m_asks.empty())
    {
        Order& bid = m_orders.at(m_bids.top().m_orderId);
        Order& ask = m_orders.at(m_asks.top().m_orderId);

        if (bid.m_price < ask.m_price || bid.m_ownerId == ask.m_ownerId) // illegal match
            return;

        Trade trade(m_ticker, bid, ask, bid.m_price, std::min(bid.m_quantity, ask.m_quantity));

        if (bid.m_quantity > ask.m_quantity)
        {
            bid.m_quantity -= ask.m_quantity;

            bid.m_status = Status::PartiallyFilled;
            ask.m_status = Status::Filled;
        }
        else if (bid.m_quantity < ask.m_quantity)
        {
            ask.m_quantity -= bid.m_quantity;

            bid.m_status = Status::Filled;
            ask.m_status = Status::PartiallyFilled;
        }
        else
        {
            bid.m_status = Status::Filled;
            ask.m_status = Status::Filled;
        }

        m_orderMatchCallback(trade);
        m_updateMarketQuoteCallback(m_ticker, bid.m_price, ask.m_price);

        if (bid.m_status == Status::Filled)
        {
            m_orders.erase(bid.m_orderId);
            m_bids.pop();
        }

        if (ask.m_status == Status::Filled)
        {
            m_orders.erase(ask.m_orderId);
            m_asks.pop();
        }
    }
}

void OrderBook::ResetQueue()
{
    m_bids = BuyOrderQueue{};
    m_asks = SellOrderQueue{};

    for (auto& [id, order] : m_orders)
    {
        order.m_side == Side::Buy ? m_bids.push(order) : m_asks.push(order);
    }
}