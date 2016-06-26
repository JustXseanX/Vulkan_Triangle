//-------------------------------------------------------------------------------------------------
// File : asvkDevice.pp
// Desc : Device Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkTypedef.h>
#include <asvkDevice.h>
#include <asvkLogger.h>
#include <array>
#include <cassert>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      メモリ確保処理.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR
void* VKAPI_CALL Alloc
(
    void*                   pUserData,
    size_t                  size,
    size_t                  alignment,
    VkSystemAllocationScope scope
)
{
    ASVK_UNUSED(pUserData);
    ASVK_UNUSED(scope);
    return _aligned_malloc(size, alignment);
}

//-------------------------------------------------------------------------------------------------
//      メモリ再確保処理.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR 
void* VKAPI_CALL Realloc
(
    void*                   pUserData,
    void*                   pOriginal,
    size_t                  size,
    size_t                  alignment,
    VkSystemAllocationScope scope
)
{
    ASVK_UNUSED(pUserData);
    ASVK_UNUSED(scope);
    return _aligned_realloc(pOriginal, size, alignment);
}

//-------------------------------------------------------------------------------------------------
//      メモリ解放処理.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR
void VKAPI_CALL Free(void* pUserData, void* pMemory)
{
    ASVK_UNUSED(pUserData);
    _aligned_free( pMemory );
}

//-------------------------------------------------------------------------------------------------
//      デバッグリポートを行います.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR
VkBool32 VKAPI_CALL DebugReport
(
    VkFlags                     msgFlags,
    VkDebugReportObjectTypeEXT  objType,
    uint64_t                    srcObject,
    size_t                      location,
    int32_t                     msgCode,
    const char*                 pLayerPrefix,
    const char*                 pMsg,
    void*                       pUserData
)
{
    ASVK_UNUSED(objType);
    ASVK_UNUSED(srcObject);
    ASVK_UNUSED(location);
    ASVK_UNUSED(pUserData);

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        asvk::SystemLogger::GetInstance().LogA( asvk::LogLevel::Error,
        "Error : [%s] Code %d : %s\n", pLayerPrefix, msgCode, pMsg);
    }
    else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) 
    {
        asvk::SystemLogger::GetInstance().LogA( asvk::LogLevel::Warning,
        "Warning : [%s] Code %d : %s\n", pLayerPrefix, msgCode, pMsg);
    }
    else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        asvk::SystemLogger::GetInstance().LogA( asvk::LogLevel::Info,
        "Info : [%s] Code %d : %s\n", pLayerPrefix, msgCode, pMsg);
    }
    else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        asvk::SystemLogger::GetInstance().LogA( asvk::LogLevel::Debug,
        "Info : [%s] Code %d : %s\n", pLayerPrefix, msgCode, pMsg);
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      インスタンスプロシージャアドレスを取得します.
//-------------------------------------------------------------------------------------------------
ASVK_TEMPLATE(T)
T GetProc(VkInstance instance, const char* name)
{ return reinterpret_cast<T>(vkGetInstanceProcAddr(instance, name)); }

//-------------------------------------------------------------------------------------------------
//      デバイスプロシージャアドレスを取得します.
//-------------------------------------------------------------------------------------------------
ASVK_TEMPLATE(T)
T GetProc(VkDevice device, const char* name)
{ return reinterpret_cast<T>(vkGetDeviceProcAddr(device, name)); }

} // namespace /* anonymous */


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DeviceMgr::DeviceMgr()
: m_Instance        ( null_handle )
, m_Device          ( null_handle )
, m_GraphicsQueue   ()
, m_ComputeQueue    ()
#if ASVK_IS_DEBUG
, m_DebugReporter               ( null_handle )
, m_CreateDebugReportCallback   ( nullptr )
, m_DestroyDebugReportCallback  ( nullptr )
, m_DebugReportMessage          ( nullptr )
#endif
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DeviceMgr::~DeviceMgr()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DeviceMgr::Init()
{
    #if ASVK_IS_DEBUG
        std::array<const char*, 3> layerExtensions;
        layerExtensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
        layerExtensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        layerExtensions[2] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;

        const char* layer[]     = { "VK_LAYER_LUNARG_standard_validation" };
        uint32_t    layerCount  = 1;
    #else
        std::array<const char*, 2> layerExtensions;
        layerExtensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
        layerExtensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;

        const char* layer[]     = { nullptr };
        uint32_t    layerCount  = 0;
    #endif

    // インスタンスの生成.
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext              = nullptr;
        appInfo.pApplicationName   = "asvkApplication";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName        = "asvk";
        appInfo.engineVersion      = ASVK_CURRENT_VERSION_NUMBER;
        appInfo.apiVersion         = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pNext                      = nullptr;
        instanceInfo.flags                      = 0;
        instanceInfo.pApplicationInfo           = &appInfo;
        instanceInfo.enabledLayerCount          = layerCount;
        instanceInfo.ppEnabledLayerNames        = layer;
        instanceInfo.enabledExtensionCount      = static_cast<uint32_t>(layerExtensions.size());
        instanceInfo.ppEnabledExtensionNames    = layerExtensions.data();

        m_Allocator.pfnAllocation         = Alloc;
        m_Allocator.pfnFree               = Free;
        m_Allocator.pfnReallocation       = Realloc;
        m_Allocator.pfnInternalAllocation = nullptr;
        m_Allocator.pfnInternalFree       = nullptr;

        auto result = vkCreateInstance(&instanceInfo, &m_Allocator, &m_Instance);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkCreatInstance() Failed." );
            return false;
        }
    }

    #if ASVK_IS_DEBUG
    {
        m_CreateDebugReportCallback  = GetProc<PFN_vkCreateDebugReportCallbackEXT>
                                            (m_Instance, "vkCreateDebugReportCallbackEXT");
        m_DestroyDebugReportCallback = GetProc<PFN_vkDestroyDebugReportCallbackEXT>
                                            (m_Instance, "vkDestroyDebugReportCallbackEXT");
        m_DebugReportMessage         = GetProc<PFN_vkDebugReportMessageEXT>
                                            (m_Instance, "vkDebugReportMessageEXT");

        if (m_CreateDebugReportCallback  != nullptr &&
            m_DestroyDebugReportCallback != nullptr &&
            m_DebugReportMessage         != nullptr)
        {
            VkDebugReportCallbackCreateInfoEXT info = {};
            info.sType          = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            info.pNext          = nullptr;
            info.pfnCallback    = DebugReport;
            info.pUserData      = nullptr;
            info.flags          = VK_DEBUG_REPORT_ERROR_BIT_EXT 
                                | VK_DEBUG_REPORT_WARNING_BIT_EXT
                                | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

            auto result = m_CreateDebugReportCallback(m_Instance, &info, nullptr, &m_DebugReporter);
            if (result != VK_SUCCESS)
            {
                ELOG( "Error : vkCreateDebugReportCallbackEXT() Failed." );
                return false;
            }
        }
    }
    #endif

    // 物理デバイスの取得.
    {
        uint32_t count = 0;
        auto result = vkEnumeratePhysicalDevices(m_Instance, &count, nullptr);
        if ( result != VK_SUCCESS || count < 1 )
        {
            ELOG( "Error : vkEnumeratePhysicalDevices() Failed." );
            return false;
        }

        std::vector<VkPhysicalDevice> gpus;
        gpus.resize(count);
        result = vkEnumeratePhysicalDevices(m_Instance, &count, gpus.data());
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkEnumeratePhysicalDevices() Failed." );
            return false;
        }

        m_PhysicalDevice.resize(count);
        for(auto i=0u; i<count; ++i)
        {
            m_PhysicalDevice[i].Gpu = gpus[i];
            vkGetPhysicalDeviceMemoryProperties(gpus[i], &m_PhysicalDevice[i].MemoryProps);
        }

        gpus.clear();
    }

    auto gpu = m_PhysicalDevice[0].Gpu;

    // デバイスとキューの生成.
    {
        uint32_t propCount;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &propCount, nullptr);

        std::vector<VkQueueFamilyProperties> props;
        props.resize(propCount);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &propCount, props.data());

        uint32_t familyIndex = 0;

        std::vector<float> priorities;
        auto offset = 0;

        for(auto i=0u; i<propCount; ++i)
        {
            for(auto j=0u; j<props[i].queueCount; ++j)
            { priorities.push_back( 0.0f ); }

            offset += props[i].queueCount;

            if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
             && (props[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
            { 
                familyIndex = i;
            }
        }

        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.pNext              = nullptr;
        queueInfo.flags              = 0;
        queueInfo.queueCount         = props[familyIndex].queueCount;
        queueInfo.queueFamilyIndex   = familyIndex;
        queueInfo.pQueuePriorities   = priorities.data();

        const char* deviceExtensions[]   = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        uint32_t    deviceExtensionCount = 1;

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pNext                    = nullptr;
        deviceInfo.queueCreateInfoCount     = 1;
        deviceInfo.pQueueCreateInfos        = &queueInfo;
        deviceInfo.enabledLayerCount        = layerCount;
        deviceInfo.ppEnabledLayerNames      = layer;
        deviceInfo.enabledExtensionCount    = deviceExtensionCount;
        deviceInfo.ppEnabledExtensionNames  = deviceExtensions;
        deviceInfo.pEnabledFeatures         = nullptr;

        auto result = vkCreateDevice(gpu, &deviceInfo, nullptr, &m_Device);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkCreateDevice() Failed." );
            return false;
        }

        m_GraphicsQueue.Init(m_Device, familyIndex, 0, QueueType_Graphics);
        m_ComputeQueue .Init(m_Device, familyIndex, 1, QueueType_Compute);

        props.clear();
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DeviceMgr::Term()
{
    m_GraphicsQueue.Term(m_Device);
    m_ComputeQueue .Term(m_Device);

    if (m_Device != null_handle)
    { vkDestroyDevice(m_Device, nullptr); }

#if ASVK_IS_DEBUG
    if (m_DebugReporter != nullptr)
    {
        m_DestroyDebugReportCallback(m_Instance, m_DebugReporter, nullptr);
        m_DebugReporter              = null_handle;
        m_CreateDebugReportCallback  = nullptr;
        m_DestroyDebugReportCallback = nullptr;
        m_DebugReportMessage         = nullptr;
    }
#endif

    if (m_Instance != null_handle)
    { vkDestroyInstance(m_Instance, &m_Allocator); }

    m_PhysicalDevice.clear();

    m_Device   = null_handle;
    m_Instance = null_handle;
}

//-------------------------------------------------------------------------------------------------
//      インスタンスを取得します.
//-------------------------------------------------------------------------------------------------
VkInstance DeviceMgr::GetInstance() const
{ return m_Instance; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
VkDevice DeviceMgr::GetDevice() const
{ return m_Device; }

//-------------------------------------------------------------------------------------------------
//      物理デバイスを取得します.
//-------------------------------------------------------------------------------------------------
const std::vector<PhysicalDevice>& DeviceMgr::GetPhysicalDevice() const
{ return m_PhysicalDevice; }

//-------------------------------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-------------------------------------------------------------------------------------------------
Queue* DeviceMgr::GetGraphicsQueue()
{ return &m_GraphicsQueue; }

//-------------------------------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-------------------------------------------------------------------------------------------------
Queue* DeviceMgr::GetComputeQueue()
{ return &m_ComputeQueue; }


} // namespace asvk
