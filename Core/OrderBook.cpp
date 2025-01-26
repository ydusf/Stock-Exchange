#include "OrderBook.hxx"

OrderBook::OrderBook(std::string ticker) : m_ticker(ticker)
{
    
}

OrderBook::~OrderBook()
{

}

std::optional<Order> OrderBook::GetOrder(std::size_t id) const
{
    auto itr = m_orders.find(id);
    return (itr != m_orders.end()) ? std::optional<Order>{itr->second} : std::nullopt;
}

std::unordered_map<std::size_t, Order> OrderBook::GetOrders() const
{
    return m_orders;
}

void OrderBook::AddOrder(std::size_t ownerId, const std::string& ticker, OrderType orderType, Side side, double quantity, double price)
{
    if (ticker != m_ticker)
        throw std::logic_error("ticker of order and order book must be the same");
    
    Order order = Order(ticker, orderType, side, quantity, price, m_nextId, ownerId);
    side == Side::Buy ? m_bids.push(order) : m_asks.push(order);
    m_orders.insert({ m_nextId, order });

    m_addOrderCallback(order);

    m_nextId++;
    MatchOrders();
}

void OrderBook::ModifyOrder(std::size_t orderId, double newQuantity, double newPrice)
{
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