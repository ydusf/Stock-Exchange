#include "SystemMediator.hxx"

#include <iostream>
#include <random>
#include <chrono>
#include <memory>
#include <algorithm>

static void TimingDiagnostics()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> sideDist(0, 1);
    std::uniform_real_distribution<> quantityDist(1, 100);
    std::uniform_real_distribution<> priceDist(1, 35);

    std::vector<Side> sides;
    std::vector<double> quantities;
    std::vector<double> prices;
    std::vector<std::chrono::nanoseconds> latencies;

    std::size_t OPERATIONS = 1000000;

    sides.reserve(OPERATIONS);
    quantities.reserve(OPERATIONS);
    prices.reserve(OPERATIONS);
    latencies.reserve(OPERATIONS);

    for (std::size_t i = 0; i < OPERATIONS; ++i)
    {
        Side side = sideDist(gen) > 0.5 ? Side::Buy : Side::Sell;
        sides.push_back(side);

        double quantity = quantityDist(gen);
        quantities.push_back(quantity);

        double price = priceDist(gen);
        prices.push_back(price);
    }

    SystemMediator systemMediator(std::make_unique<AccountManager>());
    AccountManager* accountManager = systemMediator.GetAccountManager();

    accountManager->AddAccount(0, 1000000, 1000000);
    accountManager->AddAccount(1, 1000000, 1000000);

    auto startTotal = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < OPERATIONS; ++i)
    {
        std::size_t account = sideDist(gen) > 0.5 ? 0 : 1;

        auto startOp = std::chrono::high_resolution_clock::now();
        systemMediator.SendOrderRequest(account, "AAPL", sides[i], quantities[i], prices[i]);
        auto endOp = std::chrono::high_resolution_clock::now();

        latencies.push_back(endOp - startOp);
    }

    auto endTotal = std::chrono::high_resolution_clock::now();

    // Calculate total duration
    auto totalDuration = endTotal - startTotal;
    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(totalDuration).count();
    auto avgNs = std::chrono::duration_cast<std::chrono::nanoseconds>(totalDuration).count() / static_cast<double>(OPERATIONS);

    // Sort latencies for percentile calculations
    std::sort(latencies.begin(), latencies.end());

    // Calculate percentiles
    auto& p50 = latencies[OPERATIONS * 0.50];
    auto& p90 = latencies[OPERATIONS * 0.90];
    auto& p95 = latencies[OPERATIONS * 0.95];
    auto& p99 = latencies[OPERATIONS * 0.99];
    auto& maxLatency = latencies.back();

    // Calculate mean latency
    std::chrono::nanoseconds totalLatency(0);
    for (const auto& latency : latencies) {
        totalLatency += latency;
    }
    auto meanLatency = totalLatency / OPERATIONS;

    // Print results
    std::cout << "Performance Results:\n";
    std::cout << "-------------------\n";
    std::cout << "Total time: " << totalMs << "ms for " << OPERATIONS << " operations\n";
    std::cout << "Average time per operation: " << avgNs << "ns\n\n";

    std::cout << "Latency Distribution:\n";
    std::cout << "--------------------\n";
    std::cout << "Mean: " << meanLatency.count() << "ns\n";
    std::cout << "P50:  " << p50.count() << "ns\n";
    std::cout << "P90:  " << p90.count() << "ns\n";
    std::cout << "P95:  " << p95.count() << "ns\n";
    std::cout << "P99:  " << p99.count() << "ns\n";
    std::cout << "Max:  " << maxLatency.count() << "ns\n";
}

static void PrintOrderQuantity(Side Side, double quantity)
{
    std::string colourCode = Side == Side::Sell ? "\033[31m" : "\033[32m";

    std::cout << colourCode + std::string(quantity, '#') + "\033[0m";
};

static void PrintOrderPrice(double price)
{
    std::cout << price;
};

static void PrintNewLine()
{
    std::cout << '\n';
}

static void PrintSpace()
{
    std::cout << " ";
}

static void LaunchApplication()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> SideDist(0, 1);
    std::uniform_real_distribution<> quantityDist(1, 100);
    std::uniform_real_distribution<> priceDist(50, 100);

    std::vector<Side> sides;
    std::vector<double> quantities;
    std::vector<double> prices;

    std::size_t OPERATIONS = 5;

    for (std::size_t i = 0; i < OPERATIONS; ++i)
    {
        Side Side = SideDist(gen) > 0.5 ? Side::Buy : Side::Sell;
        sides.push_back(Side);

        double quantity = quantityDist(gen);
        quantities.push_back(quantity);

        double price = priceDist(gen);
        prices.push_back(price);
    }

    SystemMediator systemMediator(std::make_unique<AccountManager>());

    AccountManager* accountManager = systemMediator.GetAccountManager();

    accountManager->AddAccount(0, 2000, 2000);
    accountManager->AddAccount(1, 2000, 2000);

    std::cout << "Trading has begun!" << '\n';
    PrintNewLine();
    for (std::size_t i = 0; i < OPERATIONS; ++i)
    {
        std::size_t id = SideDist(gen) > 0.5 ? 0 : 1;

        bool success = systemMediator.SendOrderRequest(id, "AAPL", sides[i], quantities[i], prices[i]);

        if (!success)
            continue;

        PrintOrderQuantity(sides[i], quantities[i]);
        PrintSpace();
        std::cout << quantities[i];
        PrintSpace();
        PrintOrderPrice(prices[i]);
        PrintSpace();
        std::cout << id;
        PrintNewLine();
    }
    PrintNewLine();
    std::cout << "Trading has ended!" << '\n';
    PrintNewLine();
    std::cout << "Bot1 balance: " << (*accountManager->GetAccount(0))->GetCashBalance() << "; Bot2 balance: " << (*accountManager->GetAccount(1))->GetCashBalance() << '\n';
    std::cout << "Bot1 investments: " << (*accountManager->GetAccount(0))->GetAssetBalance() << "; Bot2 investments: " << (*accountManager->GetAccount(1))->GetAssetBalance() << '\n';
}

int main()
{
    TimingDiagnostics();
    //LaunchApplication();

    return 0;
}