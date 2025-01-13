#include "OrderBook.hxx"

OrderBook::OrderBook()
{

}

OrderBook::~OrderBook()
{

}

std::optional<std::shared_ptr<Order>> OrderBook::GetOrder(std::size_t id) const
{
    std::optional<std::shared_ptr<Order>> foundOrder = std::nullopt;
    return (m_orders.find(id) != m_orders.end()) ? m_orders.at(id) : foundOrder;
}

std::unordered_map<std::size_t, std::shared_ptr<Order>> OrderBook::GetOrders() const
{
    return m_orders;
}

void OrderBook::AddOrder(std::size_t ownerId, std::string ticker, Side side, double quantity, double price)
{
    std::shared_ptr<Order> order = std::make_shared<Order>(ticker, side, quantity, price, m_currOrderId, ownerId);
    side == Side::Buy ? m_bids.push(order) : m_asks.push(order);
    m_orders.insert({ m_currOrderId, order });

    m_addOrderCallback(order);


    m_currOrderId++;

    MatchOrders();
}

void OrderBook::ModifyOrder(std::size_t orderId, double newQuantity, double newPrice)
{
    std::shared_ptr<Order>& order = m_orders.at(orderId);

    if (order->m_status != Status::New && order->m_status != Status::Modified)
        return;

    order->m_quantity = newQuantity;
    order->m_price = newPrice;
    order->m_status = Status::Modified;
    order->UpdateTime();

    ResetQueue();
    MatchOrders();
}

void OrderBook::CancelOrder(std::size_t orderId)
{
    std::shared_ptr<Order>& order = m_orders.at(orderId);

    if (order->m_status != Status::New && order->m_status != Status::Modified)
        return;

    order->m_status = Status::Cancelled;
    order->UpdateTime();

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

void OrderBook::MatchOrders()
{
    while (!m_bids.empty() && !m_asks.empty())
    {
        std::shared_ptr<Order>& bid = m_orders.at(m_bids.top()->m_orderId);
        std::shared_ptr<Order>& ask = m_orders.at(m_asks.top()->m_orderId);

        if (bid->m_ownerId == ask->m_ownerId)
            return;

        if (bid->m_price >= ask->m_price) // match is found
        {
            if (bid->m_quantity > ask->m_quantity) // bid has more quantity than ask       
            {
                bid->m_quantity -= ask->m_quantity;

                bid->m_status = Status::PartiallyFilled;
                ask->m_status = Status::Filled;

                m_orderMatchCallback(bid, ask, bid->m_price * ask->m_quantity);

                m_orders.erase(ask->m_orderId);
                m_asks.pop();
            }
            else if (bid->m_quantity < ask->m_quantity) // ask has more quantity than bid         
            {
                ask->m_quantity -= bid->m_quantity;

                bid->m_status = Status::Filled;
                ask->m_status = Status::PartiallyFilled;

                m_orderMatchCallback(bid, ask, bid->m_price * bid->m_quantity);

                m_orders.erase(bid->m_orderId);
                m_bids.pop();
            }
            else // both have same quantity
            {
                bid->m_status = Status::Filled;
                ask->m_status = Status::Filled;

                m_orderMatchCallback(bid, ask, bid->m_price * bid->m_quantity);

                m_orders.erase(ask->m_orderId);
                m_orders.erase(bid->m_orderId);

                m_bids.pop();
                m_asks.pop();
            }
        }
        else
        {
            break; // must break out of while loop as no matches will be found past this point
        }
    }
}

void OrderBook::ResetQueue()
{
    m_bids = BuyOrderQueue{};
    m_asks = SellOrderQueue{};

    for (auto& [id, order] : m_orders)
    {
        order->m_side == Side::Buy ? m_bids.push(order) : m_asks.push(order);
    }
}