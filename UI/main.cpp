#include "App.hxx"

int main()
{
    App app;
    app.OnInit();

    int exitCode = app.MainLoop();

    return exitCode;
}