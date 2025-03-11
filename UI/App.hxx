#pragma once

#include <wx/wx.h>

#include "Core/Exchange.hxx"
#include "MainFrame.hxx"

class App : public wxApp
{
private:
    Exchange m_exchange;
    std::thread m_exchangeServer;

public:
    virtual bool OnInit() override;

    ~App();
};
