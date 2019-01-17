#include <Windows.h>
#include <util/entry.h>

int CALLBACK WinMain(HINSTANCE,HINSTANCE,LPSTR lpCmdLine, int nCmdShow)
{
    vg::Entry entry;
    entry.setup();

    OutputDebugString("exit");

    return 0;
}