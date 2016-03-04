//-------------------------------------------------------------------------------------------------
// File : asvkQueue.cpp
// Desc : Queue Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include
//-------------------------------------------------------------------------------------------------
#include <asvkQueue.h>
#include <asvkLogger.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Queue class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Queue::Queue()
: m_Device      (null_handle)
, m_Queue       (null_handle)
, m_Fence       (null_handle)
, m_FamiliyIndex(0)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Queue::~Queue()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Queue::Init(VkDevice device, uint32_t familyIndex, uint32_t queueIndex, QueueType type)
{
    if (device == nullptr)
    { return false; }

    vkGetDeviceQueue(device, familyIndex, queueIndex, &m_Queue);

    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        auto result = vkCreateFence(device, &info, nullptr, &m_Fence);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkCreateFence() Failed." );
            return false;
        }
    }

    m_Device       = device;
    m_FamiliyIndex = familyIndex;
    m_Type         = type;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Term(VkDevice device)
{
    if (m_Fence != null_handle && device != null_handle)
    { vkDestroyFence(device, m_Fence, nullptr); }

    m_Device    = null_handle;
    m_Fence     = null_handle;
    m_Queue     = null_handle;
    m_FamiliyIndex = 0;
}

//-------------------------------------------------------------------------------------------------
//      コマンドを実行します.
//-------------------------------------------------------------------------------------------------
void Queue::Execute(uint32_t count, VkCommandBuffer* pBuffers)
{
    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext                  = nullptr;
    info.waitSemaphoreCount     = 0;
    info.pWaitSemaphores        = nullptr;
    info.pWaitDstStageMask      = &waitDstStageMask;
    info.commandBufferCount     = count;
    info.pCommandBuffers        = pBuffers;
    info.signalSemaphoreCount   = 0;
    info.pSignalSemaphores      = nullptr;

    vkQueueSubmit(m_Queue, 1, &info, m_Fence);
}

//-------------------------------------------------------------------------------------------------
//      コマンドの完了を待機します.
//-------------------------------------------------------------------------------------------------
void Queue::Wait(uint64_t timeout)
{
    auto result = vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, timeout);
    if (result == VK_TIMEOUT)
    { ILOG( "Info : vkWaitForFences() Timeout. time out nanoseconds = %ld", timeout ); }
    else if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
    { ELOG( "Error : vkWaitForFences() Failed. ErrorCode = VK_ERROR_OUT_OF_HOST_MEMORY" ); }
    else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    { ELOG( "Error : vkWaitForFences() Failed. ErrorCode = VK_ERROR_OUT_OF_DEVICE_MEMORY" ); }
    else if (result == VK_ERROR_DEVICE_LOST)
    {
        // エラーログ表示.
        ELOG( "Fatal Error : vkWaitForFences() Failed. ErrorCode = VK_ERROR_DEVICE_LOST " );

        // 続行不能なので殺す.
        abort();
    }

    // フェンスをリセット.
    vkResetFences(m_Device, 1, &m_Fence);
}
//-------------------------------------------------------------------------------------------------
//      キューを取得します.
//-------------------------------------------------------------------------------------------------
VkQueue Queue::GetQueue() const
{ return m_Queue; }

//-------------------------------------------------------------------------------------------------
//      フェンスを取得します.
//-------------------------------------------------------------------------------------------------
VkFence Queue::GetFence() const
{ return m_Fence; }

//-------------------------------------------------------------------------------------------------
//      ファミリーインデックスを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Queue::GetFamilyIndex() const
{ return m_FamiliyIndex; }

//-------------------------------------------------------------------------------------------------
//      キュータイプを取得します.
//-------------------------------------------------------------------------------------------------
QueueType Queue::GetType() const
{ return m_Type; }

} // namespace asvk
