#include <vulkan/vulkan.h>
#include <iostream>

int main() { 
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Refractor";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "Refractor Engine";


    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;

    VkInstance vkInst;

    VkResult result = vkCreateInstance(&instInfo, 0, &vkInst);
    if (result == VK_SUCCESS)
    {
        std::cout << "Vulkan instance created!" << std::endl;
    }

    return 0;
}