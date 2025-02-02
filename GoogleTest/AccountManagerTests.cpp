#include "pch.h"

#include "Core/OrderBook.hxx"
#include "Core/AccountManager.hxx"
#include "Core/Exchange.hxx"

#include <memory>
#include <unordered_map>
#include <cstddef>

class AccountManagerTestFixture : public ::testing::Test
{
protected:
    Exchange m_exchange;
    std::string m_stock = "AAPL";

    AccountManagerTestFixture()
    {
    }

    std::optional<OrderBook*> GetOrderBook(std::string ticker)
    {
        return m_exchange.GetOrderBook(ticker);
    }

    ~AccountManagerTestFixture()
    {

    }
};

TEST_F(AccountManagerTestFixture, AddAccount)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 1000);

    Account* account0 = accountManager->GetAccount(id0);
    EXPECT_NE(account0, nullptr);

    Account* account1 = accountManager->GetAccount(id1);
    EXPECT_NE(account1, nullptr);

    Portfolio portfolio0 = account0->GetPortfolio();
    EXPECT_EQ(portfolio0.m_cashBalance, 2000);
    EXPECT_EQ(portfolio0.m_assetBalance, 0);

    Portfolio portfolio1 = account1->GetPortfolio();
    EXPECT_EQ(portfolio1.m_cashBalance, 1000);
    EXPECT_EQ(portfolio1.m_assetBalance, 0);

    Account* account2 = accountManager->GetAccount(-1);
    EXPECT_EQ(account2, nullptr);
}

TEST_F(AccountManagerTestFixture, PortfolioUpdatedAfterOrderMatch)
{
    AccountManager* accountManager = m_exchange.GetAccountManager();
    MarketManager* marketManager = m_exchange.GetMarketManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, *marketManager, 1000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, *marketManager, 2000);

    Account* account0 = accountManager->GetAccount(id0);
    EXPECT_NE(account0, nullptr);

    Account* account1 = accountManager->GetAccount(id1);
    EXPECT_NE(account1, nullptr);

    m_exchange.AddSeedData(id0, m_stock, 18);

    // Sell order with trade value: 1800, from account with id: 1
    m_exchange.SendOrderRequest(id0, m_stock, OrderType::LimitOrder, Side::Sell, 18, 100);
    // Buy order with trade value: 1200, from account with id: 0
    m_exchange.SendOrderRequest(id1, m_stock, OrderType::LimitOrder, Side::Buy, 12, 100);

    Portfolio portfolio0 = account0->GetPortfolio();
    EXPECT_EQ(portfolio0.m_reservedBalance, 0);
    EXPECT_EQ(portfolio0.m_cashBalance, 2200);
    EXPECT_EQ(portfolio0.m_assetBalance, 600);

    Portfolio portfolio1 = account1->GetPortfolio();
    EXPECT_EQ(portfolio1.m_reservedBalance, 0);
    EXPECT_EQ(portfolio1.m_cashBalance, 800);
    EXPECT_EQ(portfolio1.m_assetBalance, 1200);
}