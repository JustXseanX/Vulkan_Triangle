﻿//-------------------------------------------------------------------------------------------------
// File : asvkRenderBuffer.h
// Desc : Render Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkDevice.h>
#include <asvkResource.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderBufferDesc structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct RenderBufferDesc
{
    VkImageType             Dimension;      //!< 次元です.
    uint32_t                Width;          //!< 横幅です.
    uint32_t                Height;         //!< 縦幅です.
    uint32_t                Depth;          //!< 奥行きです.
    uint32_t                ArraySize;      //!< 配列サイズです.
    VkFormat                Format;         //!< フォーマットです.
    uint32_t                MipLevels;      //!< ミップレベルです.
    VkSampleCountFlagBits   Samples;        //!< サンプルカウントです.
    VkImageUsageFlags       Usage;          //!< 使用用途です.

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    RenderBufferDesc()
    : Dimension (VK_IMAGE_TYPE_1D)
    , Width     (0)
    , Height    (1)
    , Depth     (1)
    , ArraySize (1)
    , Format    (VK_FORMAT_UNDEFINED)
    , Samples   (VK_SAMPLE_COUNT_1_BIT)
    , Usage     (0)
    { /* DO_NOTHING */ }
};

	
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////
class RenderBuffer
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
    RenderBuffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~RenderBuffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理です.
    //!
    //! @param[in]      pDeviceMgr      デバイスマネージャです.
    //! @param[in]      commandBuffer   コマンドバッファです.
    //! @param[in]      pDesc           構成設定です.
    //---------------------------------------------------------------------------------------------
    bool Init(
        DeviceMgr*              pDeviceMgr,
        VkCommandBuffer         commandBuffer,
        const RenderBufferDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理です.
    //!
    //! @param[in]      pDeviceMgr      デバイスマネージャです.
    //---------------------------------------------------------------------------------------------
    void Term(DeviceMgr* pDeviceMgr);

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    const RenderBufferDesc& GetDesc() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //!
    //! @return     リソースを返却します.
    //---------------------------------------------------------------------------------------------
    ImageResource* GetResource();

    //---------------------------------------------------------------------------------------------
    //! @brief      イメージビューを取得します.
    //!
    //! @return     イメージビューを返却します.
    //---------------------------------------------------------------------------------------------
    VkImageView GetView() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      イメージサブリソースレンジを取得します.
    //!
    //! @return     イメージサブリソースレンジを返却します.
    //---------------------------------------------------------------------------------------------
    VkImageSubresourceRange GetRange() const;

private:
    //=============================================================================================
    // privavte varaibles.
    //=============================================================================================
    RenderBufferDesc        m_Desc;       //!< 構成設定です.
    ImageResource           m_Resource;   //!< リソースです.
    VkImageView             m_View;       //!< イメージビューです.
    VkImageSubresourceRange m_Range;      //!< イメージサブリソースレンジです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asvk
