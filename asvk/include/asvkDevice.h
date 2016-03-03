﻿//-------------------------------------------------------------------------------------------------
// File : asvkDevice.h
// Desc : Device Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes 
//-------------------------------------------------------------------------------------------------
#include <asvkQueue.h>
#include <vector>


//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#if defined(ASVK_AUTO_LINK)
#pragma comment(lib, "vulkan-1.lib")
#endif// defined(ASVK_AUTO_LINK)


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// PhysicalDevice structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct PhysicalDevice
{
    VkPhysicalDevice                    Gpu;                //!< 物理デバイスです.
    VkPhysicalDeviceMemoryProperties    MemoryProps;        //!< 物理デバイスメモリプロパティです.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////
class DeviceMgr
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
    DeviceMgr();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~DeviceMgr();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool Init();

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void Term();

    //---------------------------------------------------------------------------------------------
    //! @brief      インスタンスを取得します.
    //!
    //! @return     インスタンスを返却します.
    //---------------------------------------------------------------------------------------------
    VkInstance GetInstance() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @return     デバイスを返却します.
    //---------------------------------------------------------------------------------------------
    VkDevice GetDevice() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      物理デバイスを取得します.
    //!
    //! @return     物理デバイスを返却します.
    //---------------------------------------------------------------------------------------------
    const std::vector<PhysicalDevice>& GetPhysicalDevice() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      グラフィックスキューを取得します.
    //!
    //! @return     グラフィックスキューを返却します.
    //---------------------------------------------------------------------------------------------
    Queue* GetGraphicsQueue();

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートキューを取得します.
    //!
    //! @return     コンピュートキューを返却します.
    //---------------------------------------------------------------------------------------------
    Queue* GetComputeQueue();

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    VkInstance                      m_Instance;         //!< インスタンスです.
    VkDevice                        m_Device;           //!< デバイスです.
    std::vector<PhysicalDevice>     m_PhysicalDevice;   //!< 物理デバイスです.
    Queue                           m_GraphicsQueue;    //!< グラフィックスキューです.
    Queue                           m_ComputeQueue;     //!< コンピュートキューです.
    VkAllocationCallbacks           m_Allocator;        //!< アロケータです.

#if ASVK_IS_DEBUG
    VkDebugReportCallbackEXT            m_DebugReporter;
    PFN_vkCreateDebugReportCallbackEXT  m_CreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT m_DestroyDebugReportCallback;
    PFN_vkDebugReportMessageEXT         m_DebugReportMessage;
#endif

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asvk
