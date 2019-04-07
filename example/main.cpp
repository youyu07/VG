#include <util/entry.h>
#include <util/geometry.h>
#include <render/renderer.h>
#include <imgui/imgui.h>
#include <glm/ext.hpp>
#include <core/camera.h>

class Demo : public vg::Entry
{
	vg::Camera camera = vg::Camera::Perspactive(45.0f);
public:
	virtual void init() override
	{
		renderer.setup(getInfo().handle);
		camera.translate(glm::vec3(0, 0, -10));
		camera.rotate(glm::vec3(45, -45, 0.0f));

		auto geometry = vg::SimpleGeometry::createSphere();
		auto info = vg::GeometryBufferInfo();
		info.vertexData(uint32_t(geometry.vertex.size() * sizeof(vg::SimpleGeometry::Vertex)), geometry.vertex.data(),vg::VertexType::PNT);
		info.indexData(uint32_t(geometry.indices.size() * sizeof(uint16_t)), geometry.indices.data());
		renderer.addGeometry(0, info);
	}

	virtual void update() override
	{
		ImGui::NewFrame();

		static bool show_demo_window = false;
		static bool show_another_window = false;
		static float clear_color[3] = { 0.0f };

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "CTRL+O")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}


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

		renderer.bindCamera(camera);
	}

	virtual void mouseEvent(const MouseEvent& event) override
	{
		static bool mouseDown[3];
		static float x, y, initX, initY;

		float dx = event.x - x;
		float dy = event.y - y;

		x = event.x;
		y = event.y;

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			return;
		}

		switch (event.type)
		{
		case MouseEvent::Type::Wheel:
			camera.translate(glm::vec3(0, 0, event.y));
			break;
		case MouseEvent::Type::LeftDown:
			mouseDown[0] = true;
			initX = x; initY = y;
			break;
		case MouseEvent::Type::RightDown:
			mouseDown[1] = true;
			break;
		case MouseEvent::Type::MiddleDown:
			mouseDown[2] = true;
			break;
		case MouseEvent::Type::LeftUp:
			mouseDown[0] = false;
			if (x - initX == 0.0f && y - initY == 0.0f) {
				renderer.click({ x,y });
			}
			break;
		case MouseEvent::Type::RightUp:
			mouseDown[1] = false;
			break;
		case MouseEvent::Type::MiddleUp:
			mouseDown[2] = false;
			break;
		case MouseEvent::Type::Move:
			{
				if (mouseDown[0] && getKeyState(vg::Key::Alt)) {
					camera.rotate(glm::vec3(dy, dx, 0.0f) * camera.getRotateSpeed());
				}

				if (mouseDown[2]) {
					camera.translate(glm::vec3(dx * 0.01f, -dy * 0.01f, 0.0f));
				}
				break;
			}
		}
	}

	virtual void windowEvent(const WindowEvent& event) override
	{
		if (event.type == WindowEvent::Type::Restored || event.type == WindowEvent::Type::Maximized) {
			renderer.resize();
		}
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