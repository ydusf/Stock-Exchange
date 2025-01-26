#include "Exchange.hxx"
#include "TradingStrategy.hxx"

#include <iostream>
#include <random>
#include <chrono>
#include <memory>
#include <algorithm>

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>

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

static std::vector<std::vector<std::string>> ParseStockCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> data;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << '\n';
        return data;
    }

    while (std::getline(file, line))
    {
        std::vector<std::string> row;
        std::stringstream lineStream(line);
        std::string cell;

        while (std::getline(lineStream, cell, ','))
        {
            row.push_back(cell);
        }

        data.push_back(row);
    }

    file.close();
    return data;
}

static void RetrieveData
(
    std::vector<std::string>& userIds,
    std::vector<std::string>& stocks,
    std::vector<Side>& sides,
    std::vector<double>& quantities,
    std::vector<double>& prices
)
{
    std::string filename = "stock_orders250k.csv";
    auto csvData = ParseStockCSV(filename);

    bool isHeader = true;

    for (const auto& row : csvData)
    {
        if (isHeader)
        {
            isHeader = false;
            continue;
        }

        if (row.size() >= 6)
        {
            std::string userId = row[1];
            userIds.push_back(userId);

            Side side = (row[2] == "Bid") ? Side::Buy : Side::Sell;
            sides.push_back(side);

            std::string stock = row[3];
            stocks.push_back(stock);

            double price = std::stod(row[4]);
            prices.push_back(price);

            double quantity = std::stod(row[5]);
            quantities.push_back(quantity);
        }
    }
}

static void TimingDiagnostics()
{
    std::vector<std::string> userIds;
    std::vector<std::string> stocks;
    std::vector<Side> sides;
    std::vector<double> quantities;
    std::vector<double> prices;

    RetrieveData(userIds, stocks, sides, quantities, prices);

    Exchange exchange;

    AccountManager* accountManager = exchange.GetAccountManager();
    MarketManager* marketManager = exchange.GetMarketManager();

    std::unordered_map<std::string, std::size_t> idMap;

    for (std::size_t id = 0; id < userIds.size(); ++id)
    {
        std::size_t nextAvailableId = accountManager->GetNextAvailableId();

        std::string uId = userIds[id];
        
        if (idMap.find(uId) != idMap.end())
            continue;
        
        idMap.insert({ uId, nextAvailableId });
        accountManager->AddAccount(nextAvailableId, *marketManager, 10000, 10000);
    }

    std::size_t simpleStrategyAccountId = accountManager->GetNextAvailableId();

    accountManager->AddAccount(simpleStrategyAccountId, *marketManager, 10000, 10000);
    TradingStrategySimple simpleStrategy(exchange, *accountManager->GetAccount(simpleStrategyAccountId), "AAPL");

    std::vector<std::chrono::nanoseconds> latencies;

    std::size_t OPERATIONS = quantities.size();

    /* Populate users with seed data to get the trading started */
    std::vector<std::string> tickers = {
        "AAPL",
        "META",
        "TSLA",
        "NVDA",
        "MSFT",
        "AMZN",
        "GOOGL",
        "NFLX"
    };

    for (auto& [idStr, idInt] : idMap)
    {
        for (std::string tikr : tickers)
        {
            exchange.AddSeedData(idInt, tikr, 500);
        }
    }

    for (std::string tikr : tickers)
    {
        exchange.AddSeedData(simpleStrategyAccountId, tikr, 500);
    }

    auto startTotal = std::chrono::high_resolution_clock::now();

    simpleStrategy.Run();

    for (std::size_t i = 0; i < OPERATIONS; ++i)
    {
        std::size_t id = idMap.at(userIds[i]);
        auto startOp = std::chrono::high_resolution_clock::now();
        exchange.SendOrderRequest(id, stocks[i], OrderType::LimitOrder, sides[i], quantities[i], prices[i]);
        auto endOp = std::chrono::high_resolution_clock::now();

        latencies.push_back(endOp - startOp);
    }

    auto endTotal = std::chrono::high_resolution_clock::now();

    
    std::unordered_map<std::size_t, Account> accounts = accountManager->GetAccounts();

    std::vector<double> networths;

    for (auto& [id, acc] : accounts)
    {
        networths.push_back(acc.GetNetworth());
    }

    std::sort(networths.begin(), networths.end());
    
    auto& minNetworth = networths.front();
    auto& np1 = networths[networths.size() * 0.01];
    auto& np10 = networths[networths.size() * 0.1];
    auto& np25 = networths[networths.size() * 0.25];
    auto& np50 = networths[networths.size() * 0.50];
    auto& np90 = networths[networths.size() * 0.90];
    auto& np95 = networths[networths.size() * 0.95];
    auto& np99 = networths[networths.size() * 0.99];
    auto& maxNetworth = networths.back();

    auto totalDuration = endTotal - startTotal;
    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(totalDuration).count();

    std::sort(latencies.begin(), latencies.end());

    auto& p50 = latencies[OPERATIONS * 0.50];
    auto& p90 = latencies[OPERATIONS * 0.90];
    auto& p95 = latencies[OPERATIONS * 0.95];
    auto& p99 = latencies[OPERATIONS * 0.99];
    auto& maxLatency = latencies.back();

    std::chrono::nanoseconds totalLatency(0);
    for (const auto& latency : latencies) 
    {
        totalLatency += latency;
    }
    auto meanLatency = totalLatency / OPERATIONS;

    std::cout << "Performance Results:\n";
    std::cout << "-------------------\n";
    std::cout << "Total time: " << totalMs << "ms for " << OPERATIONS << " operations\n";
    PrintNewLine();
    std::cout << "Latency Distribution:\n";
    std::cout << "--------------------\n";
    std::cout << "Mean: " << meanLatency.count() << "ns\n";
    std::cout << "P50:  " << p50.count() << "ns\n";
    std::cout << "P90:  " << p90.count() << "ns\n";
    std::cout << "P95:  " << p95.count() << "ns\n";
    std::cout << "P99:  " << p99.count() << "ns\n";
    std::cout << "Max:  " << maxLatency.count() << "ns\n";
    PrintNewLine();
    std::cout << "User Financials:\n";
    std::cout << "--------------------\n";
    std::cout << "Min:  " << minNetworth - minNetworth << "\n";
    std::cout << "P1:  " << np1 - minNetworth << "\n";
    std::cout << "P10:  " << np10 - minNetworth << "\n";
    std::cout << "P25:  " << np25 - minNetworth << "\n";
    std::cout << "P50:  " << np50 - minNetworth << "\n";
    std::cout << "P90:  " << np90 - minNetworth << "\n";
    std::cout << "P95:  " << np95 - minNetworth << "\n";
    std::cout << "P99:  " << np99 - minNetworth << "\n";
    std::cout << "Max:  " << maxNetworth - minNetworth << "\n";
    std::cout << "Bot: " << accountManager->GetAccount(accountManager->GetNextAvailableId()-1)->GetNetworth() << '\n';
}

static void LaunchApplication()
{
    std::vector<std::string> userIds;
    std::vector<std::string> stocks;
    std::vector<Side> sides;
    std::vector<double> quantities;
    std::vector<double> prices;
    
    RetrieveData(userIds, stocks, sides, quantities, prices);

    Exchange exchange;

    AccountManager* accountManager = exchange.GetAccountManager();
    MarketManager* marketManager = exchange.GetMarketManager();

    std::unordered_map<std::string, std::size_t> idMap;

    std::size_t OPERATIONS = quantities.size();

    for (std::size_t id = 0; id < userIds.size(); ++id)
    {
        idMap[userIds[id]] = id;
        accountManager->AddAccount(id, *marketManager, 10000, 10000);
    }

    std::cout << "Trading has begun!" << '\n';
    PrintNewLine();
    for (std::size_t i = 0; i < OPERATIONS; ++i)
    {
        bool success = exchange.SendOrderRequest(idMap.at(userIds.at(i)), stocks[i], OrderType::LimitOrder, sides[i], quantities[i], prices[i]);

        if (!success)
            continue;

        PrintOrderQuantity(sides[i], quantities[i]);
        PrintSpace();
        std::cout << quantities[i];
        PrintSpace();
        PrintOrderPrice(prices[i]);
        PrintSpace();
        std::cout << idMap.at(userIds.at(i));
        PrintNewLine();
    }
    PrintNewLine();
    std::cout << "Trading has ended!" << '\n';
    PrintNewLine();
}

int main()
{
    TimingDiagnostics();
    //LaunchApplication();
}