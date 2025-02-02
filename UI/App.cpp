#include "App.hxx"
#include "MainFrame.hxx"

bool App::OnInit()
{
    MainFrame* mainFrame = new MainFrame();
    mainFrame->Show(true);
    return true;
}

wxIMPLEMENT_APP(App);
