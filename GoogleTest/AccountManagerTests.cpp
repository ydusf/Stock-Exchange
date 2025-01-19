#include "pch.h"

#include "OrderBook.hxx"
#include "AccountManager.hxx"
#include "SystemMediator.hxx"

#include <memory>
#include <unordered_map>
#include <cstddef>

class AccountManagerTestFixture : public ::testing::Test
{
protected:
    SystemMediator m_systemMediator;
    std::string m_stock = "AAPL";

    AccountManagerTestFixture()
    {
    }

    AccountManager* GetAccountManager()
    {
        return m_systemMediator.GetAccountManager();
    }

    std::optional<OrderBook*> GetOrderBook(std::string ticker)
    {
        return m_systemMediator.GetOrderBook(ticker);
    }

    ~AccountManagerTestFixture()
    {

    }
};

TEST_F(AccountManagerTestFixture, AddAccount)
{
    AccountManager* accountManager = GetAccountManager();

    accountManager->AddAccount(0, 2000, 2000);
    accountManager->AddAccount(1, 1000, 2500);
    accountManager->AddAccount(1, 1000, 2500);

    Account* account0 = accountManager->GetAccount(0);
    ASSERT_NE(account0, nullptr);

    Account* account1 = accountManager->GetAccount(1);
    ASSERT_NE(account1, nullptr);

    EXPECT_EQ(account0->GetCashBalance(), 2000);
    EXPECT_EQ(account0->GetAssetBalance(), 2000);

    EXPECT_EQ(account1->GetCashBalance(), 1000);
    EXPECT_EQ(account1->GetAssetBalance(), 2500);

    Account* account2 = accountManager->GetAccount(2);
    EXPECT_EQ(account2, nullptr);
}

TEST_F(AccountManagerTestFixture, UpdateBalancesAfterOrder)
{
    AccountManager* accountManager = GetAccountManager();

    accountManager->AddAccount(0, 2000, 2000);
    accountManager->AddAccount(1, 1000, 2500);

    Account* account0 = accountManager->GetAccount(0);
    ASSERT_NE(account0, nullptr);

    Account* account1 = accountManager->GetAccount(1);
    ASSERT_NE(account1, nullptr);

    // Buy order with trade value: 1200, from account with id: 0
    m_systemMediator.SendOrderRequest(0, m_stock, Side::Buy, 12, 100);
    EXPECT_EQ(account0->GetReservedBalance(), 1200);
    EXPECT_EQ(account0->GetCashBalance(), 2000);
    EXPECT_EQ(account0->GetAssetBalance(), 2000);

    // Sell order with trade value: 1800, from account with id: 1
    m_systemMediator.SendOrderRequest(1, m_stock, Side::Sell, 18, 100);
    EXPECT_EQ(account1->GetReservedBalance(), 0);
    EXPECT_EQ(account1->GetCashBalance(), 2200);
    EXPECT_EQ(account1->GetAssetBalance(), 1300);
}