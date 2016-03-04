//-------------------------------------------------------------------------------------------------
// File : asvkResource.cpp
// Desc : Resource Module.
// Copyright(c) Project Asura.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkResource.h>
#include <asvkLogger.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImageResource class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ImageResource::ImageResource()
: m_Resource(null_handle)
, m_Memory  (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ImageResource::~ImageResource()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool ImageResource::Init
(
    VkDevice                         device,
    VkPhysicalDeviceMemoryProperties props,
    const VkImageCreateInfo*         pInfo 
)
{
    if (device == null_handle || pInfo == nullptr)
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto result = vkCreateImage(device, pInfo, nullptr, &m_Resource);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkCreateImage() Failed." );
        return false;
    }

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, m_Resource, &requirements);

    VkFlags requirementsMask = 0;
    auto typeBits  = requirements.memoryTypeBits;
    auto typeIndex = 0u;
    for (auto i=0u; i<32; ++i)
    {
        auto& propFlags = props.memoryTypes[i].propertyFlags;
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
    allocInfo.memoryTypeIndex   = typeIndex;
    allocInfo.allocationSize    = requirements.size;

    result = vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkAllocateMemory() Failed." );
        return false;
    }

    result = vkBindImageMemory(device, m_Resource, m_Memory, 0);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkBindBufferMemory() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void ImageResource::Term(VkDevice device)
{
    if (m_Memory != null_handle)
    { vkFreeMemory(device, m_Memory, nullptr); }

    if (m_Resource != null_handle)
    { vkDestroyImage(device, m_Resource, nullptr); }

    m_Memory    = null_handle;
    m_Resource  = null_handle;
}

//-------------------------------------------------------------------------------------------------
//      マップします.
//-------------------------------------------------------------------------------------------------
bool ImageResource::Map
(
    VkDevice            device,
    VkDeviceSize        offset,
    VkDeviceSize        size,
    VkMemoryMapFlags    flags,
    void**              ppData
) const
{
    auto result = vkMapMemory(device, m_Memory, offset, size, flags, ppData);
    if (result != VK_SUCCESS)
    {
        ELOG( "Error : vkMapMemory() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      アンマップします.
//-------------------------------------------------------------------------------------------------
void ImageResource::Unmap(VkDevice device) const
{ vkUnmapMemory(device, m_Memory); }

//-------------------------------------------------------------------------------------------------
//      デバイスメモリを取得します.
//-------------------------------------------------------------------------------------------------
VkDeviceMemory ImageResource::GetMemory() const
{ return m_Memory; }

//-------------------------------------------------------------------------------------------------
//      イメージを取得します.
//-------------------------------------------------------------------------------------------------
VkImage ImageResource::GetImage() const
{ return m_Resource; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// BufferResource class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
BufferResource::BufferResource()
: m_Resource(null_handle)
, m_Memory  (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
BufferResource::~BufferResource()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool BufferResource::Init(VkDevice device, const VkBufferCreateInfo* pInfo)
{
    if (device == null_handle || pInfo == nullptr)
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto result = vkCreateBuffer(device, pInfo, nullptr, &m_Resource);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkCreateBuffer() Failed." );
        return false;
    }

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, m_Resource, &requirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext             = nullptr;
    allocInfo.memoryTypeIndex   = 0;
    allocInfo.allocationSize    = requirements.size;

    result = vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkAllocateMemory() Failed." );
        return false;
    }

    result = vkBindBufferMemory(device, m_Resource, m_Memory, 0);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkBindBufferMemory() Failed." );
        return false;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void BufferResource::Term(VkDevice device)
{
    if (m_Memory != null_handle)
    { vkFreeMemory(device, m_Memory, nullptr); }

    if (m_Resource != null_handle)
    { vkDestroyBuffer(device, m_Resource, nullptr); }

    m_Memory   = null_handle;
    m_Resource = null_handle;
}

//-------------------------------------------------------------------------------------------------
//      マップします.
//-------------------------------------------------------------------------------------------------
bool BufferResource::Map
(
    VkDevice            device,
    VkDeviceSize        offset,
    VkDeviceSize        size,
    VkMemoryMapFlags    flags,
    void**              ppData
) const
{
    auto result = vkMapMemory(device, m_Memory, offset, size, flags, ppData);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkMapMemory() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      アンマップします.
//-------------------------------------------------------------------------------------------------
void BufferResource::Unmap(VkDevice device) const
{ vkUnmapMemory(device, m_Memory); }

//-------------------------------------------------------------------------------------------------
//      デバイスメモリを取得します.
//-------------------------------------------------------------------------------------------------
VkDeviceMemory BufferResource::GetMemory() const
{ return m_Memory; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
VkBuffer BufferResource::GetBuffer() const
{ return m_Resource; }


} // namespace asvk
