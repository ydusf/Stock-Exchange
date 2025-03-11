#include "GraphPanel.hxx"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <ctime>

GraphPanel::GraphPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    SetBackgroundColour(*wxWHITE);

    Bind(wxEVT_PAINT, &GraphPanel::OnPaint, this, wxID_ANY);
    Bind(wxEVT_SIZE, &GraphPanel::OnSize, this, wxID_ANY);
    Bind(wxEVT_MOUSEWHEEL, &GraphPanel::OnMouseWheel, this, wxID_ANY);
}

GraphPanel::~GraphPanel()
{
}

void GraphPanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    GetClientSize(&m_width, &m_height);

    DrawGraph(dc, m_width, m_height - 60);
}

void GraphPanel::OnSize(wxSizeEvent& event)
{
    Refresh();
    event.Skip();
}

void GraphPanel::OnMouseWheel(wxMouseEvent& event)
{
    int rotation = event.GetWheelRotation();
    const double zoomStep = 0.05;
    if (rotation > 0)
    {
        m_zoomFactor += zoomStep;
    }
    else if (rotation < 0 && m_zoomFactor > zoomStep)
    {
        m_zoomFactor -= zoomStep;
    }

    m_zoomFactor = std::max(m_zoomFactor, 0.05);
    m_zoomFactor = std::min(m_zoomFactor, 10.0);

    Refresh();
}

void GraphPanel::DrawGraph(wxDC& dc, int width, int height)
{
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    const int marginLeft = 50;
    const int marginRight = 50;
    const int marginTop = 20;
    const int marginBottom = 20;
    const int constantMargin = 20;

    int graphWidth = width - marginLeft - marginRight;
    int graphHeight = height - marginTop - marginBottom;

    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRectangle(marginLeft, marginTop, graphWidth, graphHeight);

    dc.SetClippingRegion(marginLeft, marginTop, graphWidth, graphHeight);

    if (m_candles.empty())
        return;

    double fullMinPrice = m_candles.front().low;
    double fullMaxPrice = m_candles.front().high;

    for (const auto& c : m_candles)
    {
        fullMinPrice = std::min(fullMinPrice, c.low);
        fullMaxPrice = std::max(fullMaxPrice, c.high);
    }

    double fullRange = fullMaxPrice - fullMinPrice;
    double midPrice = (fullMaxPrice + fullMinPrice) / 2.0;
    double visibleRange = fullRange / m_zoomFactor;
    double visibleMinPrice = midPrice - visibleRange / 2.0;
    double visibleMaxPrice = midPrice + visibleRange / 2.0;

    if (visibleMaxPrice == visibleMinPrice)
    {
        visibleMaxPrice += 1.0;
    }

    int candleCount = m_candles.size();
    double candleSpacing = (double)(graphWidth) / candleCount;
    double bodyFraction = 0.6;

    for (int i = 0; i < candleCount; ++i)
    {
        const Candle& candle = m_candles[i];
        int xCenter = marginLeft + (int)((i + 0.5) * candleSpacing);
        int bodyWidth = (int)(candleSpacing * bodyFraction);
        int halfBody = bodyWidth / 2;

        auto priceToY = [=](double price) -> int
            {
                double norm = (price - visibleMinPrice) / (visibleMaxPrice - visibleMinPrice);
                return marginTop + graphHeight - (int)(norm * graphHeight);
            };

        int yHigh = priceToY(candle.high);
        int yLow = priceToY(candle.low);
        int yOpen = priceToY(candle.open);
        int yClose = priceToY(candle.close);


        wxColor colour;
        colour = candle.close >= candle.open ? *wxGREEN : *wxRED;

        wxPen extremesPen;
        extremesPen.SetColour(colour);
        dc.SetPen(extremesPen);
        dc.DrawLine(xCenter, yHigh, xCenter, yLow);

        int topBody = std::min(yOpen, yClose);
        int bottomBody = std::max(yOpen, yClose);
        wxRect bodyRect(xCenter - halfBody, topBody, bodyWidth, bottomBody - topBody);

        wxBrush bodyBrush;
        bodyBrush.SetColour(colour);

        bodyBrush.SetStyle(wxBRUSHSTYLE_SOLID);
        dc.SetBrush(bodyBrush);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(bodyRect);
    }

    dc.DestroyClippingRegion();

    dc.SetPen(*wxBLACK_PEN);
    const int tickCount = 5;

    for (int i = 0; i < tickCount; ++i)
    {
        double t = (double)(i) / (tickCount - 1);
        int x = marginLeft + (int)(t * graphWidth);
        int yStart = marginTop + graphHeight;
        int yEnd = yStart + 5;
        dc.DrawLine(x, yStart, x, yEnd);

        std::ostringstream oss;
        oss << "T" << i;
        wxString label(oss.str());
        wxSize textSize = dc.GetTextExtent(label);
        dc.DrawText(label, x - textSize.GetWidth() / 2, yEnd + 2);
    }

    for (int i = 0; i < tickCount; ++i)
    {
        double t = (double)(i) / (tickCount - 1);
        double normalized = 1.0 - t;
        int y = marginTop + graphHeight - (int)(normalized * graphHeight);
        int xStart = marginLeft + graphWidth;
        int xEnd = xStart + 5;
        dc.DrawLine(xStart, y, xEnd, y);

        double priceLabel = visibleMinPrice + normalized * (visibleMaxPrice - visibleMinPrice);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << priceLabel;
        wxString label(oss.str());
        wxSize textSize = dc.GetTextExtent(label);
        dc.DrawText(label, xEnd + 2, y - textSize.GetHeight() / 2);
    }
}

void GraphPanel::UpdateCurrentCandle(double price)
{
    Candle& candle = m_candles.back();

    if (price > candle.high)
    {
        candle.high = price;
    }

    if (price < candle.low)
    {
        candle.low = price;
    }

    candle.close = price;
}

void GraphPanel::UpdateCandles(double price)
{
    auto currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_currentCandleTime).count() >= 1)
    {
        m_currentCandleTime = currentTime;
        m_candles.emplace_back(price, price, price, price);
    }

    UpdateCurrentCandle(price);

    Refresh();
}

void GraphPanel::SetZoomFactor(double zoom)
{
    m_zoomFactor = zoom;
    Refresh();
}