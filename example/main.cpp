#include <util/entry.h>
#include <util/geometry.h>
#include <render/renderer.h>
#include <imgui/imgui.h>

class Demo : public vg::Entry
{

public:
	virtual void init() override
	{
		renderer.setup(getInfo().handle);

		auto geometry = vg::SimpleGeometry::createSphere();

		const vg::GeometryBufferInfo info = { 
			{vg::GeometryBufferInfo::Format::rgb32, static_cast<uint32_t>(geometry.position.size()), geometry.position.data()},
			{vg::GeometryBufferInfo::Format::r16, static_cast<uint32_t>(geometry.indices.size()), geometry.indices.data()}
		};
		renderer.createGeometry(info);
	}

	virtual void update() override
	{
		ImGui::NewFrame();

		static bool show_demo_window = true;
		static bool show_another_window = false;
		static float clear_color[3] = { 0.0f };


		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)& clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
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