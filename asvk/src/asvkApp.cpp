//-------------------------------------------------------------------------------------------------
// File : asvkApp.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <list>
#include <cassert>
#include <asvkApp.h>
#include <asvkLogger.h>


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// AppList class
///////////////////////////////////////////////////////////////////////////////////////////////////
class AppList
{
public:
    typedef std::list<asvk::App*>                    app_list;
    typedef std::list<asvk::App*>::iterator          iter;
    typedef std::list<asvk::App*>::const_iterator    const_iter;

    AppList ()    { m_List.clear(); }
    ~AppList()    { m_List.clear(); }

    void        remove      ( asvk::App* app )  { m_List.remove   ( app ); }
    void        push_back   ( asvk::App* app )  { m_List.push_back( app ); }
    void        pop_back    ()                  { m_List.pop_back    (); }
    void        pop_front   ()                  { m_List.pop_front   (); }
    void        clear       ()                  { m_List.clear       (); }
    iter        begin       ()                  { return m_List.begin(); }
    const_iter  begin       () const            { return m_List.begin(); }
    iter        end         ()                  { return m_List.end  (); }
    const_iter  end         () const            { return m_List.end  (); }

private:
    app_list   m_List;
};

//-------------------------------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------------------------------
AppList             g_AppList;

//-------------------------------------------------------------------------------------------------
// Constant Values
//-------------------------------------------------------------------------------------------------
constexpr LPCWSTR WndClassName = L"asvkWindowClass";

} // namespace /* anonymous */


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
App::App
(
    LPWSTR      title,
    uint32_t    width,
    uint32_t    height,
    HICON       hIcon,
    HMENU       hMenu,
    HACCEL      hAccel
)
: m_hInst               ( nullptr )
, m_hWnd                ( nullptr )
, m_StepTimer           ()
, m_Title               ( title )
, m_hIcon               ( hIcon )
, m_hMenu               ( hMenu )
, m_hAccel              ( hAccel )
, m_Width               ( width )
, m_Height              ( height )
, m_AspectRatio         ( static_cast<float>( width ) / static_cast<float>( height ) )
, m_FrameCount          ( 0 )
, m_FramePerSec         ( 0.0f )
, m_LastUpdateSec       ( 0 )
, m_IsStopDraw          ( false )
, m_ChainIndex          ( 0 )
, m_ClearColor          ( 0.392156899f, 0.584313750f, 0.929411829f, 1.0f )
, m_ClearDepth          ( 1.0f )
, m_ClearStencil        ( 0 )
, m_DeviceMgr           ()
, m_CommandList         ()
, m_SwapChain           ()
, m_SwapChainFormat     ( VK_FORMAT_B8G8R8A8_UNORM )
, m_DepthBuffer         ()
, m_DepthFormat         ( VK_FORMAT_D24_UNORM_S8_UINT )
, m_RenderPass          ( null_handle )
{
    m_Viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) };
    m_Scissor  = { 0, 0, width, height };
    for(auto i=0u; i<ChainCount; ++i)
    { m_FrameBuffer[i] = null_handle; }
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{ TermApp(); }

//-------------------------------------------------------------------------------------------------
//      描画停止フラグを設定します.
//-------------------------------------------------------------------------------------------------
void App::StopDraw( bool isStopDraw )
{ m_IsStopDraw = isStopDraw; }

//-------------------------------------------------------------------------------------------------
//      描画停止フラグを取得します.
//-------------------------------------------------------------------------------------------------
bool App::IsStopDraw() const
{ return m_IsStopDraw; }

//-------------------------------------------------------------------------------------------------
//      フレームカウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t App::GetFrameCount() const
{ return m_FrameCount; }

//-------------------------------------------------------------------------------------------------
//      FPSを取得します.
//-------------------------------------------------------------------------------------------------
float App::GetFramePerSec() const
{ return m_FramePerSec; }


//-------------------------------------------------------------------------------------------------
//      アプリを初期化します.
//-------------------------------------------------------------------------------------------------
bool App::InitApp()
{
    // COMライブラリを初期化します.
    auto hr = CoInitialize( nullptr );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : Com Library Initialize Failed." );
        return false;
    }

    // COMライブラリのセキュリティレベルを設定.
    hr = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE,
        nullptr);
    if ( FAILED( hr ) )
    {
        ELOG( "Error : Com Librarary Initialize Secutrity Failed." );
        return false;
    }

    // ウィンドウの初期化.
    if ( !InitWnd() )
    {
        ELOG( "Error : InitWnd() Failed." );
        return false;
    }

    // Vulkanの初期化.
    if ( !InitVulkan() )
    {
        ELOG( "Error : InitVulkan() Failed." );
        return false;
    }

    // アプリケーション固有の初期化.
    if ( !OnInit() )
    {
        ELOG( "Error : OnInit() Failed." );
        return false;
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      アプリケーションの終了処理.
//-------------------------------------------------------------------------------------------------
void App::TermApp()
{
    // アプリケーション固有の終了処理.
    OnTerm();

    // Vulkanの終了処理.
    TermVulkan();

    // ウィンドウの終了処理.
    TermWnd();

    // COMライブラリの終了処理.
    CoUninitialize();
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの初期化処理.
//-------------------------------------------------------------------------------------------------
bool App::InitWnd()
{
    // インスタンスハンドルを取得.
    auto hInst = GetModuleHandle( nullptr );
    if ( hInst == nullptr )
    {
        ELOG( "Error : GetModuleHandle() Failed." );
        return false;
    }

    // アイコンなしの場合はデフォルトのアイコンを使用.
    if ( m_hIcon == nullptr )
    { m_hIcon = LoadIcon( hInst, IDI_APPLICATION ); }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEX wc;
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = m_hIcon;
    wc.hCursor          = LoadCursor( nullptr, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = WndClassName;
    wc.hIconSm          = m_hIcon;

    // ウィンドウクラスを登録します.
    if ( !RegisterClassExW( &wc ) )
    {
        ELOG( "Error : RegisterClassExW() Failed." );
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    // 矩形を設定.
    RECT rc = {  0, 0, LONG(m_Width), LONG(m_Height) };

    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成.
    m_hWnd = CreateWindowW(
        WndClassName,
        m_Title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (rc.right - rc.left),
        (rc.bottom - rc.top),
        nullptr,
        m_hMenu,
        m_hInst,
        nullptr );
    if ( m_hWnd == nullptr )
    {
        ELOG( "Error : CreateWindowW() Failed" );
        return false;
    }

    // ウィンドウを表示します.
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカスを設定.
    SetFocus( m_hWnd );

    // アプリケーション管理リストに登録.
    g_AppList.push_back( this );

    // タイマーを開始.
    m_StepTimer.Start();

    // 開始時刻を取得.
    m_LastUpdateSec = m_StepTimer.GetElapsedSec();

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの終了処理.
//-------------------------------------------------------------------------------------------------
void App::TermWnd()
{
    m_StepTimer.Stop();

    // ウィンドウクラスの登録を解除.
    if ( m_hInst != nullptr )
    { UnregisterClassW( WndClassName, m_hInst ); }

    // アクセレレータテーｂるを破棄.
    if ( m_hAccel != nullptr )
    { DestroyAcceleratorTable( m_hAccel ); }

    // アイコンを破棄.
    if ( m_hIcon != nullptr )
    { DestroyIcon( m_hIcon ); }

    // ポインタクリア.
    m_Title  = nullptr;
    m_hInst  = nullptr;
    m_hWnd   = nullptr;
    m_hIcon  = nullptr;
    m_hMenu  = nullptr;
    m_hAccel = nullptr;

    // アプリケーション管理リストから削除.
    g_AppList.remove( this );
}

//-------------------------------------------------------------------------------------------------
//      Vulkanの初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::InitVulkan()
{
    // デバイスマネージャ生成.
    if (!m_DeviceMgr.Init())
    {
        ELOG( "Error : Device::Init() Failed." );
        return false;
    }

    // コマンドリスト生成.
    if (!m_CommandList.Init(
        &m_DeviceMgr, 
        QueueType_Graphics,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        ChainCount))
    {
        ELOG( "Error : CommandList::Init() Failed." );
        return false;
    }

    // コマンドリストをリセットしておく.
    m_CommandList.Reset();
    auto cmd = m_CommandList.GetCurrentCommandBuffer();

    // スワップチェインの生成.
    {
        SwapChainDesc desc;
        desc.Width       = m_Width;
        desc.Height      = m_Height;
        desc.Format      = m_SwapChainFormat;
        desc.ColorSpace  = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        desc.BufferCount = ChainCount;
        desc.hInstance   = m_hInst;
        desc.hWnd        = m_hWnd;

        if (!m_SwapChain.Init(&m_DeviceMgr, cmd, &desc))
        {
            ELOG( "Error : SwapChain::Init() Falied." );
            return false;
        }
    }

    // 深度バッファの生成.
    {
        RenderBufferDesc desc;
        desc.Dimension   = VK_IMAGE_TYPE_2D;
        desc.Width       = m_Width;
        desc.Height      = m_Height;
        desc.Depth       = 1;
        desc.ArraySize   = 1;
        desc.Format      = m_DepthFormat;
        desc.MipLevels   = 1;
        desc.Samples     = VK_SAMPLE_COUNT_1_BIT;
        desc.Usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (!m_DepthBuffer.Init(&m_DeviceMgr, cmd, &desc))
        {
            ELOG( "Error : DepthBuffer::Init() Failed." );
            return false;
        }
    }

    // レンダーパスの生成.
    {
        VkAttachmentDescription attachments[2];
        attachments[0].format           = m_SwapChain.GetDesc().Format;
        attachments[0].samples          = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments[0].finalLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments[0].flags            = 0;

        attachments[1].format           = m_DepthBuffer.GetDesc().Format;
        attachments[1].samples          = m_DepthBuffer.GetDesc().Samples;
        attachments[1].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[1].finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[1].flags            = 0;

        VkAttachmentReference colorRef = {};
        colorRef.attachment = 0;
        colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthRef = {};
        depthRef.attachment = 1;
        depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.flags                   = 0;
        subpass.inputAttachmentCount    = 0;
        subpass.pInputAttachments       = nullptr;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorRef;
        subpass.pResolveAttachments     = nullptr;
        subpass.pDepthStencilAttachment = &depthRef;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments    = nullptr;

        VkRenderPassCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.attachmentCount    = 2;
        info.pAttachments       = attachments;
        info.subpassCount       = 1;
        info.pSubpasses         = &subpass;
        info.dependencyCount    = 0;
        info.pDependencies      = nullptr;

        auto result = vkCreateRenderPass(m_DeviceMgr.GetDevice(), &info, nullptr, &m_RenderPass);
        if (result != VK_SUCCESS)
        {
            ELOG( "Error : vkCreateRenderPass() Failed." );
            return false;
        }
    }

    // フレームバッファの生成.
    {
        VkImageView attachments[2];

        VkFramebufferCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.renderPass         = m_RenderPass;
        info.attachmentCount    = 2;
        info.pAttachments       = attachments;
        info.width              = m_Width;
        info.height             = m_Height;
        info.layers             = 1;

        for(auto i=0u; i<ChainCount; ++i)
        {
            attachments[0] = m_SwapChain.GetBuffer(i)->View;
            attachments[1] = m_DepthBuffer.GetView();

            auto result = vkCreateFramebuffer(m_DeviceMgr.GetDevice(), &info, nullptr, &m_FrameBuffer[i]);
            if ( result != VK_SUCCESS )
            {
                ELOG( "Error : vkCreateFramebuffer() Failed." );
                return false;
            }
        }
    }

    // ビューポートとシザー矩形の設定.
    {
        m_Viewport.x        = 0.0f;
        m_Viewport.y        = 0.0f;
        m_Viewport.width    = static_cast<float>(m_Width);
        m_Viewport.height   = static_cast<float>(m_Height);
        m_Viewport.minDepth = 0.0f;
        m_Viewport.maxDepth = 1.0f;

        m_Scissor.offset.x      = 0;
        m_Scissor.offset.y      = 0;
        m_Scissor.extent.width  = m_Width;
        m_Scissor.extent.height = m_Height;
    }

    // イメージレイアウトを設定しておく.
    {
        // コマンドの記録を終了.
        m_CommandList.Close();

        // キューを取得.
        auto pQueue = m_DeviceMgr.GetGraphicsQueue()->GetQueue();

        VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        // サブミット情報を設定する.
        VkSubmitInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext                  = nullptr;
        info.waitSemaphoreCount     = 0;
        info.pWaitSemaphores        = nullptr;
        info.pWaitDstStageMask      = &pipeStageFlags;
        info.commandBufferCount     = 1;
        info.pCommandBuffers        = &cmd;
        info.signalSemaphoreCount   = 0;
        info.pSignalSemaphores      = nullptr;

        // コマンド実行.
        auto result = vkQueueSubmit(pQueue, 1, &info, null_handle);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkQueueSubmit() Failed." );
            return false;
        }

        // 完了を待機.
        result = vkQueueWaitIdle(pQueue);
        if ( result != VK_SUCCESS )
        {
            ELOG( "Error : vkQueueWaitIdle() Failed." );
            return false;
        }
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      Vulkanの終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermVulkan()
{
    for(auto i=0u; i<ChainCount; ++i)
    {
        if(auto device = m_DeviceMgr.GetDevice())
        {
            vkDestroyFramebuffer(device, m_FrameBuffer[i], nullptr);
            m_FrameBuffer[i] = null_handle;
        }
    }

    if (m_RenderPass != null_handle)
    {
        vkDestroyRenderPass(m_DeviceMgr.GetDevice(), m_RenderPass, nullptr);
        m_RenderPass = null_handle;
    }

    m_DepthBuffer.Term(&m_DeviceMgr);
    m_SwapChain  .Term(&m_DeviceMgr);
    m_CommandList.Term(&m_DeviceMgr);

    m_DeviceMgr.Term();
}

//-------------------------------------------------------------------------------------------------
//      メインループ処理.
//-------------------------------------------------------------------------------------------------
void App::MainLoop()
{
    MSG msg = { 0 };

    FrameEventArgs args;
    auto frameCount = 0;

    while( WM_QUIT != msg.message )
    {
        auto hasMsg = PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE );
        if ( hasMsg )
        {
            auto ret = TranslateAccelerator( m_hWnd, m_hAccel, &msg );
            if ( 0 == ret )
            {
                TranslateMessage( &msg );
                DispatchMessage ( &msg );
            }
        }
        else
        {
            double uptimeSec;
            double absTimeSec;
            double elapsedTimeSec;
            m_StepTimer.GetValues( uptimeSec, absTimeSec, elapsedTimeSec );

            auto interval = float( uptimeSec - m_LastUpdateSec );
            if ( interval > 0.5 )
            {
                m_FramePerSec   = frameCount / interval;
                m_LastUpdateSec = uptimeSec;
                frameCount      = 0;
            }

            args.UpTimeSec   = uptimeSec;
            args.FramePerSec = 1.0f / static_cast<float>(elapsedTimeSec);
            args.ElapsedSec  = elapsedTimeSec;
            args.IsStopDraw  = m_IsStopDraw;

            OnFrameMove( args );

            if ( !IsStopDraw() )
            {
                OnFrameRender( args );
                m_FrameCount++;
            }

            frameCount++;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      アプリケーションを実行します.
//-------------------------------------------------------------------------------------------------
void App::Run()
{
    if ( InitApp() )
    { MainLoop(); }

    TermApp();
}

//-------------------------------------------------------------------------------------------------
//      キー処理.
//-------------------------------------------------------------------------------------------------
void App::DoKeyEvent( const KeyEventArgs& args )
{ OnKey( args ); }

//-------------------------------------------------------------------------------------------------
//      リサイズ処理.
//-------------------------------------------------------------------------------------------------
void App::DoResizeEvent( const ResizeEventArgs& args )
{
    // パラメータ設定.
    m_Width       = args.Width;
    m_Height      = args.Height;
    m_AspectRatio = args.AspectRatio;

    m_Viewport.width  = static_cast<float>(m_Width);
    m_Viewport.height = static_cast<float>(m_Height);

    m_Scissor.extent.width  = m_Width;
    m_Scissor.extent.height = m_Height;

    for(auto i=0u; i<ChainCount; ++i)
    {
        vkDestroyFramebuffer(m_DeviceMgr.GetDevice(), m_FrameBuffer[i], nullptr);
        m_FrameBuffer[i] = null_handle;
    }
    m_DepthBuffer.Term(&m_DeviceMgr);
    m_SwapChain  .Term(&m_DeviceMgr);

    auto cmdBuffer = m_CommandList.GetCommandBuffer(m_CommandList.GetBufferIndex());

    // スワップチェインを再生成.
    {
        SwapChainDesc desc;
        desc.Width       = m_Width;
        desc.Height      = m_Height;
        desc.Format      = m_SwapChainFormat;
        desc.ColorSpace  = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        desc.BufferCount = ChainCount;
        desc.hInstance   = m_hInst;
        desc.hWnd        = m_hWnd;

        if (!m_SwapChain.Init(&m_DeviceMgr, cmdBuffer, &desc))
        { ELOG( "Error : SwapChain::Init() Falied." ); }
    }

    // 深度バッファの生成.
    {
        RenderBufferDesc desc;
        desc.Dimension  = VK_IMAGE_TYPE_2D;
        desc.Width      = m_Width;
        desc.Height     = m_Height;
        desc.Depth      = 1;
        desc.ArraySize  = 1;
        desc.Format     = m_DepthFormat;
        desc.MipLevels  = 1;
        desc.Samples    = VK_SAMPLE_COUNT_1_BIT;
        desc.Usage      = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (!m_DepthBuffer.Init(&m_DeviceMgr, cmdBuffer, &desc))
        { ELOG("Error : DepthBuffer::Init() Failed."); }
    }

    // フレームバッファの生成.
    {
        VkImageView attachments[2];

        VkFramebufferCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.renderPass         = m_RenderPass;
        info.attachmentCount    = 2;
        info.pAttachments       = attachments;
        info.width              = m_Width;
        info.height             = m_Height;
        info.layers             = 1;

        for(auto i=0u; i<ChainCount; ++i)
        {
            attachments[0] = m_SwapChain.GetBuffer(i)->View;
            attachments[1] = m_DepthBuffer.GetView();

            auto result = vkCreateFramebuffer(m_DeviceMgr.GetDevice(), &info, nullptr, &m_FrameBuffer[i]);
            if ( result != VK_SUCCESS )
            { ELOG( "Error : vkCreateFramebuffer() Failed." ); }
        }
    }

    OnResize( args );
}

//-------------------------------------------------------------------------------------------------
//      マウス処理.
//-------------------------------------------------------------------------------------------------
void App::DoMouseEvent( const MouseEventArgs& args )
{ OnMouse( args ); }

//-------------------------------------------------------------------------------------------------
//      ドロップ時の処理.
//-------------------------------------------------------------------------------------------------
void App::DoDropEvent( const DropEventArgs& args )
{ OnDrop( args ); }

//-------------------------------------------------------------------------------------------------
//      ウィンドウプロシージャです.
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK App::MsgProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
    if ( ( msg == WM_KEYDOWN )
      || ( msg == WM_SYSKEYDOWN )
      || ( msg == WM_KEYUP )
      || ( msg == WM_SYSKEYUP ) )
    {
        auto isKeyDown = ( msg == WM_KEYDOWN ) || ( msg == WM_SYSKEYDOWN );
        uint32_t mask = ( 1 << 29 );
        auto isAltDown = ( ( lp & mask ) != 0 );

        KeyEventArgs args;
        args.KeyCode   = uint32_t( wp );
        args.IsAltDown = isAltDown;
        args.IsKeyDown = isKeyDown;

        for( auto app : g_AppList )
        { app->DoKeyEvent( args ); }
    }

    const UINT OLD_WM_MOUSEWHEEL = 0x020A;

    if ( ( msg == WM_LBUTTONDOWN ) 
      || ( msg == WM_LBUTTONUP )
      || ( msg == WM_LBUTTONDBLCLK )
      || ( msg == WM_MBUTTONDOWN )
      || ( msg == WM_MBUTTONUP )
      || ( msg == WM_MBUTTONDBLCLK )
      || ( msg == WM_RBUTTONDOWN )
      || ( msg == WM_RBUTTONUP )
      || ( msg == WM_RBUTTONDBLCLK )
      || ( msg == WM_XBUTTONDOWN )
      || ( msg == WM_XBUTTONUP )
      || ( msg == WM_XBUTTONDBLCLK )
      || ( msg == WM_MOUSEHWHEEL )
      || ( msg == WM_MOUSEMOVE )
      || ( msg == OLD_WM_MOUSEWHEEL ) )
    {
        auto x = static_cast<int>( (int16_t)LOWORD( lp ) );
        auto y = static_cast<int>( (int16_t)HIWORD( lp ) );

        auto wheelDelta = 0;
        POINT pt = { x, y };
        ScreenToClient( hWnd, &pt );
        x = pt.x;
        y = pt.y;

        if ( ( msg == WM_MOUSEHWHEEL ) || ( msg == OLD_WM_MOUSEWHEEL ) )
        {
            wheelDelta += static_cast<int16_t>( wp );
        }

        auto state = LOWORD( wp );
        auto isDownL  = ( ( state & MK_LBUTTON )  != 0 );
        auto isDownR  = ( ( state & MK_RBUTTON )  != 0 );
        auto isDownM  = ( ( state & MK_MBUTTON )  != 0 );
        auto isDownX1 = ( ( state & MK_XBUTTON1 ) != 0 );
        auto isDownX2 = ( ( state & MK_XBUTTON2 ) != 0 );

        MouseEventArgs args;
        args.CursorX      = x;
        args.CursorY      = y;
        args.IsLeftDown   = isDownL;
        args.IsMiddleDown = isDownM;
        args.IsRightDown  = isDownR;
        args.IsSide1Down  = isDownX1;
        args.IsSide2Down  = isDownX2;

        for( auto app : g_AppList )
        { app->DoMouseEvent( args ); }
    }

    switch( msg )
    {
    case WM_CREATE:
        { DragAcceptFiles( hWnd, TRUE ); }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
        }
        break;

    case WM_DESTROY:
        { PostQuitMessage( 0 ); }
        break;

    case WM_SIZE:
        {
            auto w = static_cast<uint32_t>( LOWORD( lp ) );
            auto h = static_cast<uint32_t>( HIWORD( lp ) );

            ResizeEventArgs args;
            args.Width  = asvk::Max( w, uint32_t( 1 ) );
            args.Height = asvk::Max( h, uint32_t( 1 ) );
            args.AspectRatio = float( args.Width ) / float( args.Height );

            for( auto app : g_AppList )
            { app->DoResizeEvent( args ); }
        }
        break;

    case WM_DROPFILES:
        {
            auto fileCount = DragQueryFileW( (HDROP)wp, 0xFFFFFFFF, nullptr, 0 );
            if (fileCount <= 0)
            { break; }

            DropEventArgs args;
            args.Files = new wchar_t* [fileCount];
            args.FileCount = fileCount;
            for( uint32_t i=0; i<fileCount; ++i )
            {
                wchar_t* file = new wchar_t [ MAX_PATH ];
                DragQueryFileW( (HDROP)wp, i, file, MAX_PATH );
                args.Files[i] = file;
            }

            for ( auto app : g_AppList )
            { app->DoDropEvent( args ); }

            for( uint32_t i=0; i<fileCount; ++i )
            { SafeDeleteArray( args.Files[i] ); }
            SafeDeleteArray( args.Files );

            DragFinish( (HDROP)wp );
        }
        break;
    }

    for( auto app : g_AppList )
    { app->OnMsgProc( hWnd, msg, wp, lp ); }

    return DefWindowProc( hWnd, msg, wp, lp );
}

//-------------------------------------------------------------------------------------------------
//      アプリケーション固有の初期化時の処理.
//-------------------------------------------------------------------------------------------------
bool App::OnInit()
{
    /* DO_NOTHING */
    return true;
}

//-------------------------------------------------------------------------------------------------
//      アプリケーション固有の終了時の処理.
//-------------------------------------------------------------------------------------------------
void App::OnTerm()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      フレーム遷移時の処理.
//-------------------------------------------------------------------------------------------------
void App::OnFrameMove( const FrameEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      フレーム描画時の処理.
//-------------------------------------------------------------------------------------------------
void App::OnFrameRender( const FrameEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      リサイズ時の処理
//-------------------------------------------------------------------------------------------------
void App::OnResize( const ResizeEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      キーの処理
//-------------------------------------------------------------------------------------------------
void App::OnKey( const KeyEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      マウスの処理.
//-------------------------------------------------------------------------------------------------
void App::OnMouse( const MouseEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      ドロップ時の処理.
//-------------------------------------------------------------------------------------------------
void App::OnDrop( const DropEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      メッセージプロシージャです.
//-------------------------------------------------------------------------------------------------
void App::OnMsgProc( HWND, UINT, WPARAM, LPARAM )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      フォーカスを持つかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool App::HasFocus() const
{ return ( GetActiveWindow() == m_hWnd ); }

} // namespace asvk
