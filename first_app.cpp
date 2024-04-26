#include "first_app.hpp"

#include "Engine/vulkanengine_buffer.hpp"
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
	// Global Uniform Buffer Object
	struct GlobalUbo
	{
		glm::mat4 projection_view{1.f};
		glm::vec3 light_direction = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
	};

	FirstApp::FirstApp()
	{
		LoadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::Run()
	{
		VulkanEngineBuffer global_ubo_buffer{
			vulkanengine_device_,
			sizeof(GlobalUbo),
			VulkanEngineSwapChain::MAX_FRAMES_IN_FLIGHT,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			vulkanengine_device_.properties_.limits.minUniformBufferOffsetAlignment
		};
		global_ubo_buffer.Map();

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
				int frame_index = vulkanengine_renderer_.GetFrameIndex();
				FrameInfo frame_info{
					frame_index,
					frame_time,
					command_buffer,
					camera
				};

				// update
				GlobalUbo ubo{};
				ubo.projection_view = camera.GetProjection() * camera.GetView();
				global_ubo_buffer.WriteToIndex(&ubo, frame_index);
				global_ubo_buffer.FlushIndex(frame_index);

				// render
				vulkanengine_renderer_.BeginSwapChainRenderPass(command_buffer);
				simple_render_system.RenderGameObjects(frame_info, game_objects_);
				vulkanengine_renderer_.EndSwapChainRenderPass(command_buffer);
				vulkanengine_renderer_.EndFrame();
			}
		}

		vkDeviceWaitIdle(vulkanengine_device_.Device());
	}
	
	void FirstApp::LoadGameObjects()
	{
		std::shared_ptr<VulkanEngineModel> vulkanengine_model = VulkanEngineModel::CreateModelFromFile(vulkanengine_device_, "Models/flat_vase.obj");
		auto flat_vase = VulkanEngineGameObject::CreateGameObject();
		flat_vase.model_ = vulkanengine_model;
		flat_vase.transform_.translation = { -.5f, .5f, 2.5f };
		flat_vase.transform_.scale = { 3.f, 1.5f, 3.f };
		game_objects_.push_back(std::move(flat_vase));

		vulkanengine_model = VulkanEngineModel::CreateModelFromFile(vulkanengine_device_, "Models/smooth_vase.obj");
		auto smooth_vase = VulkanEngineGameObject::CreateGameObject();
		smooth_vase.model_ = vulkanengine_model;
		smooth_vase.transform_.translation = { .5f, .5f, 2.5f };
		smooth_vase.transform_.scale = { 3.f, 1.5f, 3.f };
		game_objects_.push_back(std::move(smooth_vase));
	}

}  // namespace vulkanengine
