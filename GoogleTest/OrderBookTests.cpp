#include "pch.h"

#include "OrderBook.hxx"
#include "AccountManager.hxx"
#include "Exchange.hxx"

#include <memory>

class OrderBookTestFixture : public testing::Test
{
protected:
    Exchange m_exchange;
    std::string m_stock = "AAPL";

    OrderBookTestFixture()
    {
        
    }

    ~OrderBookTestFixture()
    {

    }
};

static std::vector<Order> GetOrders(OrderBook& orderBook)
{
    std::unordered_map<std::size_t, Order> orderMap = orderBook.GetOrders();

    std::vector<Order> orders;
    orders.reserve(orderMap.size());

    for (auto& [id, order] : orderMap) 
    {
        orders.push_back(order);
    }

    return orders;
}

TEST_F(OrderBookTestFixture, AddOrder)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();


    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    Side side = Side::Sell;
    double quantity = 30;
    double price = 20;
    m_exchange.AddSeedData(id0, m_stock, 30);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, side, quantity, price);

    OrderBook* orderBook = m_exchange.GetOrderBook(m_stock);
    EXPECT_NE(orderBook, nullptr);

    std::vector<Order> orders = GetOrders(*orderBook);
    ASSERT_EQ(orders.size(), 1);

    EXPECT_EQ(orders.at(0).m_side, side);
    EXPECT_EQ(orders.at(0).m_quantity, quantity);
    EXPECT_EQ(orders.at(0).m_price, price);
}

TEST_F(OrderBookTestFixture, SellOrderFilled)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    m_exchange.AddSeedData(id0, m_stock, 50);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 30, 20);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 20, 18);
    m_exchange.SendOrderRequest(id1, m_stock, OrderType::LimitOrder, Side::Buy, 25, 20);

    OrderBook* orderBook = m_exchange.GetOrderBook(m_stock);
    EXPECT_NE(orderBook, nullptr);

    std::unordered_map<std::size_t, Order> orders = orderBook->GetOrders();

    ASSERT_EQ(orders.size(), 1);
    EXPECT_NE(orders.find(0), orders.end());
    ASSERT_EQ(orders.find(1), orders.end());
    ASSERT_EQ(orders.find(2), orders.end());

    EXPECT_EQ(orders.at(0).m_quantity, 25);
    EXPECT_EQ(orders.at(0).m_status, Status::PartiallyFilled);
}

TEST_F(OrderBookTestFixture, BuyOrderFilled)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    m_exchange.AddSeedData(id0, m_stock, 50);

    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 30, 20);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 20, 18);
    m_exchange.SendOrderRequest(id1, m_stock, OrderType::LimitOrder, Side::Buy, 15, 20);

    OrderBook* orderBook = m_exchange.GetOrderBook(m_stock);
    EXPECT_NE(orderBook, nullptr);

    std::unordered_map<std::size_t, Order> orders = orderBook->GetOrders();

    ASSERT_EQ(orders.size(), 2);
    EXPECT_NE(orders.find(0), orders.end());
    EXPECT_NE(orders.find(1), orders.end());
    ASSERT_EQ(orders.find(2), orders.end());

    EXPECT_EQ(orders.at(0).m_quantity, 30);
    EXPECT_EQ(orders.at(0).m_status, Status::New);
    EXPECT_EQ(orders.at(1).m_quantity, 5);
    EXPECT_EQ(orders.at(1).m_status, Status::PartiallyFilled);
}

TEST_F(OrderBookTestFixture, OrdersForDifferentStocksMatched)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    std::string aapl = "AAPL";
    std::string tsla = "TSLA";

    m_exchange.AddSeedData(id0, aapl, 30);
    m_exchange.AddSeedData(id1, tsla, 15);

    m_exchange.SendOrderRequest(id0, aapl, OrderType::LimitOrder, Side::Sell, 30, 20);
    m_exchange.SendOrderRequest(id0, tsla, OrderType::LimitOrder, Side::Buy, 20, 25);
    m_exchange.SendOrderRequest(id1, aapl, OrderType::LimitOrder, Side::Buy, 15, 20);
    m_exchange.SendOrderRequest(id1, tsla, OrderType::LimitOrder, Side::Sell, 15, 20);

    OrderBook* aaplOrderBook = m_exchange.GetOrderBook(aapl);
    EXPECT_NE(aaplOrderBook, nullptr);

    OrderBook* tslaOrderBook = m_exchange.GetOrderBook(tsla);
    EXPECT_NE(tslaOrderBook, nullptr);

    std::unordered_map<std::size_t, Order> aaplOrders = aaplOrderBook->GetOrders();

    ASSERT_EQ(aaplOrders.size(), 1);
    EXPECT_NE(aaplOrders.find(0), aaplOrders.end());
    ASSERT_EQ(aaplOrders.find(1), aaplOrders.end());

    EXPECT_EQ(aaplOrders.at(0).m_quantity, 15);
    EXPECT_EQ(aaplOrders.at(0).m_status, Status::PartiallyFilled);

    std::unordered_map<std::size_t, Order> tslaOrders = tslaOrderBook->GetOrders();
    ASSERT_EQ(tslaOrders.size(), 1);
    EXPECT_NE(tslaOrders.find(0), tslaOrders.end());
    ASSERT_EQ(tslaOrders.find(1), tslaOrders.end());

    EXPECT_EQ(tslaOrders.at(0).m_quantity, 5);
    EXPECT_EQ(tslaOrders.at(0).m_status, Status::PartiallyFilled);
}

TEST_F(OrderBookTestFixture, OrdersForDifferentStocksNotMatched)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    std::string aapl = "AAPL";
    std::string tsla = "TSLA";

    m_exchange.AddSeedData(id0, aapl, 30);

    m_exchange.SendOrderRequest(id0, aapl, OrderType::LimitOrder, Side::Sell, 30, 20);
    m_exchange.SendOrderRequest(id1, tsla, OrderType::LimitOrder, Side::Buy, 20, 25);

    OrderBook* aaplOrderBook = m_exchange.GetOrderBook(aapl);
    EXPECT_NE(aaplOrderBook, nullptr);
    EXPECT_EQ(aaplOrderBook->GetOrders().size(), 1);

    OrderBook* tslaOrderBook = m_exchange.GetOrderBook(tsla);
    EXPECT_NE(tslaOrderBook, nullptr);
    EXPECT_EQ(tslaOrderBook->GetOrders().size(), 1);
}

TEST_F(OrderBookTestFixture, BothOrdersFilled)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    m_exchange.AddSeedData(id0, m_stock, 50);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 30, 20);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 20, 18);
    m_exchange.SendOrderRequest(id1, m_stock, OrderType::LimitOrder, Side::Buy, 20, 19);

    OrderBook* orderBook = m_exchange.GetOrderBook(m_stock);
    EXPECT_NE(orderBook, nullptr);

    std::unordered_map<std::size_t, Order> orders = orderBook->GetOrders();

    ASSERT_EQ(orders.size(), 1);
    EXPECT_NE(orders.find(0), orders.end());
    ASSERT_EQ(orders.find(1), orders.end());
    ASSERT_EQ(orders.find(2), orders.end());

    EXPECT_EQ(orders.at(0).m_quantity, 30);
    EXPECT_EQ(orders.at(0).m_status, Status::New);
}

TEST_F(OrderBookTestFixture, OrderBookComplexMatching)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    std::vector<std::size_t> tradingEntities{ id0, id1, id1, id1, id1, id0, id0, id0, id1, id1 };
    std::vector<Side> sides{ Side::Buy, Side::Buy, Side::Sell, Side::Sell, Side::Buy, Side::Buy, Side::Sell, Side::Sell, Side::Buy, Side::Sell };
    std::vector<double> quantities{ 20, 14, 17, 30, 35, 14, 34, 12, 25, 16 };
    std::vector<double> prices{ 10, 12, 11.5, 13.2, 11.3, 14.2, 10.6, 10.76, 10.98, 15 };

    m_exchange.AddSeedData(id0, m_stock, 2000);
    m_exchange.AddSeedData(id1, m_stock, 2000);

    for (std::size_t i = 0; i < 10; ++i)
    {
        m_exchange.SendOrderRequest(tradingEntities[i], m_stock, OrderType::LimitOrder, sides[i], quantities[i], prices[i]);
    }

    OrderBook* orderBook = m_exchange.GetOrderBook(m_stock);
    EXPECT_NE(orderBook, nullptr);

    std::unordered_map<std::size_t, Order> orders = orderBook->GetOrders();

    ASSERT_EQ(orders.size(), 6);
    EXPECT_NE(orders.find(0), orders.end());
    ASSERT_EQ(orders.find(1), orders.end());
    EXPECT_NE(orders.find(2), orders.end());
    EXPECT_NE(orders.find(3), orders.end());
    EXPECT_NE(orders.find(4), orders.end());
    ASSERT_EQ(orders.find(5), orders.end());
    ASSERT_EQ(orders.find(6), orders.end());
    ASSERT_EQ(orders.find(7), orders.end());
    EXPECT_NE(orders.find(8), orders.end());
    EXPECT_NE(orders.find(9), orders.end());
}

TEST_F(OrderBookTestFixture, OrderModified)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    m_exchange.AddSeedData(id0, m_stock, 50);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 30, 20);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 20, 18);
    m_exchange.SendOrderRequest(id1, m_stock, OrderType::LimitOrder, Side::Buy, 20, 16);

    OrderBook* orderBook = m_exchange.GetOrderBook(m_stock);
    EXPECT_NE(orderBook, nullptr);

    orderBook->ModifyOrder(2, 25, 19);

    std::unordered_map<std::size_t, Order> orders = orderBook->GetOrders();

    ASSERT_EQ(orders.size(), 2);
    EXPECT_NE(orders.find(0), orders.end());
    ASSERT_EQ(orders.find(1), orders.end());
    EXPECT_NE(orders.find(2), orders.end());

    EXPECT_EQ(orders.at(0).m_quantity, 30);
    EXPECT_EQ(orders.at(0).m_status, Status::New);
    EXPECT_EQ(orders.at(2).m_quantity, 5);
    EXPECT_EQ(orders.at(2).m_status, Status::PartiallyFilled);
}

TEST_F(OrderBookTestFixture, OrderCancelled)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    m_exchange.AddSeedData(id0, m_stock, 50);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 30, 20);
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 20, 18);
    m_exchange.SendOrderRequest(id1, m_stock, OrderType::LimitOrder, Side::Buy, 20, 16);

    OrderBook* orderBook = m_exchange.GetOrderBook(m_stock);
    EXPECT_NE(orderBook, nullptr);

    orderBook->CancelOrder(0);

    std::unordered_map<std::size_t, Order> orders = orderBook->GetOrders();

    ASSERT_EQ(orders.size(), 2);
    ASSERT_EQ(orders.find(0), orders.end());
    EXPECT_NE(orders.find(1), orders.end());
    EXPECT_NE(orders.find(2), orders.end());

    EXPECT_EQ(orders.at(1).m_quantity, 20);
    EXPECT_EQ(orders.at(1).m_status, Status::New);
    EXPECT_EQ(orders.at(2).m_quantity, 20);
    EXPECT_EQ(orders.at(2).m_status, Status::New);
}
