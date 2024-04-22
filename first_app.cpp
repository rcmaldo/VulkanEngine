#include "first_app.hpp"

#include "Engine/vulkanengine_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace vulkanengine
{
	FirstApp::FirstApp()
	{
		LoadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::Run()
	{
		SimpleRenderSystem simple_render_system{ vulkanengine_device_, vulkanengine_renderer_.GetSwapChainRenderPass() };
		VulkanEngineCamera camera{};

		auto viewer_object = VulkanEngineGameObject::CreateGameObject();
		KeyboardMovementController camera_controller{};

		auto current_time = std::chrono::high_resolution_clock::now();

		while (!vulkanengine_window_.ShouldClose())
		{
			glfwPollEvents();

			auto new_time = std::chrono::high_resolution_clock::now();
			float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
			current_time = new_time;

			camera_controller.MoveInPlaneXZ(vulkanengine_window_.GetGLFWwindow(), frame_time, viewer_object);
			camera.SetViewYXZ(viewer_object.transform_.translation, viewer_object.transform_.rotation);

			float aspect = vulkanengine_renderer_.GetAspectRatio();
			// camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			camera.SetPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.f);

			if (auto command_buffer = vulkanengine_renderer_.BeginFrame())
			{
				vulkanengine_renderer_.BeginSwapChainRenderPass(command_buffer);
				simple_render_system.RenderGameObjects(command_buffer, game_objects_, camera);
				vulkanengine_renderer_.EndSwapChainRenderPass(command_buffer);
				vulkanengine_renderer_.EndFrame();
			}
		}

		vkDeviceWaitIdle(vulkanengine_device_.Device());
	}
	
	void FirstApp::LoadGameObjects()
	{
		std::shared_ptr<VulkanEngineModel> vulkanengine_model = VulkanEngineModel::CreateModelFromFile(vulkanengine_device_, "Models/smooth_vase.obj");

		auto game_object = VulkanEngineGameObject::CreateGameObject();
		game_object.model_ = vulkanengine_model;
		game_object.transform_.translation = { 0.f, 0.f, 2.5f };
		game_object.transform_.scale = glm::vec3(3.f);
		game_objects_.push_back(std::move(game_object));
	}

}  // namespace vulkanengine
