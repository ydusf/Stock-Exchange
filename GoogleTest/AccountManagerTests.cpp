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
    std::unique_ptr<SystemMediator> m_systemMediator;


    AccountManagerTestFixture()
    {
        m_systemMediator = std::make_unique<SystemMediator>(std::make_unique<AccountManager>());
    }

    AccountManager* GetAccountManager()
    {
        return m_systemMediator->GetAccountManager();
    }

    OrderBook* GetOrderBook(std::string ticker)
    {
        return m_systemMediator->GetOrderBook(ticker);
    }

    ~AccountManagerTestFixture()
    {

    }
};

TEST_F(AccountManagerTestFixture, AddAccount)
{
    AccountManager* accountManager = GetAccountManager();
    ASSERT_NE(accountManager, nullptr);

    accountManager->AddAccount(0, 2000, 2000);
    accountManager->AddAccount(1, 1000, 2500);
    accountManager->AddAccount(1, 1000, 2500);

    std::unordered_map<std::size_t, std::shared_ptr<Account>> accounts = accountManager->GetAccounts();
    EXPECT_EQ(accounts.size(), 2);

    EXPECT_NE(accounts.find(0), accounts.end());
    EXPECT_EQ(accounts.at(0)->GetCashBalance(), 2000);
    EXPECT_EQ(accounts.at(0)->GetAssetBalance(), 2000);

    EXPECT_NE(accounts.find(1), accounts.end());
    EXPECT_EQ(accounts.at(1)->GetCashBalance(), 1000);
    EXPECT_EQ(accounts.at(1)->GetAssetBalance(), 2500);

    EXPECT_EQ(accounts.find(2), accounts.end());
}

TEST_F(AccountManagerTestFixture, UpdateBalances)
{
    AccountManager* accountManager = GetAccountManager();
    ASSERT_NE(accountManager, nullptr);

    accountManager->AddAccount(0, 2000, 2000);
    accountManager->AddAccount(1, 1000, 2500);

    std::unordered_map<std::size_t, std::shared_ptr<Account>> accounts = accountManager->GetAccounts();
    ASSERT_EQ(accounts.size(), 2);
    ASSERT_NE(accounts.find(0), accounts.end());
    ASSERT_NE(accounts.find(1), accounts.end());

    // Buy order with trade value: 1200, from account with id: 0
    accountManager->UpdateBalances(0, 1200);
    EXPECT_EQ(accounts.at(0)->GetCashBalance(), 800);
    EXPECT_EQ(accounts.at(0)->GetAssetBalance(), 3200);

    // Buy order with trade value: 1800, from account with id: 1
    accountManager->UpdateBalances(1, -1800);
    EXPECT_EQ(accounts.at(1)->GetCashBalance(), 2800);
    EXPECT_EQ(accounts.at(1)->GetAssetBalance(), 700);
}