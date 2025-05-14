#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <set>
#include <vector>
#include <optional>

// function to load vkCreateDebugUtilsMessengerEXT
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr){
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, 
                                       const VkAllocationCallbacks* pAllocator){
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr){
    func(instance, debugMessenger, pAllocator);
  }
}


class HelloTriangleApplication {
private:
  const uint32_t WIDTH = 800;
  const uint32_t HEIGHT = 600;

  std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
  };

  #ifdef NDEBUG
    const bool enableValidationLayers = false;
  #else
    const bool enableValidationLayers = true;
  #endif

  GLFWwindow *window;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device;
  VkQueue graphicsQueue;

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
  }

  VkSurfaceKHR surface;
  VkQueue presentQueue;

  struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete(){
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
  }

  void setupDebugMessenger() {
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS){
      throw std::runtime_error("Failed to set up Debug Messenger!");
    }
  }

  std::vector<const char*> getRequiredExtensions(){
    // Checking Extension Support
    uint32_t glfwExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionsList(glfwExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, extensionsList.data());

    std::cout << "available extensions" << std::endl;
    for(const auto& ext : extensionsList){
      std::cout << '\t' <<ext.extensionName << std::endl;
    }

    const char** glfwExtensions;


    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if(enableValidationLayers){
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
  }

  void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  }

  void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
  }

  void createSurface(){
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
      throw std::runtime_error("Failed to create window surface!");
    }
  }
  
  void createInstance(){
    
    if(enableValidationLayers && !checkValidationSupport()) {
      throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;


    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;


    std::vector<const char*> requiredExtensions = getRequiredExtensions();


    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    requiredExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    // attach debug instance to the pNext pointer to inspect instance creation and destruction
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers && checkValidationSupport()){
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS){
      throw std::runtime_error("failed to create VkInstance");
    }

  }

  bool checkValidationSupport() {
    // check if the validationLayers are supported
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* layerName : validationLayers) {
      bool layerFound = false;

      for(const auto& LayerProperties : availableLayers) {
        if (strcmp(layerName, LayerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if(!layerFound){
        return false;
      }
    }
    return true;
  }

  void pickPhysicalDevice(){

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0){
      throw std::runtime_error("failed to find GPU with vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(VkPhysicalDevice& device : devices){
      if(isDeviceSuitable(device)){
        physicalDevice = device;
        break;
      }
    }

    if (physicalDevice == VK_NULL_HANDLE){
      throw std::runtime_error("No suitbale GPU found!");
    }

  }

  void mainLoop() {

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

  void cleanup() {
    vkDestroyDevice(device, nullptr);

    if(enableValidationLayers){
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }

  bool isDeviceSuitable(VkPhysicalDevice device){
    QueueFamilyIndices indices = findQueueFamilies(device);

    if(indices.isComplete()){ 
      // Checking Extension Support
      uint32_t extensionCount = 0;
      vkEnumerateDeviceExtensionProperties(device, nullptr,  &extensionCount, nullptr);
      std::vector<VkExtensionProperties> extensionsList(extensionCount);
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensionsList.data());

      std::cout << "available device extensions" << std::endl;
      for(const auto& ext : extensionsList){
        std::cout << '\t' << ext.extensionName << std::endl;
        if( strcmp(ext.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0){
          return true;
        }
      }
    }
    return false;
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device){
    QueueFamilyIndices indices;
    // logic to find family idices and populate the struct
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for(const auto& queueFamily : queueFamilies){
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }
      if(presentSupport) {
        indices.presentFamily = i;
      }
      if (indices.isComplete()){
        break;
      }
      i++;
    }

    return indices;
  }

  void createLogicalDevice(){
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for(uint32_t queueFamily : uniqueQueueFamilies){
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    // createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char*> requiredExtensions = {};

    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();


    if(enableValidationLayers){
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
    deviceFeatures2.features = deviceFeatures;
    

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilitySubsetFeaturesKHR{};
    // todo:
    //  find missing include
    portabilitySubsetFeaturesKHR.sType = static_cast<VkStructureType>(1000163000);
    deviceFeatures2.pNext = &portabilitySubsetFeaturesKHR;


    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
      throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
  }

};

int main() {
  HelloTriangleApplication app;

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
