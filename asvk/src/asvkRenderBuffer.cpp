//-------------------------------------------------------------------------------------------------
// File : asvkRenderBuffer.cpp
// Desc : Render Buffer Moudle.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkRenderBuffer.h>
#include <asvkLogger.h>
#include <cassert>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      イメージレイアウトを設定します.
//-------------------------------------------------------------------------------------------------
void SetImageLayout
(
    VkDevice                device,
    VkCommandBuffer         commandBuffer,
    VkImage                 image,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout,
    VkImageSubresourceRange range
)
{
    assert(device        != nullptr);
    assert(commandBuffer != nullptr);

    VkImageMemoryBarrier barrier = {};
    barrier.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext            = nullptr;
    barrier.srcAccessMask    = 0;
    barrier.dstAccessMask    = 0;
    barrier.oldLayout        = oldLayout;
    barrier.newLayout        = newLayout;
    barrier.image            = image;
    barrier.subresourceRange = range;

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    { barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; }

    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    { barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; }

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    { barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; }

    if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    { barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; }

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);
}

} // namespace /* anonymous */

namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
RenderBuffer::RenderBuffer()
: m_Image   (VK_NULL_HANDLE)
, m_View    (VK_NULL_HANDLE)
, m_Memory  (VK_NULL_HANDLE)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
RenderBuffer::~RenderBuffer()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool RenderBuffer::Init
(
    DeviceMgr*              pDeviceMgr,
    VkCommandBuffer         commandBuffer,
    const RenderBufferDesc* pDesc
)
{
    if (pDeviceMgr == nullptr || commandBuffer == nullptr || pDesc == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto device     = pDeviceMgr->GetDevice();
    auto gpu        = pDeviceMgr->GetPhysicalDevice()[0].Gpu;
    auto memoryProp = pDeviceMgr->GetPhysicalDevice()[0].MemoryProps;

    VkImageLayout       imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageAspectFlags  aspect      = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageTiling       tiling      = VK_IMAGE_TILING_OPTIMAL;
    VkFormatProperties  props;
    vkGetPhysicalDeviceFormatProperties(gpu, pDesc->Format, &props);

    if(pDesc->Usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        aspect      = VK_IMAGE_ASPECT_COLOR_BIT;
        imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if(props.linearTilingFeatures & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        { tiling = VK_IMAGE_TILING_LINEAR; }
        else if(props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
        { tiling = VK_IMAGE_TILING_OPTIMAL; }
    }
    else if (pDesc->Usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        aspect      = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        { tiling = VK_IMAGE_TILING_LINEAR; }
        else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        { tiling = VK_IMAGE_TILING_OPTIMAL; }
    }

    VkImageCreateInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext                  = nullptr;
    info.flags                  = 0;
    info.imageType              = pDesc->Dimension;
    info.format                 = pDesc->Format;
    info.extent                 = { pDesc->Width, pDesc->Height, pDesc->Depth };
    info.mipLevels              = pDesc->MipLevels;
    info.arrayLayers            = pDesc->ArraySize;
    info.samples                = pDesc->Samples;
    info.tiling                 = tiling;
    info.usage                  = pDesc->Usage;
    info.queueFamilyIndexCount  = 0;
    info.pQueueFamilyIndices    = nullptr;
    info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout          = VK_IMAGE_LAYOUT_UNDEFINED;

    auto result = vkCreateImage(device, &info, nullptr, &m_Image);
    if (result != VK_SUCCESS)
    {
        ELOG("Error : vkCreatImage() Failed.");
        return false;
    }

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, m_Image, &requirements);

    VkFlags requirementsMask = 0;
    uint32_t typeBits  = requirements.memoryTypeBits;
    uint32_t typeIndex = 0;
    for (auto i=0u; i<32; ++i)
    {
        auto& propFlags = memoryProp.memoryTypes[i].propertyFlags;
        if ((typeBits & 0x1) == 0x1)
        {
            if ((propFlags & requirementsMask) == requirementsMask)
            {
                typeIndex = i;
                break;
            }
        }
        typeBits >>= 1;
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext             = nullptr;
    allocInfo.allocationSize    = requirements.size;
    allocInfo.memoryTypeIndex   = typeIndex;

    result = vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory);
    if (result != VK_SUCCESS)
    {
        ELOG( "Error : vkAllocateMemory() Failed." );
        return false;
    }

    result = vkBindImageMemory(device, m_Image, m_Memory, 0);
    if (result != VK_SUCCESS)
    {
        ELOG( "Error : vkBindImageMemory() Failed." );
        return false;
    }

    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    if (pDesc->Dimension == VK_IMAGE_TYPE_1D)
    {
        if(pDesc->ArraySize > 1)
        { viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY; }
        else
        { viewType = VK_IMAGE_VIEW_TYPE_1D; }
    }
    else if (pDesc->Dimension == VK_IMAGE_TYPE_2D)
    {
        if (pDesc->ArraySize > 1)
        { viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY; }
        else
        { viewType = VK_IMAGE_VIEW_TYPE_2D; }
    }
    else if (pDesc->Dimension == VK_IMAGE_TYPE_3D)
    { viewType = VK_IMAGE_VIEW_TYPE_3D; }

    m_Range.aspectMask      = aspect;
    m_Range.baseMipLevel    = 0;
    m_Range.levelCount      = pDesc->MipLevels;
    m_Range.baseArrayLayer  = 0;
    m_Range.layerCount      = pDesc->ArraySize;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext            = nullptr;
    viewInfo.flags            = 0;
    viewInfo.image            = m_Image;
    viewInfo.viewType         = viewType;
    viewInfo.format           = pDesc->Format;
    viewInfo.components.r     = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g     = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b     = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a     = VK_COMPONENT_SWIZZLE_A;
    viewInfo.subresourceRange = m_Range;

    result = vkCreateImageView(device, &viewInfo, nullptr, &m_View);
    if (result != VK_SUCCESS)
    {
        ELOG( "Error : vkCreateImageView() Failed." );
        return false;
    }

    SetImageLayout(
        device,
        commandBuffer,
        m_Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        imageLayout,
        m_Range);

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void RenderBuffer::Term(DeviceMgr* pDeviceMgr)
{
    if (pDeviceMgr == nullptr)
    { return; }

    auto device = pDeviceMgr->GetDevice();
    if (device == nullptr)
    { return; }

    if (m_View != VK_NULL_HANDLE)
    { vkDestroyImageView(device, m_View, nullptr); }

    if (m_Image != VK_NULL_HANDLE)
    { vkDestroyImage(device, m_Image, nullptr); }

    if (m_Memory != VK_NULL_HANDLE)
    { vkFreeMemory(device, m_Memory, nullptr); }

    memset(&m_Desc,  0, sizeof(m_Desc));
    memset(&m_Range, 0, sizeof(m_Range));

    m_Memory = VK_NULL_HANDLE;
    m_View   = VK_NULL_HANDLE;
    m_Image  = VK_NULL_HANDLE;
}

//-------------------------------------------------------------------------------------------------
//      設定を取得します.
//-------------------------------------------------------------------------------------------------
const RenderBufferDesc& RenderBuffer::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      イメージを取得します.
//-------------------------------------------------------------------------------------------------
VkImage RenderBuffer::GetImage() const
{ return m_Image; }

//-------------------------------------------------------------------------------------------------
//      デバイスメモリを取得します.
//-------------------------------------------------------------------------------------------------
VkDeviceMemory RenderBuffer::GetDeviceMemory() const
{ return m_Memory; }

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView RenderBuffer::GetView() const
{ return m_View; }

//-------------------------------------------------------------------------------------------------
//      イメージサブリソースレンジを取得します.
//-------------------------------------------------------------------------------------------------
VkImageSubresourceRange RenderBuffer::GetRange() const
{ return m_Range; }

} // namespace asvk
