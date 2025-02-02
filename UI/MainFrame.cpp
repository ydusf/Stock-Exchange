#include "MainFrame.hxx"

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Stock Exchange Simulation", wxDefaultPosition, wxSize(800, 600))
{
    GraphPanel* graphPanel = new GraphPanel(this);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(graphPanel, 1, wxEXPAND);
    SetSizer(sizer);
    Layout();
}
