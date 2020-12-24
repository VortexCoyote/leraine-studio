#include "program/program.h"

int main()
{
    Program program;

    program.Init();
    program.StartUp();

    while (program.HandleEvents())
    {
        program.Tick();
        program.Render();
    }

    program.ShutDown();
}