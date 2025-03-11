#include "MainFrame.hxx"

MainFrame::MainFrame(Exchange& exchange)
    : wxFrame(nullptr, wxID_ANY, "Stock Exchange Simulation", wxDefaultPosition, wxSize(800, 600)),
    m_exchange(&exchange)
{
    // UI RELATED LOGIC
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    m_graphPanel = std::make_unique<GraphPanel>(this);
    mainSizer->Add(m_graphPanel.get(), 1, wxEXPAND);

    m_statusBar = CreateStatusBar();
    m_statusBar->SetStatusText("Stock Price: ");

    m_ordersList = new wxListBox(this, wxID_ANY);
    mainSizer->Add(m_ordersList, 0, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
    Layout();

    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this, wxID_CLOSE);
    Bind(MarketUpdateEvt, &MainFrame::OnMarketUpdate, this);

    // MARKET SIMULATION LOGIC
    std::vector<std::size_t> accounts = AddDummyAccounts(100);
    ObserveStocks();
    BeginSimulation(accounts);
}

MainFrame::~MainFrame()
{
    m_running = false;
    if (m_ordersThread.joinable())
    {
        m_ordersThread.join();
    }

    if (m_updateUIThread.joinable())
    {
        m_updateUIThread.join();
    }

    Destroy();
}

void MainFrame::ObserveStocks()
{
    MarketManager* marketManager = m_exchange->GetMarketManager();
    assert(marketManager);

    std::function<void(MarketQuote quote)> callback = [this](MarketQuote quote)
        {
            wxThreadEvent* evt = new wxThreadEvent(MarketUpdateEvt);
            evt->SetString(
                "Stock Price: " + std::to_string(quote.m_lastPrice) +
                "; Top Bid: " + std::to_string(quote.m_topBid) +
                "; Top Ask: " + std::to_string(quote.m_topAsk) +
                "; Volume: " + std::to_string(quote.m_volume)
            );
            evt->SetPayload(quote);
            wxQueueEvent(this, evt);
        };
    marketManager->AddObserver("NVDA", callback);
}

std::vector<std::size_t> MainFrame::AddDummyAccounts(std::size_t numAccounts)
{
    AccountManager* accountManager = m_exchange->GetAccountManager();
    assert(accountManager);
    MarketManager* marketManager = m_exchange->GetMarketManager();
    assert(marketManager);

    std::vector<std::size_t> accounts;
    for (int i = 0; i < numAccounts; ++i)
    {
        std::size_t accountId = accountManager->GetNextAvailableId();
        accountManager->AddAccount(accountId, *marketManager, 1'000'000 + (std::rand() % 5'000'000));
        m_exchange->AddSeedData(accountId, "NVDA", 100'000);
        accounts.push_back(accountId);
    }

    return accounts;
}

void MainFrame::BeginSimulation(std::vector<std::size_t>& accounts)
{
    m_ordersThread = std::thread([this, accounts]()
        {
            using namespace std::chrono;

            double midPrice = 110.0;
            const double riskFreeRate = 0.0001;
            double volatility = 0.5;

            const double dt = 1.0 / 60.0;

            std::random_device rd;
            std::mt19937 gen(rd());

            std::normal_distribution<double> volShock(0.0, 0.05);
            std::lognormal_distribution<double> sizeDistribution(5.0, 0.8);
            std::normal_distribution<double> priceOffsetDist(0.0, 0.5);

            const double baseDelayMs = 50.0;
            std::exponential_distribution<double> delayDistribution(1.0 / baseDelayMs);
            std::uniform_int_distribution<int> sideDistribution(0, 1);

            while (m_running.load())
            {
                volatility += volShock(gen);
                if (volatility < 0.1)
                {
                    volatility = 0.1;
                }

                std::normal_distribution<double> norm(0.0, 1.0);
                double Z = norm(gen);
                midPrice *= std::exp((riskFreeRate - 0.5 * volatility * volatility) * dt + volatility * std::sqrt(dt) * Z);

                if (midPrice < 1.0)
                {
                    midPrice = 1.0;
                }

                for (const auto& account : accounts)
                {
                    Side side = (sideDistribution(gen) == 0 ? Side::Buy : Side::Sell);

                    int size = static_cast<int>(sizeDistribution(gen));
                    if (size <= 0)
                    {
                        size = 1;
                    }

                    double priceNoise = priceOffsetDist(gen);
                    int price = static_cast<int>(midPrice + priceNoise);
                    if (price <= 0)
                    {
                        price = 1;
                    }

                    OrderType orderType = sideDistribution(gen) > 0.5 ? OrderType::LimitOrder : OrderType::MarketOrder;
                    m_exchange->SendOrderRequest(account, "NVDA", orderType, side, size, price);

                    auto delay = microseconds(static_cast<int>(delayDistribution(gen)));
                    std::this_thread::sleep_for(delay);
                }
            }
        });
}

Side MainFrame::ConvertSideStringToSide(wxString sideString)
{
    if (sideString == "buy")
    {
        return Side::Buy;
    }
    else if (sideString == "sell")
    {
        return Side::Sell;
    }
    else
    {
        wxMessageBox("Invalid Side. Enter 'buy' or 'sell'", "Input Error");
        return Side::Null;
    }
}

void MainFrame::OnMarketUpdate(wxThreadEvent& evt)
{
    m_statusBar->SetStatusText(evt.GetString());

    MarketQuote quote = evt.GetPayload<MarketQuote>();
    m_ordersList->Append("New Price: " + std::to_string(quote.m_lastPrice));
    m_graphPanel->UpdateCandles(quote.m_lastPrice);
}

void MainFrame::OnClose(wxCloseEvent& evt)
{
    evt.Skip();
    Close(true);
}