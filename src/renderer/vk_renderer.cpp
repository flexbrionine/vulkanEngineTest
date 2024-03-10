#include <vulkan/vulkan.h>

struct VkContext
{
    VkInstance vkInst;
};


bool vk_init(VkContext* vkContext) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Refractor";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "Refractor Engine";


    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;



    VkResult result = vkCreateInstance(&instInfo, 0, &vkContext->vkInst);

    if (result == !VK_SUCCESS) {
        return false;
    }

    return true;
}
