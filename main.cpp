#include <QCoreApplication>
#include <vulkan/vulkan.h>
#include <vulkan_profiles.hpp>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    VkResult res;
    VkInstance instance;

    VkInstanceCreateInfo instanceCI{};
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    uint32_t extCount;
    res = vkEnumerateInstanceExtensionProperties(NULL, &extCount, NULL);
    assert(res == VK_SUCCESS);
    std::vector<VkExtensionProperties> instanceExtensions(extCount);
    res = vkEnumerateInstanceExtensionProperties(NULL, &extCount, &instanceExtensions.front());
    assert(res == VK_SUCCESS);
    bool deviceProperties2Available = false;
    std::vector<const char*> enabledExtensions{};
    for (auto& ext : instanceExtensions) {
        if (strcmp(ext.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
            deviceProperties2Available = true;
            enabledExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            break;
        }
    }
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instanceCI.ppEnabledExtensionNames = enabledExtensions.data();
    res = vkCreateInstance(&instanceCI, nullptr, &instance);
    assert(res == VK_SUCCESS);
    if (!deviceProperties2Available) {
        qDebug() << "Instance does not support device props2";
        exit(-1);
    }

    uint32_t numGPUs;
    res = vkEnumeratePhysicalDevices(instance, &numGPUs, nullptr);
    assert(res == VK_SUCCESS);
    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(numGPUs);
    res = vkEnumeratePhysicalDevices(instance, &numGPUs, &physicalDevices.front());
    assert(res == VK_SUCCESS);

    std::vector<VpProfileProperties> availableProfiles{};
    uint32_t profilesCount;
    if (vpGetProfiles(&profilesCount, nullptr) == VK_SUCCESS) {
        availableProfiles.resize(profilesCount);
        vpGetProfiles(&profilesCount, availableProfiles.data());
    }

    for (auto& physDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physDevice, &props);
        qDebug() << props.deviceName;
        for (VpProfileProperties& profile : availableProfiles) {
            qInfo() << "Reading profile" << profile.profileName;
            VkBool32 supported = VK_FALSE;
            vpGetPhysicalDeviceProfileSupport(instance, physDevice, &profile, &supported);
        }

    }

    return a.exec();
}
