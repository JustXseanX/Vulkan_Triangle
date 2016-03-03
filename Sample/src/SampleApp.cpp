//-------------------------------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <SampleApp.h>
#include <cassert>
#include <asvkLogger.h>
#include <asvkBlob.h>
#include <asvkMisc.h>


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    asvk::Vector3 Position;     //!< 位置座標です.
    asvk::Vector2 TexCoord;     //!< テクスチャ座標です.
    asvk::Vector4 Color;        //!< 頂点カラーです.
};

} // namespace /* anonymous */


///////////////////////////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
SampleApp::SampleApp()
: asvk::App(L"SampleApp", 960, 540, nullptr, nullptr, nullptr)
, m_pQueue          ( nullptr )
, m_PipelineLayout  ( VK_NULL_HANDLE )
, m_PipelineCache   ( VK_NULL_HANDLE )
, m_Pipeline        ( VK_NULL_HANDLE )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
SampleApp::~SampleApp()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化時の処理です.
//-------------------------------------------------------------------------------------------------
bool SampleApp::OnInit()
{
    // グラフィックスキューを取得します.
    m_pQueue = m_DeviceMgr.GetGraphicsQueue();
    assert(m_pQueue != nullptr);

    // デバイス取得.
    auto device = m_DeviceMgr.GetDevice();

    // メッシュの初期化.
    {
        // 頂点データ.
        Vertex vertices[3] = {
            { asvk::Vector3(-0.5f, -0.75f, 0.0f), asvk::Vector2(0.0f, 0.0f), asvk::Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
            { asvk::Vector3( 0.0f,  0.75f, 0.0f), asvk::Vector2(0.5f, 1.0f), asvk::Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
            { asvk::Vector3( 0.5f, -0.75f, 0.0f), asvk::Vector2(1.0f, 0.0f), asvk::Vector4(0.0f, 0.0f, 1.0f, 1.0f) }
        };

        // バッファ生成情報の設定.
        VkBufferCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.pNext                  = nullptr;
        info.usage                  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        info.size                   = sizeof(Vertex) * 3;
        info.queueFamilyIndexCount  = 0;
        info.pQueueFamilyIndices    = nullptr;
        info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
        info.flags                  = 0;

        // 頂点バッファ生成.
        auto result = vkCreateBuffer(device, &info, nullptr, &m_Mesh.Buffer);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkCreateBuffer() Failed." );
            return false;
        }

        // メモリ要件を取得.
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device, m_Mesh.Buffer, &requirements);

        // メモリ割り当て情報を設定.
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext             = nullptr;
        allocInfo.memoryTypeIndex   = 0;
        allocInfo.allocationSize    = requirements.size;

        // メモリを確保.
        result = vkAllocateMemory(device, &allocInfo, nullptr, &m_Mesh.Memory);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkAllocateMemory() Failed." );
            return false;
        }

        // マップする.
        uint8_t* pData = nullptr;
        result = vkMapMemory(device, m_Mesh.Memory, 0, requirements.size, 0, reinterpret_cast<void**>(&pData));
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkMapMemory() Failed." );
            return false;
        }

        // 頂点データを設定.
        memcpy(pData, vertices, sizeof(vertices));

        // アンマップする.
        vkUnmapMemory(device, m_Mesh.Memory);

        // メモリをバッファに関連付ける.
        result = vkBindBufferMemory(device, m_Mesh.Buffer, m_Mesh.Memory, 0);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkBindBufferMemory() Failed." );
            return false;
        }

        // バインディング情報の設定.
        m_Mesh.Bindings.binding     = 0;
        m_Mesh.Bindings.inputRate   = VK_VERTEX_INPUT_RATE_VERTEX;
        m_Mesh.Bindings.stride      = sizeof(Vertex);

        uint32_t offset = 0;

        // 頂点属性の設定.
        m_Mesh.Attributes[0].binding    = 0;
        m_Mesh.Attributes[0].location   = 0;
        m_Mesh.Attributes[0].format     = VK_FORMAT_R32G32B32_SFLOAT;
        m_Mesh.Attributes[0].offset     = offset;
        offset += sizeof(asvk::Vector3);

        m_Mesh.Attributes[1].binding    = 0;
        m_Mesh.Attributes[1].location   = 1;
        m_Mesh.Attributes[1].format     = VK_FORMAT_R32G32_SFLOAT;
        m_Mesh.Attributes[1].offset     = offset;
        offset += sizeof(asvk::Vector2);

        m_Mesh.Attributes[2].binding    = 0;
        m_Mesh.Attributes[2].location   = 2;
        m_Mesh.Attributes[2].format     = VK_FORMAT_R32G32B32A32_SFLOAT;
        m_Mesh.Attributes[2].offset     = offset;
        offset += sizeof(asvk::Vector4);
    }

    // パイプラインレイアウトの生成.
    {
        VkPipelineLayoutCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.pNext                  = nullptr;
        info.pushConstantRangeCount = 0;
        info.pPushConstantRanges    = nullptr;
        info.setLayoutCount         = 0;
        info.pSetLayouts            = nullptr;

        auto result = vkCreatePipelineLayout(device, &info, nullptr, &m_PipelineLayout);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkCreatePipelineLayout() Failed." );
            return false;
        }
    }

    // パイプラインキャッシュの生成.
    {
        VkPipelineCacheCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.initialDataSize    = 0;
        info.pInitialData       = nullptr;

        auto result = vkCreatePipelineCache(device, &info, nullptr, &m_PipelineCache);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkCreatePipelineCache() Failed." );
            return false;
        }
    }

    VkShaderModule vs = VK_NULL_HANDLE;
    VkShaderModule fs = VK_NULL_HANDLE;

    // 頂点シェーダの生成.
    {
        asvk::RefPtr<asvk::IBlob> blob;
        std::wstring path;

        if (!asvk::SearchFilePath(L"/res/SimpleVS.spv", path))
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        if (!asvk::ReadFileToBlob(path.c_str(), blob.GetAddress()))
        {
            ELOG( "Error : Vertex Shader Load Failed." );
            return false;
        }

        VkShaderModuleCreateInfo info = {};
        info.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pNext      = nullptr;
        info.flags      = 0;
        info.codeSize   = blob->GetBufferSize();
        info.pCode      = reinterpret_cast<uint32_t*>(blob->GetBufferPointer());

        auto result = vkCreateShaderModule(device, &info, nullptr, &vs);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkCreateShaderModule() Failed." );
            return false;
        }
    }

    // ピクセルシェーダの生成.
    {
        asvk::RefPtr<asvk::IBlob> blob;
        std::wstring path;

        if (!asvk::SearchFilePath(L"res/SimpleFS.spv", path))
        {
            ELOG( "Error : File Not Found." );
            vkDestroyShaderModule(device, vs, nullptr);
            return false;
        }
     
        if (!asvk::ReadFileToBlob(path.c_str(), blob.GetAddress()))
        {
            ELOG( "Error : Fragment Shader Load Failed." );
            vkDestroyShaderModule(device, vs, nullptr);
            return false;
        }

        VkShaderModuleCreateInfo info = {};
        info.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pNext      = nullptr;
        info.flags      = 0;
        info.codeSize   = blob->GetBufferSize();
        info.pCode      = reinterpret_cast<uint32_t*>(blob->GetBufferPointer());

        auto result = vkCreateShaderModule(device, &info, nullptr, &fs);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkCreateShaderModule() Failed." );
            vkDestroyShaderModule(device, vs, nullptr);
            return false;
        }
    }

    // パイプラインの生成.
    {
        // シェーダステージの設定.
        VkPipelineShaderStageCreateInfo stageInfo[2];
        stageInfo[0].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo[0].pNext               = nullptr;
        stageInfo[0].flags               = 0;
        stageInfo[0].stage               = VK_SHADER_STAGE_VERTEX_BIT;
        stageInfo[0].module              = vs;
        stageInfo[0].pName               = "main";
        stageInfo[0].pSpecializationInfo = nullptr;

        stageInfo[1].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo[1].pNext               = nullptr;
        stageInfo[1].flags               = 0;
        stageInfo[1].stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
        stageInfo[1].module              = fs;
        stageInfo[1].pName               = "main";
        stageInfo[1].pSpecializationInfo = nullptr;

        // 頂点入力の設定.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.pNext                           = nullptr;
        vertexInputInfo.flags                           = 0;
        vertexInputInfo.vertexBindingDescriptionCount   = 1;
        vertexInputInfo.pVertexBindingDescriptions      = &m_Mesh.Bindings;
        vertexInputInfo.vertexAttributeDescriptionCount = 3;
        vertexInputInfo.pVertexAttributeDescriptions    = m_Mesh.Attributes;

        // 入力アセンブリの設定.
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
        inputAssemblyInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.pNext                     = nullptr;
        inputAssemblyInfo.flags                     = 0;
        inputAssemblyInfo.topology                  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable    = VK_FALSE;

        // ビューポートステートの設定.
        VkPipelineViewportStateCreateInfo viewportInfo = {};
        viewportInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.pNext          = nullptr;
        viewportInfo.flags          = 0;
        viewportInfo.viewportCount  = 1;
        viewportInfo.pViewports     = &m_Viewport;
        viewportInfo.scissorCount   = 1;
        viewportInfo.pScissors      = &m_Scissor;

        // ラスタライザ―ステートの設定.
        VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
        rasterizationInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.pNext                     = nullptr;
        rasterizationInfo.flags                     = 0;
        rasterizationInfo.depthClampEnable          = VK_FALSE;
        rasterizationInfo.rasterizerDiscardEnable   = VK_FALSE;
        rasterizationInfo.polygonMode               = VK_POLYGON_MODE_FILL;
        rasterizationInfo.cullMode                  = VK_CULL_MODE_BACK_BIT;
        rasterizationInfo.frontFace                 = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationInfo.depthBiasEnable           = VK_FALSE;
        rasterizationInfo.depthBiasConstantFactor   = 0.0f;
        rasterizationInfo.depthBiasClamp            = 0.0f;
        rasterizationInfo.depthBiasSlopeFactor      = 0.0f;
        rasterizationInfo.lineWidth                 = 1.0f;

        // マルチサンプルステートの設定.
        VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
        multisampleInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.pNext                   = nullptr;
        multisampleInfo.flags                   = 0;
        multisampleInfo.rasterizationSamples    = VK_SAMPLE_COUNT_1_BIT;
        multisampleInfo.sampleShadingEnable     = VK_FALSE;
        multisampleInfo.minSampleShading        = 0.0f;
        multisampleInfo.pSampleMask             = nullptr;
        multisampleInfo.alphaToCoverageEnable   = VK_FALSE;
        multisampleInfo.alphaToOneEnable        = VK_FALSE;

        // ステンシル操作の設定.
        VkStencilOpState stencilOp = {};
        stencilOp.failOp        = VK_STENCIL_OP_KEEP;
        stencilOp.passOp        = VK_STENCIL_OP_KEEP;
        stencilOp.depthFailOp   = VK_STENCIL_OP_KEEP;
        stencilOp.compareOp     = VK_COMPARE_OP_NEVER;
        stencilOp.compareMask   = 0;
        stencilOp.writeMask     = 0;
        stencilOp.reference     = 0;

        // 深度・ステンシルステートの設定.
        VkPipelineDepthStencilStateCreateInfo depthInfo = {};
        depthInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthInfo.pNext                 = nullptr;
        depthInfo.flags                 = 0;
        depthInfo.depthTestEnable       = VK_TRUE;
        depthInfo.depthWriteEnable      = VK_TRUE;
        depthInfo.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthInfo.depthBoundsTestEnable = VK_FALSE;
        depthInfo.stencilTestEnable     = VK_FALSE;
        depthInfo.front                 = stencilOp;
        depthInfo.back                  = stencilOp;
        depthInfo.minDepthBounds        = 0.0f;
        depthInfo.maxDepthBounds        = 1.0f;

        // ブレンドアタッチメントの設定.
        VkPipelineColorBlendAttachmentState blendState = {};
        blendState.blendEnable          = VK_FALSE;
        blendState.srcColorBlendFactor  = VK_BLEND_FACTOR_ZERO;
        blendState.dstColorBlendFactor  = VK_BLEND_FACTOR_ZERO;
        blendState.colorBlendOp         = VK_BLEND_OP_ADD;
        blendState.srcAlphaBlendFactor  = VK_BLEND_FACTOR_ZERO;
        blendState.dstAlphaBlendFactor  = VK_BLEND_FACTOR_ZERO;
        blendState.alphaBlendOp         = VK_BLEND_OP_ADD;
        blendState.colorWriteMask       = 0xf;

        // ブレンドステートの設定.
        VkPipelineColorBlendStateCreateInfo blendInfo = {};
        blendInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendInfo.pNext             = nullptr;
        blendInfo.flags             = 0;
        blendInfo.logicOpEnable     = VK_FALSE;
        blendInfo.logicOp           = VK_LOGIC_OP_CLEAR;
        blendInfo.attachmentCount   = 1;
        blendInfo.pAttachments      = &blendState;
        blendInfo.blendConstants[0] = 0.0f;
        blendInfo.blendConstants[1] = 0.0f;
        blendInfo.blendConstants[2] = 0.0f;
        blendInfo.blendConstants[3] = 0.0f;

        // 動的ステート.
        VkDynamicState dynamicState[2] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        // 動的ステートの生成.
        VkPipelineDynamicStateCreateInfo dynamicInfo = {};
        dynamicInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicInfo.pNext               = nullptr;
        dynamicInfo.flags               = 0;
        dynamicInfo.dynamicStateCount   = 2;
        dynamicInfo.pDynamicStates      = dynamicState;

        // グラフィックスパイプラインの設定.
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext                  = nullptr;
        pipelineInfo.stageCount             = 2;
        pipelineInfo.pStages                = stageInfo;
        pipelineInfo.pVertexInputState      = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState    = &inputAssemblyInfo;
        pipelineInfo.pTessellationState     = nullptr;
        pipelineInfo.pViewportState         = &viewportInfo;
        pipelineInfo.pRasterizationState    = &rasterizationInfo;
        pipelineInfo.pMultisampleState      = &multisampleInfo;
        pipelineInfo.pDepthStencilState     = nullptr;
        pipelineInfo.pColorBlendState       = &blendInfo;
        pipelineInfo.pDynamicState          = &dynamicInfo;
        pipelineInfo.layout                 = m_PipelineLayout;
        pipelineInfo.renderPass             = m_RenderPass;
        pipelineInfo.subpass                = 0;
        pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex      = 0;

        // グラフィックスパイプラインの生成.
        auto result = vkCreateGraphicsPipelines(device, m_PipelineCache, 1, &pipelineInfo, nullptr, &m_Pipeline);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkCreateGraphicsPipelines() Failed." );
            vkDestroyShaderModule(device, vs, nullptr);
            vkDestroyShaderModule(device, fs, nullptr);
            return false;
        }

        // 不要なオブジェクトを破棄.
        vkDestroyShaderModule(device, vs, nullptr);
        vkDestroyShaderModule(device, fs, nullptr);
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了時の処理です.
//-------------------------------------------------------------------------------------------------
void SampleApp::OnTerm()
{
    auto device = m_DeviceMgr.GetDevice();
    assert(device != nullptr);

    // メッシュの破棄処理.
    {
        if (m_Mesh.Memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, m_Mesh.Memory, nullptr); 
            m_Mesh.Memory = VK_NULL_HANDLE;
        }

        if (m_Mesh.Buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, m_Mesh.Buffer, nullptr);
            m_Mesh.Buffer = VK_NULL_HANDLE;
        }

        memset(&m_Mesh.Bindings,   0, sizeof(m_Mesh.Bindings));
        memset(&m_Mesh.Attributes, 0, sizeof(m_Mesh.Attributes));
    }

    if (m_Pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
    }

    if (m_PipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    if (m_PipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(device, m_PipelineCache, nullptr);
        m_PipelineCache = VK_NULL_HANDLE;
    }

    m_pQueue = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      フレーム遷移時の処理です.
//-------------------------------------------------------------------------------------------------
void SampleApp::OnFrameMove(const asvk::FrameEventArgs& args)
{
    ASVK_UNUSED(args);
}

//-------------------------------------------------------------------------------------------------
//      フレーム描画時の処理です.
//-------------------------------------------------------------------------------------------------
void SampleApp::OnFrameRender(const asvk::FrameEventArgs& args)
{
    ASVK_UNUSED(args);

    // コマンドの記録を開始.
    m_CommandList.Reset();

    // 現在のコマンドリストを取得.
    auto cmd = m_CommandList.GetCurrentCommandBuffer();

    VkDeviceSize offset = 0;

    // 描画処理.
    {
        // Present ---> Attachment へのパイプラインバリアを張る.
        ResourceBarrier(
            cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // レンダーパスを開始.
        BeginRenderPass(cmd);

        // パイプラインをバインドする.
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

        // ビューポート・シザー矩形の設定.
        vkCmdSetViewport(cmd, 0, 1, &m_Viewport);
        vkCmdSetScissor (cmd, 0, 1, &m_Scissor);

        // 頂点バッファの設定.
        vkCmdBindVertexBuffers(cmd, 0, 1, &m_Mesh.Buffer, &offset);

        // 描画コマンドを積む.
        vkCmdDraw(cmd, 3, 1, 0, 0);

        // レンダーパスを終了.
        EndRenderPass(cmd);

        // Attachment ---> Present へのパイプラインバリアを張る.
        ResourceBarrier(
            cmd,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    // コマンドの記録を終了.
    m_CommandList.Close();

    // コマンドを実行.
    m_pQueue->Execute(1, &cmd);

    // コマンドの完了を待機.
    m_pQueue->Wait(TimeOut);

    // 画面に表示.
    m_SwapChain.Present(TimeOut);
}

//-------------------------------------------------------------------------------------------------
//      リソースバリアを設定します.
//-------------------------------------------------------------------------------------------------
void SampleApp::ResourceBarrier
(
    VkCommandBuffer         commandBuffer,
    VkPipelineStageFlags    srcStageFlags,
    VkPipelineStageFlags    dstStageFlags,
    VkAccessFlags           srcAccessMask,
    VkAccessFlags           dstAccessMask,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout
)
{
    VkImageMemoryBarrier barrier;
    barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcAccessMask       = srcAccessMask;
    barrier.dstAccessMask       = dstAccessMask;
    barrier.oldLayout           = oldLayout;
    barrier.newLayout           = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange    = m_SwapChain.GetRange();
    barrier.image               = m_SwapChain.GetCurrentBuffer()->Image;

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageFlags,
        dstStageFlags,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);
}

//-------------------------------------------------------------------------------------------------
//      レンダーパスを開始します.
//-------------------------------------------------------------------------------------------------
void SampleApp::BeginRenderPass(VkCommandBuffer commandBuffer)
{
    // カラーバッファのクリアカラーの設定.
    VkClearColorValue clearColor = {};
    clearColor.float32[0] = 0.392156899f;
    clearColor.float32[1] = 0.584313750f;
    clearColor.float32[2] = 0.929411829f;
    clearColor.float32[3] = 1.0f;

    // 深度・ステンシルのクリア値の設定.
    VkClearDepthStencilValue clearDepthStencil = {};
    clearDepthStencil.depth   = 1.0f;
    clearDepthStencil.stencil = 0;

    VkClearValue clearValues[2];
    clearValues[0].color        = clearColor;
    clearValues[1].depthStencil = clearDepthStencil;

    auto idx = m_SwapChain.GetBufferIndex();
    auto frameBuffer = m_FrameBuffer[idx];

    VkRenderPassBeginInfo info = {};
    info.sType                      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.pNext                      = nullptr;
    info.renderPass                 = m_RenderPass;
    info.framebuffer                = frameBuffer;
    info.renderArea.offset.x        = 0;
    info.renderArea.offset.y        = 0;
    info.renderArea.extent.width    = m_Width;
    info.renderArea.extent.height   = m_Height;
    info.clearValueCount            = 2;
    info.pClearValues               = clearValues;

    // レンダーパス開始コマンドを積む.
    vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
}

//-------------------------------------------------------------------------------------------------
//      レンダーパスを終了します.
//-------------------------------------------------------------------------------------------------
void SampleApp::EndRenderPass(VkCommandBuffer commandBuffer)
{
    // レンダーパス終了コマンドを積む.
    vkCmdEndRenderPass(commandBuffer);
}