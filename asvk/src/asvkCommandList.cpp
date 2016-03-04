//-------------------------------------------------------------------------------------------------
// File : asvkCommandList.cpp
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkCommandList.h>
#include <asvkLogger.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::CommandList()
: m_CommandPool(null_handle)
, m_BufferIndex(0)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::~CommandList()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandList::Init
(
    DeviceMgr*                  pDeviceMgr,
    QueueType                   queueType,
    VkCommandPoolCreateFlags    createFlags,
    VkCommandBufferLevel        level,
    uint32_t                    count
)
{
    if (pDeviceMgr == nullptr || count == 0)
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    // コマンドプールの生成.
    {
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        if (queueType == QueueType_Graphics)
        { info.queueFamilyIndex = pDeviceMgr->GetGraphicsQueue()->GetFamilyIndex(); }
        else
        { info.queueFamilyIndex = pDeviceMgr->GetComputeQueue()->GetFamilyIndex(); }
        info.flags = createFlags;

        auto result = vkCreateCommandPool(pDeviceMgr->GetDevice(), &info, nullptr, &m_CommandPool);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkCreateCommandPool() Failed." );
            return false;
        }
    }

    // コマンドバッファの生成
    {
        m_CommandBuffers.resize(count);

        VkCommandBufferAllocateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext              = nullptr;
        info.commandPool        = m_CommandPool;
        info.level              = level;
        info.commandBufferCount = count;
       
        auto result = vkAllocateCommandBuffers(pDeviceMgr->GetDevice(), &info, m_CommandBuffers.data());
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkAllocateCommandBuffers() Failed." );
            return false;
        }
    }

    m_BufferIndex = 0;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandList::Term(DeviceMgr* pDeviceMgr)
{
    if (m_CommandPool != null_handle && !m_CommandBuffers.empty())
    {
        vkFreeCommandBuffers(
            pDeviceMgr->GetDevice(),
            m_CommandPool,
            static_cast<uint32_t>(m_CommandBuffers.size()),
            m_CommandBuffers.data());
    }

    if (m_CommandPool != null_handle)
    { vkDestroyCommandPool(pDeviceMgr->GetDevice(), m_CommandPool, nullptr); }

    m_CommandPool = null_handle;
    m_BufferIndex = 0;
    m_CommandBuffers.clear();
}

//-------------------------------------------------------------------------------------------------
//      コマンドバッファを先頭に戻し，記録を開始します
//-------------------------------------------------------------------------------------------------
bool CommandList::Reset()
{
    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext                = nullptr;
    inheritanceInfo.renderPass           = null_handle;
    inheritanceInfo.subpass              = 0;
    inheritanceInfo.framebuffer          = null_handle;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags           = 0;
    inheritanceInfo.pipelineStatistics   = 0;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = nullptr;
    beginInfo.flags            = 0;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    auto result = vkBeginCommandBuffer(m_CommandBuffers[m_BufferIndex], &beginInfo);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkBeginCommandBuffer() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドバッファへの記録を終了します.
//-------------------------------------------------------------------------------------------------
bool CommandList::Close()
{
    auto result = vkEndCommandBuffer(m_CommandBuffers[m_BufferIndex]);
    if ( result != VK_SUCCESS )
    {
        ELOG( "Error : vkEndCommandBuffer() Failed." );
        return false;
    }

    // コマンドバッファインデックスを交換.
    m_BufferIndex = (m_BufferIndex + 1) % m_CommandBuffers.size();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドプールを取得します.
//-------------------------------------------------------------------------------------------------
VkCommandPool CommandList::GetCommandPool() const
{ return m_CommandPool; }

//-------------------------------------------------------------------------------------------------
//      コマンドバッファを取得します.
//-------------------------------------------------------------------------------------------------
VkCommandBuffer CommandList::GetCommandBuffer(uint32_t index) const
{ return m_CommandBuffers[index]; }

//-------------------------------------------------------------------------------------------------
//      現在のコマンドバッファを取得します.
//-------------------------------------------------------------------------------------------------
VkCommandBuffer CommandList::GetCurrentCommandBuffer() const
{ return m_CommandBuffers[m_BufferIndex]; }

//-------------------------------------------------------------------------------------------------
//      バッファインデックスを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t CommandList::GetBufferIndex() const
{ return m_BufferIndex; }

} // namespace asvk
