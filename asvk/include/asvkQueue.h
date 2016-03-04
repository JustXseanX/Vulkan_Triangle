﻿//-------------------------------------------------------------------------------------------------
// File : asvkQueue.h
// Desc : Queue Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <vulkan/vulkan.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// QueueType enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum QueueType
{
    QueueType_Graphics = 0,     //!< グラフィックス用途です.
    QueueType_Compute,          //!< コンピュート用途です.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Queue class
///////////////////////////////////////////////////////////////////////////////////////////////////
class Queue
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    Queue();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~Queue();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理です.
    //!
    //! @param[in]      device          デバイスです.
    //! @param[in]      familyIndex     ファミリーインデックスです.
    //! @param[in]      queueIndex      キューインデックスです.
    //---------------------------------------------------------------------------------------------
    bool Init(VkDevice device, uint32_t familyIndex, uint32_t queueIndex, QueueType type);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理です.
    //---------------------------------------------------------------------------------------------
    void Term(VkDevice device);

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドを実行します.
    //---------------------------------------------------------------------------------------------
    void Execute(uint32_t count, VkCommandBuffer* pBuffers);

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドの完了を待機します.
    //!
    //! @param[in]      timeout     タイムアウト時間です(ナノ秒単位).
    //---------------------------------------------------------------------------------------------
    void Wait(uint64_t timeout);

    //---------------------------------------------------------------------------------------------
    //! @brief      キューを取得します.
    //!
    //! @return     キューを返却します.
    //---------------------------------------------------------------------------------------------
    VkQueue GetQueue() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      フェンスを取得します.
    //!
    //! @return     フェンスを返却します.
    //---------------------------------------------------------------------------------------------
    VkFence GetFence() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      ファミリーインデックスを取得します.
    //!
    //! @return     ファミリーインデックスを返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t GetFamilyIndex() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      キュータイプを取得します.
    //!
    //! @return     キュータイプを返却します.
    //---------------------------------------------------------------------------------------------
    QueueType GetType() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    VkDevice    m_Device;           //!< デバイスです.
    VkQueue     m_Queue;            //!< キューです.
    VkFence     m_Fence;            //!< フェンスです.
    uint32_t    m_FamiliyIndex;     //!< ファミリーインデックスです.
    QueueType   m_Type;             //!< キュータイプです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asvk
