#include <Windows.h>
#include <util/entry.h>


class Demo : public vg::Entry
{

public:
	virtual void init() override
	{

	}

	virtual void update() override
	{

	}

	virtual void draw() override
	{

	}

};


int CALLBACK WinMain(HINSTANCE,HINSTANCE,LPSTR lpCmdLine, int nCmdShow)
{
	Demo demo;
	demo.start();

    return 0;
}