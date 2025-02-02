#pragma once

#include <wx/wx.h>

#include <vector>
#include <memory>
#include <thread>
#include <atomic>

enum class Side { Buy, Sell, Null };

struct Candle
{
    double open;
    double high;
    double low;
    double close;
};

class GraphPanel : public wxPanel
{
private:
    int m_ordersInCurrentCandle = 0;
    const int m_ordersPerCandle = 5;
    Candle m_currentCandle = Candle();
    std::vector<Candle> m_candles;

    wxTextCtrl* m_sideCtrl = nullptr;
    wxTextCtrl* m_priceCtrl = nullptr;
    wxTextCtrl* m_quantityCtrl = nullptr;
    wxButton* m_placeOrderBtn = nullptr;

    int m_width, m_height;
    double m_zoomFactor = 1.0;

    std::atomic<bool> m_runRealTime = { true };
    std::thread m_workerThread;

public:
    GraphPanel(wxWindow* parent);

    Side ConvertSideStringToSide(wxString sideString);
    void AddCandle(const Candle& candle);
    void UpdateCurrentCandle(const Candle& candle, double price);
    void UpdateCandles(const Candle& candle, bool finalize);
    void SetZoomFactor(double zoom);
    void StartRealTimeUpdate();

    ~GraphPanel();

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    void OnPlaceOrder(wxCommandEvent& event);

    void DrawGraph(wxDC& dc, int width, int height);

    wxDECLARE_EVENT_TABLE();
};
