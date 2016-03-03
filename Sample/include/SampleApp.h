﻿//-------------------------------------------------------------------------------------------------
// File : SampleApp.h
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkApp.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Mesh
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Mesh
{
    VkBuffer                            Buffer;             //!< 頂点バッファ.
    VkDeviceMemory                      Memory;             //!< デバイスメモリ.
    VkVertexInputBindingDescription     Bindings;           //!< 入力バインディング設定.
    VkVertexInputAttributeDescription   Attributes[3];      //!< 入力属性設定.

    Mesh()
        : Buffer(VK_NULL_HANDLE)
        , Memory(VK_NULL_HANDLE)
    {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////////////////////////
class SampleApp : public asvk::App
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    static constexpr uint64_t TimeOut = 1000 * 1000 * 100;      //!< タイムアウト時間(ナノ秒単位).

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    SampleApp();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~SampleApp();

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    asvk::Queue*            m_pQueue;           //!< グラフィックスキューです.
    VkPipelineLayout        m_PipelineLayout;   //!< パイプラインレイアウトです.
    VkPipelineCache         m_PipelineCache;    //!< パイプラインキャッシュです.
    VkPipeline              m_Pipeline;         //!< パイプラインです.
    Mesh                    m_Mesh;             //!< メッシュです.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化時の処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool OnInit() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      終了時の処理です.
    //---------------------------------------------------------------------------------------------
    void OnTerm() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレーム遷移時の処理です.
    //!
    //! @param[in]      args        フレームイベント引数.
    //---------------------------------------------------------------------------------------------
    void OnFrameMove(const asvk::FrameEventArgs& args) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレーム描画時の処理です.
    //!
    //! @param[in]      args        フレームイベント引数.
    //---------------------------------------------------------------------------------------------
    void OnFrameRender(const asvk::FrameEventArgs& args) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースバリアを設定します.
    //!
    //! @param[in]      commandBuffer       コマンドバッファ.
    //---------------------------------------------------------------------------------------------
    void ResourceBarrier(
        VkCommandBuffer         commandBuffer,
        VkPipelineStageFlags    srcStageFlags,
        VkPipelineStageFlags    dstStageFlags,
        VkAccessFlags           srcAccessMask,
        VkAccessFlags           dstAccessMask,
        VkImageLayout           oldLayout,
        VkImageLayout           newLayout);

    //---------------------------------------------------------------------------------------------
    //! @brief      レンダーパスを開始します.
    //!
    //! @param[in]      commandBuffer       コマンドバッファ.
    //---------------------------------------------------------------------------------------------
    void BeginRenderPass(VkCommandBuffer commandBuffer);

    //---------------------------------------------------------------------------------------------
    //! @brief      レンダーパスを終了します.
    //!
    //! @param[in]      commandBuffer       コマンドバッファ.
    //---------------------------------------------------------------------------------------------
    void EndRenderPass(VkCommandBuffer commandBuffer);
};