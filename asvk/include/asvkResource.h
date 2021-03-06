﻿//-------------------------------------------------------------------------------------------------
// File : asvkResource.h
// Desc : Resource Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkTypedef.h>
#include <vulkan/vulkan.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImageResource class
///////////////////////////////////////////////////////////////////////////////////////////////////
class ImageResource : NonCopyable
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
    ImageResource();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~ImageResource();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      device      デバイスです.
    //! @param[in]      props       物理デバイスメモリプロパティです.
    //! @param[in]      pInfo       イメージ生成情報です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool Init(
        VkDevice                            device,
        VkPhysicalDeviceMemoryProperties    props,
        const VkImageCreateInfo*            pInfo);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //!
    //! @param[in]      device      デバイスです.
    //---------------------------------------------------------------------------------------------
    void Term(VkDevice device);

    //---------------------------------------------------------------------------------------------
    //! @brief      マップします.
    //!
    //! @param[in]      device      デバイスです.
    //! @param[in]      offset      オフセットです.
    //! @param[in]      size        サイズです.
    //! @param[in]      flags       メモリマップフラグです.
    //! @param[out]     ppData      バッファポインタの格納先.
    //! @retval true    マップに成功.
    //! @retval false   マップに失敗.
    //---------------------------------------------------------------------------------------------
    bool Map(
        VkDevice            device,
        VkDeviceSize        offset,
        VkDeviceSize        size,
        VkMemoryMapFlags    flags,
        void**              ppData) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      アンマップします.
    //!
    //! @param[in]      device      デバイスです.
    //---------------------------------------------------------------------------------------------
    void Unmap(VkDevice device) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスメモリを取得します.
    //!
    //! @return     デバイスメモリを返却します.
    //---------------------------------------------------------------------------------------------
    VkDeviceMemory GetMemory() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      イメージを取得します.
    //!
    //! @return     イメージを返却します.
    //---------------------------------------------------------------------------------------------
    VkImage GetImage() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    VkImage         m_Resource;     //!< イメージです.
    VkDeviceMemory  m_Memory;       //!< デバイスメモリです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// BufferResource class
///////////////////////////////////////////////////////////////////////////////////////////////////
class BufferResource : NonCopyable
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
    BufferResource();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~BufferResource();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      device          デバイスです.
    //! @param[in]      pInfo           バッファ生成情報です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool Init(VkDevice device, const VkBufferCreateInfo* pInfo);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //!
    //! @param[in]      device          デバイスです.
    //---------------------------------------------------------------------------------------------
    void Term(VkDevice device);

    //---------------------------------------------------------------------------------------------
    //! @brief      マップします.
    //!
    //! @param[in]      device          デバイスです.
    //! @param[in]      offset          オフセットです.
    //! @param[in]      size            サイズです.
    //! @param[in]      flags           メモリマップフラグです.
    //! @param[out]     ppData          バッファポインタの格納先です.
    //! @retval true    マップに成功.
    //! @retval false   マップに失敗.
    //---------------------------------------------------------------------------------------------
    bool Map(
        VkDevice            device,
        VkDeviceSize        offset,
        VkDeviceSize        size,
        VkMemoryMapFlags    flags,
        void**              ppData) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      アンマップします.
    //!
    //! @param[in]      device          デバイスです.
    //---------------------------------------------------------------------------------------------
    void Unmap(VkDevice device) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスメモリを取得します.
    //!
    //! @return     デバイスメモリを返却します.
    //---------------------------------------------------------------------------------------------
    VkDeviceMemory GetMemory() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファを取得します.
    //!
    //! @return     バッファを返却します.
    //---------------------------------------------------------------------------------------------
    VkBuffer GetBuffer() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    VkBuffer        m_Resource;     //!< バッファです.
    VkDeviceMemory  m_Memory;       //!< デバイスメモリです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asvk
