#pragma once


#include "Engine/vulkanengine_game_object.hpp"
#include "Engine/vulkanengine_window.hpp"

namespace vulkanengine
{
	class KeyboardMovementController
	{
	public:
		struct KeyMappings
		{
            int move_left = GLFW_KEY_A;
            int move_right = GLFW_KEY_D;
            int move_forward = GLFW_KEY_W;
            int move_backward = GLFW_KEY_S;
            int move_up = GLFW_KEY_E;
            int move_down = GLFW_KEY_Q;
            int look_left = GLFW_KEY_LEFT;
            int look_right = GLFW_KEY_RIGHT;
            int look_up = GLFW_KEY_UP;
            int look_down = GLFW_KEY_DOWN;
		};

        void MoveInPlaneXZ(GLFWwindow* window, float dt, VulkanEngineGameObject& game_object);

        KeyMappings keys_{};
        float move_speed_{ 3.f };
        float look_speed_{ 1.5f };
	};
}