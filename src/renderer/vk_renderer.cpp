#include <vulkan/vulkan.h>
#include <windows.h>
#include <vector>
#include <iostream>
#ifdef WINDOWS_BUILD
#include <vulkan/vulkan_win32.h>
#elif LINUX_BUILD
#endif


#define VK_CHECK(result)                \
if (result != VK_SUCCESS) {             \
    std::cout << result << std::endl;   \
    __debugbreak();                     \
    return false;                       \
}

#define ArraySize(arr) sizeof((arr)) / sizeof((arr[0]))

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgFlags,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void *pUserData
) {
    std::cout << "Validation Error: " << pCallbackData->pMessage << std::endl;
    return false;
}

struct VkContext {
    VkInstance vkInst;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSwapchainKHR swapchain;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;

    uint32_t scImgCount;
    VkImage scImages[5];
    // TODO: change aray to vecto=>array

    int graphicsIndex;
};

bool vk_init(VkContext* vkContext, void* window) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Refractor";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "Refractor Engine";

    char* extensions[] = {
#ifdef WINDOWS_BUILD
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif LINUX_BUILD
#endif

        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    char* layers[] {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.ppEnabledExtensionNames = extensions;
    instInfo.enabledExtensionCount = ArraySize(extensions);
    instInfo.ppEnabledLayerNames = layers;
    instInfo.enabledLayerCount = ArraySize(layers);

    VK_CHECK(vkCreateInstance(&instInfo, 0, &vkContext->vkInst)); 

    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkContext->vkInst, "vkCreateDebugUtilsMessengerEXT");

    if (vkCreateDebugUtilsMessengerEXT) {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = vk_debug_callback;

        vkCreateDebugUtilsMessengerEXT(vkContext->vkInst, &debugInfo, 0, &vkContext->debugMessenger);
    } else { return false; }
    

    //* Creating Surface
#ifdef WINDOWS_BUILD
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd = (HWND)window;
    surfaceInfo.hinstance = GetModuleHandleA(0);

    VK_CHECK(vkCreateWin32SurfaceKHR(vkContext->vkInst, &surfaceInfo, 0, &vkContext->surface));
#elif LINUX_BUILD
#endif

    //* Select GPU
    {
        vkContext->graphicsIndex = -1;
        uint32_t gpuCount = 0;
        VkPhysicalDevice gpus[10];
        
            VK_CHECK(vkEnumeratePhysicalDevices(vkContext->vkInst, &gpuCount, 0));
            VK_CHECK(vkEnumeratePhysicalDevices(vkContext->vkInst, &gpuCount, gpus));
        for (uint32_t i = 0; i < gpuCount; i++) {
            VkPhysicalDevice gpu = gpus[i];

            uint32_t queueFamilyCount = 0;
            VkQueueFamilyProperties queueProps[10];
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, 0);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps);

            for (uint32_t j = 0; j < queueFamilyCount; j++) {
                if (queueProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    VkBool32 surfaceSupport = false;
                    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, vkContext->surface, &surfaceSupport));
                    
                    if (surfaceSupport) {
                        vkContext->graphicsIndex = j;
                        vkContext->gpu = gpu;
                        break;
                    }
                }
            }   
        }
        if (vkContext->graphicsIndex < 0) {
            return false;
        }
        
    }

    //* Creating Logical Devise
    {
        float queuePriority = 1.0f;
        
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = vkContext->graphicsIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        char* extensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.ppEnabledExtensionNames = extensions;
        deviceInfo.enabledExtensionCount = ArraySize(extensions);

        VK_CHECK(vkCreateDevice(vkContext->gpu, &deviceInfo, 0, &vkContext->device));

        vkGetDeviceQueue(vkContext->device, vkContext->graphicsIndex, 0, &vkContext->graphicsQueue);
    }

    //* Creating Swap Chain
    {
        uint32_t formatCount = 0;
        VkSurfaceFormatKHR surfaceFormats[10];
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkContext->gpu, vkContext->surface, &formatCount, 0));
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkContext->gpu, vkContext->surface, &formatCount, surfaceFormats));

        for (uint32_t i = 0; i < formatCount; i++) {
            VkSurfaceFormatKHR format = surfaceFormats[i];
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB) {
                vkContext->surfaceFormat = format;
                break;
            }
            
        }
        
        
        VkSurfaceCapabilitiesKHR surfaceCaps = {};
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkContext->gpu, vkContext->surface, &surfaceCaps));
        uint32_t imgCount = surfaceCaps.minImageCount + 1;
        imgCount = imgCount > surfaceCaps.maxImageCount ? imgCount - 1 : imgCount;
        
        VkSwapchainCreateInfoKHR scInfo = {};
        scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        scInfo.surface = vkContext->surface;
        scInfo.imageFormat = vkContext->surfaceFormat.format;
        scInfo.preTransform = surfaceCaps.currentTransform;

        surfaceCaps.currentExtent.height=600;
        surfaceCaps.currentExtent.width=800;

        scInfo.imageExtent = surfaceCaps.currentExtent;
        scInfo.minImageCount = imgCount;
        scInfo.imageArrayLayers = 1;
        
        VK_CHECK(vkCreateSwapchainKHR(vkContext->device, &scInfo, 0, &vkContext->swapchain));

        VK_CHECK(vkGetSwapchainImagesKHR(vkContext->device, vkContext->swapchain, &vkContext->scImgCount, 0));   
        VK_CHECK(vkGetSwapchainImagesKHR(vkContext->device, vkContext->swapchain, &vkContext->scImgCount, vkContext->scImages));

    }

    //* Command Pool
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = vkContext->graphicsIndex;
        VK_CHECK(vkCreateCommandPool(vkContext->device, &poolInfo, 0, &vkContext->commandPool));
    }


    return true;
}

bool ck_renderer(VkContext* vkContext){
    uint32_t imgIdx;
    VK_CHECK(vkAcquireNextImageKHR(vkContext->device, vkContext->swapchain, 0,0,0, &imgIdx));

    //* Rendering Commands

    VkCommandBuffer cmd;

    VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = vkContext->commandPool;
    VK_CHECK(vkAllocateCommandBuffers(vkContext->device, &allocInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo subminInfo = {};
        subminInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        subminInfo.commandBufferCount = 1;
        subminInfo.pCommandBuffers = &cmd;
    VK_CHECK(vkQueueSubmit(vkContext->graphicsQueue, 1, &subminInfo, 0));

    VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = &vkContext->swapchain;
        presentInfo.swapchainCount = 1;
        presentInfo.pImageIndices = &imgIdx;
    VK_CHECK(vkQueuePresentKHR(vkContext->graphicsQueue, &presentInfo));

    VK_CHECK(vkDeviceWaitIdle(vkContext->device));

    vkFreeCommandBuffers(vkContext->device, vkContext->commandPool, 1, &cmd);
}