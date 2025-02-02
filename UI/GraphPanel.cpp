#include "GraphPanel.hxx"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <ctime>

wxBEGIN_EVENT_TABLE(GraphPanel, wxPanel)
EVT_PAINT(GraphPanel::OnPaint)
EVT_SIZE(GraphPanel::OnSize)
EVT_BUTTON(wxID_ANY, GraphPanel::OnPlaceOrder)
EVT_MOUSEWHEEL(GraphPanel::OnMouseWheel)
wxEND_EVENT_TABLE()

GraphPanel::GraphPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    SetBackgroundColour(*wxWHITE);

    m_sideCtrl = new wxTextCtrl(this, wxID_ANY, "buy", wxDefaultPosition, wxSize(100, -1));
    m_priceCtrl = new wxTextCtrl(this, wxID_ANY, "100.0", wxDefaultPosition, wxSize(100, -1));
    m_quantityCtrl = new wxTextCtrl(this, wxID_ANY, "10", wxDefaultPosition, wxSize(100, -1));
    m_placeOrderBtn = new wxButton(this, wxID_ANY, "Place Order");

    wxBoxSizer* orderSizer = new wxBoxSizer(wxHORIZONTAL);
    orderSizer->Add(new wxStaticText(this, wxID_ANY, "Side:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    orderSizer->Add(m_sideCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    orderSizer->Add(new wxStaticText(this, wxID_ANY, "Price:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    orderSizer->Add(m_priceCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    orderSizer->Add(new wxStaticText(this, wxID_ANY, "Quantity:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    orderSizer->Add(m_quantityCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    orderSizer->Add(m_placeOrderBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(1, 1, 1, wxEXPAND);
    mainSizer->Add(orderSizer, 0, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
    Refresh();
    Layout();

    StartRealTimeUpdate();
}

GraphPanel::~GraphPanel()
{
    m_runRealTime = false;
    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }
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

void GraphPanel::OnPlaceOrder(wxCommandEvent& event)
{
    wxString sideStr = m_sideCtrl->GetValue();
    wxString priceStr = m_priceCtrl->GetValue();
    wxString qtyStr = m_quantityCtrl->GetValue();

    if (sideStr.IsEmpty() || priceStr.IsEmpty() || qtyStr.IsEmpty())
    {
        wxMessageBox("Please fill in Side, Price, and Quantity", "Input Error");
        return;
    }

    Side side = ConvertSideStringToSide(sideStr.Lower());

    UpdateCurrentCandle(m_currentCandle, wxAtof(priceStr));
    UpdateCandles(m_currentCandle, (++m_ordersInCurrentCandle) >= m_ordersPerCandle);
}

Side GraphPanel::ConvertSideStringToSide(wxString sideString)
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

void GraphPanel::AddCandle(const Candle& candle)
{
    m_candles.push_back(candle);
}

void GraphPanel::UpdateCurrentCandle(const Candle& candle, double price)
{
    if (m_ordersInCurrentCandle == 0)
    {
        m_currentCandle.open = price;
        m_currentCandle.high = price;
        m_currentCandle.low = price;
        m_currentCandle.close = price;
    }
    else
    {
        if (price > m_currentCandle.high)
        {
            m_currentCandle.high = price;
        }

        if (price < m_currentCandle.low)
        {
            m_currentCandle.low = price;
        }

        m_currentCandle.close = price;
    }
}

void GraphPanel::UpdateCandles(const Candle& candle, bool finalize)
{
    m_currentCandle = candle;
    if (finalize)
    {
        AddCandle(m_currentCandle);
        m_ordersInCurrentCandle = 0;
        Refresh();
    }
}

void GraphPanel::SetZoomFactor(double zoom)
{
    m_zoomFactor = zoom;
    Refresh();
}

void GraphPanel::StartRealTimeUpdate()
{
    m_workerThread = std::thread([this]()
        {
            using namespace std::chrono;
            auto interval = milliseconds(50);

            double basePrice = (m_ordersInCurrentCandle > 0) ? m_currentCandle.close : 100.0;

            while (m_runRealTime)
            {
                double fluctuation = (((std::rand() % 1001) - 500) / 1000.0) * 0.2;
                double simulatedPrice = basePrice + fluctuation;
                basePrice = simulatedPrice;

                CallAfter([this, simulatedPrice]()
                {
                    UpdateCurrentCandle(m_currentCandle, simulatedPrice);
                    bool finalize = (++m_ordersInCurrentCandle) >= m_ordersPerCandle;
                    UpdateCandles(m_currentCandle, finalize);
                });

                std::this_thread::sleep_for(interval);
            }
        });
}