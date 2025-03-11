#pragma once

#include <wx/wx.h>

#include <vector>
#include <memory>
#include <chrono>

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
    std::chrono::time_point<std::chrono::steady_clock> m_currentCandleTime;
    std::vector<Candle> m_candles;

    int m_width = 800, m_height = 600;
    double m_zoomFactor = 1.0;

public:
    GraphPanel(wxWindow* parent);

    void UpdateCurrentCandle(double price);
    void UpdateCandles(double price);
    void SetZoomFactor(double zoom);

    ~GraphPanel();

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    void DrawGraph(wxDC& dc, int width, int height);
};
