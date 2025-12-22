#include "Core/Application.h"

int main()
{
    auto* app = new Echo::Application();

    app->Run();
    
    return 0;
}