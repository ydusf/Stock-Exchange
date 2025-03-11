#pragma once

#include <wx/wx.h>
#include <wx/msgqueue.h>

#include "GraphPanel.hxx"
#include "Core/Exchange.hxx"
#include "Websockets/TradingStrategy.hxx"

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>

#include <vector>
#include <queue>


wxDEFINE_EVENT(MarketUpdateEvt, wxThreadEvent);

class MainFrame : public wxFrame
{
private:
    std::unique_ptr<GraphPanel> m_graphPanel;
    wxStatusBar* m_statusBar;
    wxListBox* m_ordersList;

    // Exchange & simulation
    Exchange* m_exchange;
    std::thread m_ordersThread;
    std::thread m_updateUIThread;
    std::atomic<bool> m_running{ true };

public:
    MainFrame(Exchange& exchange);

    ~MainFrame();

private:
    void ObserveStocks();
    std::vector<std::size_t> AddDummyAccounts(std::size_t numAccounts);
    void BeginSimulation(std::vector<std::size_t>& accounts);
    Side ConvertSideStringToSide(wxString sideString);

private:
    void OnMarketUpdate(wxThreadEvent& evt);
    void OnClose(wxCloseEvent& evt);
};
