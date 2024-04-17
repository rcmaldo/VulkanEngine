#include "first_app.hpp"

#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <cassert>
#include <array>

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
		while (!vulkanengine_window_.ShouldClose())
		{
			glfwPollEvents();

			if (auto command_buffer = vulkanengine_renderer_.BeginFrame())
			{
				vulkanengine_renderer_.BeginSwapChainRenderPass(command_buffer);
				simple_render_system.RenderGameObjects(command_buffer, game_objects_);
				vulkanengine_renderer_.EndSwapChainRenderPass(command_buffer);
				vulkanengine_renderer_.EndFrame();
			}
		}

		vkDeviceWaitIdle(vulkanengine_device_.Device());
	}

	void FirstApp::LoadGameObjects()
	{
		std::vector<VulkanEngineModel::Vertex> vertices {
			{ {0.0f, -0.5f}, { 1.0f, 0.0f, 0.0f } },
			{ {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },
			{ {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }
		};

		// GetSierpinskiTriangle(5, vertices);

		auto vulkanengine_model_ = std::make_shared<VulkanEngineModel>(vulkanengine_device_, vertices);

		auto triangle = VulkanEngineGameObject::CreateGameObject();
		triangle.model_ = vulkanengine_model_;
		triangle.color_ = { .1f, .8f, .1f };
		triangle.transform_2d_.translation.x = .2f;
		triangle.transform_2d_.scale = { 2.f, .5f };
		triangle.transform_2d_.rotation = .25f * glm::two_pi<float>();

		game_objects_.push_back(std::move(triangle));
	}

	void FirstApp::GetSierpinskiTriangle(int num_recursions, std::vector<VulkanEngineModel::Vertex>& vertices)
	{
		if (num_recursions == 0)
		{
			return;
		}

		std::vector<VulkanEngineModel::Vertex> triangle0 = {
			vertices[0],
			{{(vertices[0].position.x + vertices[1].position.x) / 2.0f , (vertices[0].position.y + vertices[1].position.y) / 2.0f}},
			{{(vertices[0].position.x + vertices[2].position.x) / 2.0f , (vertices[0].position.y + vertices[2].position.y) / 2.0f}}
		};

		std::vector<VulkanEngineModel::Vertex> triangle1 = {
			{{(vertices[0].position.x + vertices[1].position.x) / 2.0f , (vertices[0].position.y + vertices[1].position.y) / 2.0f}},
			vertices[1],
			{{(vertices[1].position.x + vertices[2].position.x) / 2.0f , (vertices[1].position.y + vertices[2].position.y) / 2.0f}}
		};

		std::vector<VulkanEngineModel::Vertex> triangle2 = {
			{{(vertices[0].position.x + vertices[2].position.x) / 2.0f , (vertices[0].position.y + vertices[2].position.y) / 2.0f}},
			{{(vertices[1].position.x + vertices[2].position.x) / 2.0f , (vertices[1].position.y + vertices[2].position.y) / 2.0f}},
			vertices[2]
		};

		GetSierpinskiTriangle(num_recursions - 1, triangle0);
		GetSierpinskiTriangle(num_recursions - 1, triangle1);
		GetSierpinskiTriangle(num_recursions - 1, triangle2);

		vertices.clear();
		vertices.reserve(triangle0.size() + triangle1.size() + triangle2.size());
		vertices.insert(vertices.end(), triangle0.begin(), triangle0.end());
		vertices.insert(vertices.end(), triangle1.begin(), triangle1.end());
		vertices.insert(vertices.end(), triangle2.begin(), triangle2.end());
	}

}  // namespace vulkanengine
