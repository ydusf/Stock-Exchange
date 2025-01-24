#include "pch.h"

#include "OrderBook.hxx"
#include "AccountManager.hxx"
#include "Exchange.hxx"

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

    AccountManager* GetAccountManager()
    {
        return m_exchange.GetAccountManager();
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
    AccountManager* accountManager = GetAccountManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, 2000, 2000);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, 1000, 2500);

    Account* account0 = accountManager->GetAccount(id0);
    ASSERT_NE(account0, nullptr);

    Account* account1 = accountManager->GetAccount(id1);
    ASSERT_NE(account1, nullptr);

    Portfolio portfolio0 = account0->GetPortfolio();
    EXPECT_EQ(portfolio0.m_cashBalance, 2000);
    EXPECT_EQ(portfolio0.m_assetBalance, 2000);

    Portfolio portfolio1 = account1->GetPortfolio();
    EXPECT_EQ(portfolio1.m_cashBalance, 1000);
    EXPECT_EQ(portfolio1.m_assetBalance, 2500);

    Account* account2 = accountManager->GetAccount(-1);
    EXPECT_EQ(account2, nullptr);
}

TEST_F(AccountManagerTestFixture, PortfolioUpdatedAfterOrderMatch)
{
    AccountManager* accountManager = GetAccountManager();

    std::size_t id0 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id0, 1000, 2500);

    std::size_t id1 = accountManager->GetNextAvailableId();
    accountManager->AddAccount(id1, 2000, 2000);

    Account* account0 = accountManager->GetAccount(id0);
    ASSERT_NE(account0, nullptr);

    Account* account1 = accountManager->GetAccount(id1);
    ASSERT_NE(account1, nullptr);

    m_exchange.AddSeedData(id0, m_stock, 18);

    // Sell order with trade value: 1800, from account with id: 1
    m_exchange.SendOrderRequest(id0, m_stock, Side::Sell, 18, 100);
    // Buy order with trade value: 1200, from account with id: 0
    m_exchange.SendOrderRequest(id1, m_stock, Side::Buy, 12, 100);

    Portfolio portfolio0 = account0->GetPortfolio();
    EXPECT_EQ(portfolio0.m_reservedBalance, 0);
    EXPECT_EQ(portfolio0.m_cashBalance, 2200);
    EXPECT_EQ(portfolio0.m_assetBalance, 1300);

    Portfolio portfolio1 = account1->GetPortfolio();
    EXPECT_EQ(portfolio1.m_reservedBalance, 0);
    EXPECT_EQ(portfolio1.m_cashBalance, 800);
    EXPECT_EQ(portfolio1.m_assetBalance, 3200);
}