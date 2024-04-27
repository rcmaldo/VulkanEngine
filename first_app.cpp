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
		glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .02f}; // w is light intensity
		glm::vec3 light_position{-1.f};
		alignas(16) glm::vec4 light_color{1.f}; // w is light intensity
	};

	FirstApp::FirstApp()
	{
		global_pool_ = VulkanEngineDescriptorPool::Builder(vulkanengine_device_)
			.SetMaxSets(VulkanEngineSwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanEngineSwapChain::MAX_FRAMES_IN_FLIGHT)
			.Build();
		LoadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::Run()
	{
		std::vector<std::unique_ptr<VulkanEngineBuffer>> ubo_buffers(VulkanEngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < ubo_buffers.size(); ++i)
		{
			ubo_buffers[i] = std::make_unique<VulkanEngineBuffer>(
				vulkanengine_device_,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			ubo_buffers[i]->Map();
		}

		auto global_set_layout = VulkanEngineDescriptorSetLayout::Builder(vulkanengine_device_)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build();

		std::vector<VkDescriptorSet> global_descriptor_sets(VulkanEngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < global_descriptor_sets.size(); ++i)
		{
			auto buffer_info = ubo_buffers[i]->DescriptorInfo();
			VulkanEngineDescriptorWriter(*global_set_layout, *global_pool_)
				.WriteBuffer(0, &buffer_info)
				.Build(global_descriptor_sets[i]);
		}

		SimpleRenderSystem simple_render_system{
			vulkanengine_device_,
			vulkanengine_renderer_.GetSwapChainRenderPass(),
			global_set_layout->GetDescriptorSetLayout() };
		VulkanEngineCamera camera{};

		auto viewer_object = VulkanEngineGameObject::CreateGameObject();
		viewer_object.transform_.translation.z = -2.5f; // initial position
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
			camera.SetPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);

			if (auto command_buffer = vulkanengine_renderer_.BeginFrame())
			{
				int frame_index = vulkanengine_renderer_.GetFrameIndex();
				FrameInfo frame_info{
					frame_index,
					frame_time,
					command_buffer,
					camera,
					global_descriptor_sets[frame_index]
				};

				// update
				GlobalUbo ubo{};
				ubo.projection_view = camera.GetProjection() * camera.GetView();
				ubo_buffers[frame_index]->WriteToBuffer(&ubo);
				ubo_buffers[frame_index]->Flush();

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
		flat_vase.transform_.translation = { -.5f, .5f, 0.f };
		flat_vase.transform_.scale = { 3.f, 1.5f, 3.f };
		game_objects_.push_back(std::move(flat_vase));

		vulkanengine_model = VulkanEngineModel::CreateModelFromFile(vulkanengine_device_, "Models/smooth_vase.obj");
		auto smooth_vase = VulkanEngineGameObject::CreateGameObject();
		smooth_vase.model_ = vulkanengine_model;
		smooth_vase.transform_.translation = { .5f, .5f, 0.f };
		smooth_vase.transform_.scale = { 3.f, 1.5f, 3.f };
		game_objects_.push_back(std::move(smooth_vase));

		vulkanengine_model = VulkanEngineModel::CreateModelFromFile(vulkanengine_device_, "Models/quad.obj");
		auto floor = VulkanEngineGameObject::CreateGameObject();
		floor.model_ = vulkanengine_model;
		floor.transform_.translation = { 0.f, .5f, 0.f };
		floor.transform_.scale = { 3.f, 1.5f, 3.f };
		game_objects_.push_back(std::move(floor));
	}

}  // namespace vulkanengine
