﻿//-------------------------------------------------------------------------------------------------
// File : asvkSwapChain.cpp
// Desc : Swap Chain Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkSwapChain.h>
#include <asvkLogger.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SwapChain class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
SwapChain::SwapChain()
: m_BufferIndex (0)
, m_Surface     (VK_NULL_HANDLE)
, m_SwapChain   (VK_NULL_HANDLE)
, m_Device      (VK_NULL_HANDLE)
, m_Semaphore   (VK_NULL_HANDLE)
, m_pQueue      (VK_NULL_HANDLE)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
SwapChain::~SwapChain()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool SwapChain::Init(DeviceMgr* pDeviceMgr, VkCommandBuffer commandBuffer,const SwapChainDesc* pDesc)
{
    // 引数チェック.
    if (pDeviceMgr == nullptr || commandBuffer == nullptr || pDesc == nullptr)
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    // セマフォの生成.
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        auto result = vkCreateSemaphore(pDeviceMgr->GetDevice(), &info, nullptr, &m_Semaphore);
        if (result != VK_SUCCESS)
        {
            ELOG("Error : vkCreatSemaphore() Failed.");
            return false;
        }
    }

    // サーフェイス生成情報を設定する.
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext       = nullptr;
    surfaceInfo.flags       = 0;
    surfaceInfo.hinstance   = pDesc->hInstance;
    surfaceInfo.hwnd        = pDesc->hWnd;

    // サーフェイス生成.
    auto result = vkCreateWin32SurfaceKHR(pDeviceMgr->GetInstance(), &surfaceInfo, nullptr, &m_Surface);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkCreateWin32SurfaceKHR() Failed." );
        return false;
    }

    // 物理デバイス取得.
    auto gpu = pDeviceMgr->GetPhysicalDevice()[0].Gpu;

    uint32_t count  = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_Surface, &count, nullptr);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkGetPhysicalDeviceSurfaceFormatKHR() Failed." );
        return false;
    }

    std::vector<VkSurfaceFormatKHR> formats;
    formats.resize(count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_Surface, &count, formats.data());
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkGetPhysicalDeviceSUrfaceFormatsKHR() Failed." );
        return false;
    }

    VkFormat        imageFormat     = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    bool isFind = false;
    for(size_t i=0; i<formats.size(); ++i)
    {
        if (pDesc->Format     == formats[i].format &&
            pDesc->ColorSpace == formats[i].colorSpace)
        { 
            imageFormat     = pDesc->Format;
            imageColorSpace = pDesc->ColorSpace;
            isFind          = true;
            break;
        }
    }

    if (!isFind)
    {
        ELOG( "Error : Specified Format Not Found." );
        return false;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    {
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            gpu,
            m_Surface,
            &capabilities);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed.");
            return false;
        }

        if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        { preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; }
        else
        { preTransform = capabilities.currentTransform; }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    {
        uint32_t presentModeCount;
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            gpu,
            m_Surface,
            &presentModeCount,
            nullptr);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkGetPhysicalDeviceSurfacePresentModesKHR() Failed." );
            return false;
        }

        std::vector<VkPresentModeKHR> presentModes;
        presentModes.resize(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            gpu,
            m_Surface,
            &presentModeCount,
            presentModes.data());
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkGetPhysicalDeviceSurfacePresentModesKHR() Failed." );
            return false;
        }

        for(size_t i=0; i<presentModes.size(); ++i)
        {
            // 垂直同期OFFを優先.
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                break;
            }

            // 垂直同期ON
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            { presentMode = VK_PRESENT_MODE_MAILBOX_KHR; }
        }

        presentModes.clear();
    }

    // スワップチェインを生成.
    {
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.surface                  = m_Surface;
        createInfo.minImageCount            = pDesc->BufferCount;
        createInfo.imageFormat              = imageFormat;
        createInfo.imageColorSpace          = imageColorSpace;
        createInfo.imageExtent              = { pDesc->Width, pDesc->Height };
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = preTransform;
        createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode              = presentMode;
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = VK_NULL_HANDLE;

        auto result = vkCreateSwapchainKHR(pDeviceMgr->GetDevice(), &createInfo, nullptr, &m_SwapChain);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkCreateSwapChainKHR() Failed." );
            return false;
        }
    }

    // イメージを取得.
    {
        uint32_t chainCount;
        result = vkGetSwapchainImagesKHR(pDeviceMgr->GetDevice(), m_SwapChain, &chainCount, nullptr);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkGetSwapChainImagesKHR() Failed." );
            return false;
        }

        if ( chainCount != pDesc->BufferCount )
        {
            ELOG( "Error : SwapChain Count is Invalid." );
            return false;
        }

        std::vector<VkImage> images;
        images.resize(chainCount);
        result = vkGetSwapchainImagesKHR(pDeviceMgr->GetDevice(), m_SwapChain, &chainCount, images.data());
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkGetSwapCHainImagesKHR() Failed." );
            return false;
        }

        m_Buffers.resize(chainCount);
        for(size_t i=0; i<m_Buffers.size(); ++i)
        { m_Buffers[i].Image = images[i]; }

        images.clear();
    }

    // イメージビューを生成.
    {
        m_Range.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        m_Range.baseMipLevel    = 0;
        m_Range.levelCount      = 1;
        m_Range.baseArrayLayer  = 0;
        m_Range.layerCount      = 1;

        for(size_t i=0; i<m_Buffers.size(); ++i)
        {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext            = nullptr;
            viewInfo.flags            = 0;
            viewInfo.image            = m_Buffers[i].Image;
            viewInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format           = imageFormat;
            viewInfo.components.r     = VK_COMPONENT_SWIZZLE_R;
            viewInfo.components.g     = VK_COMPONENT_SWIZZLE_G;
            viewInfo.components.b     = VK_COMPONENT_SWIZZLE_B;
            viewInfo.components.a     = VK_COMPONENT_SWIZZLE_A;
            viewInfo.subresourceRange = m_Range;

            auto result = vkCreateImageView(pDeviceMgr->GetDevice(), &viewInfo, nullptr, &m_Buffers[i].View);
            if ( result != VK_SUCCESS )
            {
                ELOG( "Error : vkCreateImageView() Failed." );
                return false;
            }

            VkImageMemoryBarrier barrier = {};
            barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext               = nullptr;
            barrier.srcAccessMask       = 0;
            barrier.dstAccessMask       = 0;
            barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = 0;
            barrier.dstQueueFamilyIndex = 0;
            barrier.image               = m_Buffers[i].Image;
            barrier.subresourceRange    = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier );
        }
    }

    // イメージを取得.
    result = vkAcquireNextImageKHR(
        pDeviceMgr->GetDevice(),
        m_SwapChain,
        UINT64_MAX,
        m_Semaphore,
        VK_NULL_HANDLE,
        &m_BufferIndex);
    if ( result != VK_SUCCESS )
    { 
        ELOG( "Error : vkAcquireNextImageKHR() Failed." );
        return false;
    }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_Device      = pDeviceMgr->GetDevice();
    m_pQueue      = pDeviceMgr->GetGraphicsQueue();
    m_BufferIndex = 0;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void SwapChain::Term(DeviceMgr* pDeviceMgr)
{
    if (pDeviceMgr == nullptr)
    { return; }

    for(size_t i=0; i<m_Buffers.size(); ++i)
    {
        if (m_Buffers[i].View != VK_NULL_HANDLE)
        { vkDestroyImageView(pDeviceMgr->GetDevice(), m_Buffers[i].View, nullptr); }
    }

    if (m_SwapChain != VK_NULL_HANDLE)
    { vkDestroySwapchainKHR(pDeviceMgr->GetDevice(), m_SwapChain, nullptr); }

    if (m_Surface != VK_NULL_HANDLE)
    { vkDestroySurfaceKHR(pDeviceMgr->GetInstance(), m_Surface, nullptr); }

    if (m_Semaphore != VK_NULL_HANDLE)
    { vkDestroySemaphore(pDeviceMgr->GetDevice(), m_Semaphore, nullptr); }

    memset(&m_Desc,  0, sizeof(m_Desc));
    memset(&m_Range, 0, sizeof(m_Range));

    m_SwapChain = VK_NULL_HANDLE;
    m_Surface   = VK_NULL_HANDLE;
    m_pQueue    = VK_NULL_HANDLE;
    m_Device    = VK_NULL_HANDLE;
    m_Semaphore = VK_NULL_HANDLE;
    m_Buffers.clear();
}

//-------------------------------------------------------------------------------------------------
//      表示します.
//-------------------------------------------------------------------------------------------------
void SwapChain::Present(uint64_t timeout)
{
    VkPresentInfoKHR present = {};
    present.sType           = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext           = nullptr;
    present.swapchainCount  = 1;
    present.pSwapchains     = &m_SwapChain;
    present.pImageIndices   = &m_BufferIndex;

    // 表示.
    auto result = vkQueuePresentKHR(m_pQueue->GetQueue(), &present);
    if (result == VK_ERROR_OUT_OF_HOST_MEMORY )
    { ELOG( "Error : vkQueuePresentKHR() Failed. ErrorCode = VK_ERROR_OUT_OF_HOST_MEMORY" ); }
    else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY )
    { ELOG( "Error : vkQueuePresentKHR() Failed. ErrorCode = VK_ERROR_OUT_OF_DEVICE_MEMORY" ); }
    else if (result == VK_ERROR_OUT_OF_DATE_KHR)
    { ELOG( "Error : vkQueuePresentKHR() Failed. ErrorCode = VK_EROR_OUT_OF_DATE_KHR" ); }
    else if (result == VK_ERROR_SURFACE_LOST_KHR )
    { ELOG( "Error : vkQueuePresentKHR() Failed. ErrorCode = VK_ERROR_SURFACE_LOST_KHR" ); }
    else if (result == VK_ERROR_DEVICE_LOST)
    {
        // エラーログ出力.
        ELOG( "Fatal Error : vkWaitForFences() Error code = VK_ERROR_DEVICE_LOST" );

        // メッセージボックスを表示.
        MessageBoxW(
            nullptr,
            L"致命的なエラーが発生したため，アプリケーションを終了致します。\nご迷惑をお掛けして申し訳ございません。",
            L"致命的なエラーが発生しました",
            MB_ICONERROR | MB_OK);

        // 殺す.
        abort();
    }

    // イメージを取得.
    result = vkAcquireNextImageKHR(
        m_Device,
        m_SwapChain,
        timeout,
        m_Semaphore,
        VK_NULL_HANDLE,
        &m_BufferIndex);
    if ( result != VK_SUCCESS )
    { ELOG( "Error : vkAcquireNextImageKHR() Failed." ); }
}

//-------------------------------------------------------------------------------------------------
//      バッファ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t SwapChain::GetBufferIndex() const
{ return m_BufferIndex; }

//-------------------------------------------------------------------------------------------------
//      スワップチェインを取得します.
//-------------------------------------------------------------------------------------------------
VkSwapchainKHR SwapChain::GetSwapChain() const
{ return m_SwapChain; }

//-------------------------------------------------------------------------------------------------
//      サーフェイスを取得します.
//-------------------------------------------------------------------------------------------------
VkSurfaceKHR SwapChain::GetSurface() const
{ return m_Surface; }

//-------------------------------------------------------------------------------------------------
//      セマフォを取得します.
//-------------------------------------------------------------------------------------------------
VkSemaphore SwapChain::GetSemaphore() const
{ return m_Semaphore; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
SwapChain::Buffer const * SwapChain::GetBuffer(uint32_t index) const
{
    if (index < m_Buffers.size() )
    { return &m_Buffers[index]; }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
//      現在のバッファを取得します.
//-------------------------------------------------------------------------------------------------
SwapChain::Buffer const * SwapChain::GetCurrentBuffer() const
{ return &m_Buffers[m_BufferIndex]; }

//-------------------------------------------------------------------------------------------------
//      イメージサブリソースレンジを取得します.
//-------------------------------------------------------------------------------------------------
VkImageSubresourceRange SwapChain::GetRange() const
{ return m_Range; }

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
const SwapChainDesc& SwapChain::GetDesc() const
{ return m_Desc; }

} // namespace asvk
