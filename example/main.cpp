#include <util/entry.h>
#include "render/renderer.h"

class Demo : public vg::Entry
{

public:
	virtual void init() override
	{
		renderer.setup(getInfo().handle);
	}

	virtual void update() override
	{

	}

	virtual void draw() override
	{
		renderer.draw();
	}

private:
	vg::Renderer renderer;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	Demo demo;
	demo.start();

    return 0;
}