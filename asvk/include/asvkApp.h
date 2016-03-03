﻿//-------------------------------------------------------------------------------------------------
// File : asvkApp.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkTypedef.h>
#include <asvkStepTimer.h>
#include <asvkMath.h>
#include <asvkDevice.h>
#include <asvkCommandList.h>
#include <asvkSwapChain.h>
#include <asvkRenderBuffer.h>
#include <atomic>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MouseEventArgs structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MouseEventArgs
{
    int     CursorX;            //!< カーソルのX座標です.
    int     CursorY;            //!< カーソルのY座標です.
    int     WheelDelta;         //!< ホイールの移動量です.
    bool    IsLeftDown;         //!< 左ボタンが押されているかどうか.
    bool    IsRightDown;        //!< 右ボタンが押されているかどうか.
    bool    IsMiddleDown;       //!< 中ボタンが押されているかどうか.
    bool    IsSide1Down;        //!< サイドボタン1が押されているかどうか.
    bool    IsSide2Down;        //!< サイドボタン2が押されているかどうか.

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    MouseEventArgs()
    : CursorX       (0)
    , CursorY       (0)
    , WheelDelta    (0)
    , IsLeftDown    (false)
    , IsRightDown   (false)
    , IsMiddleDown  (false)
    , IsSide1Down   (false)
    , IsSide2Down   (false)
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// KeyEventArgs structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct KeyEventArgs
{
    uint32_t    KeyCode;        //!< キーコードです.
    bool        IsKeyDown;      //!< キーが押されているかどうか.
    bool        IsAltDown;      //!< ALTキーが押されているかどうか.

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    KeyEventArgs()
    : KeyCode   (0)
    , IsKeyDown (false)
    , IsAltDown (false)
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// ResizeEventArgs structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResizeEventArgs
{
    uint32_t    Width;          //!< ウィンドウの横幅.
    uint32_t    Height;         //!< ウィンドウの縦幅.
    float       AspectRatio;    //!< ウィンドウのアスペクト比.

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    ResizeEventArgs()
    : Width         (0)
    , Height        (0)
    , AspectRatio   (0.0f)
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameEventArgs
///////////////////////////////////////////////////////////////////////////////////////////////////
struct FrameEventArgs
{
    double  UpTimeSec;      //!< アプリの起動時間です.
    double  ElapsedSec;     //!< 前のフレームからの経過時間です.
    float   FramePerSec;    //!< FPSです.
    bool    IsStopDraw;     //!< 描画停止中かどうか.

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    FrameEventArgs()
    : UpTimeSec  (0)
    , ElapsedSec (0)
    , FramePerSec(0.0f)
    , IsStopDraw (false)
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// DropEventArgs
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DropEventArgs
{
    wchar_t**   Files;          //!< ファイル名リストです. 
    uint32_t    FileCount;      //!< ファイル数です.

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    DropEventArgs()
    : Files     (nullptr)
    , FileCount (0)
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////
class App : private NonCopyable
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
    //!
    //! @param[in]      title       タイトル名です.
    //! @param[in]      width       ウィンドウの横幅です.
    //! @param[in]      height      ウィンドウの縦幅です.
    //! @param[in]      hIcon       アイコンハンドルです.
    //! @param[in]      hMenu       メニューハンドルです.
    //! @param[in]      hAccel      アクセレレータテーブルです.
    //---------------------------------------------------------------------------------------------
    App(
        LPWSTR      title,
        uint32_t    width,
        uint32_t    height,
        HICON       hIcon,
        HMENU       hMenu,
        HACCEL      hAccel);

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~App();

    //---------------------------------------------------------------------------------------------
    //! @brief      アプリケーションを実行します.
    //---------------------------------------------------------------------------------------------
    void Run();

    //---------------------------------------------------------------------------------------------
    //! @brief      フォーカスを持つかどうかチェックします.
    //---------------------------------------------------------------------------------------------
    bool HasFocus() const;

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    static constexpr uint32_t   ChainCount = 2;             //!< スワップチェイン数です.
    HINSTANCE                   m_hInst;                    //!< インスタンスハンドルです.
    HWND                        m_hWnd;                     //!< ウィンドウハンドルです.
    LPWSTR                      m_Title;                    //!< タイトル名です.
    HICON                       m_hIcon;                    //!< アイコンです.
    HMENU                       m_hMenu;                    //!< メニューです.
    HACCEL                      m_hAccel;                   //!< アクセレレータテーブルです.
    uint32_t                    m_Width;                    //!< ウィンドウの横幅です.
    uint32_t                    m_Height;                   //!< ウィンドウの縦幅です.
    float                       m_AspectRatio;              //!< ウィンドウのアスペクト比です.
    StepTimer                   m_StepTimer;                //!< タイマーです.
    uint32_t                    m_ChainIndex;               //!< 現在のチェーン番号です.
    Vector4                     m_ClearColor;               //!< クリアカラーです.
    float                       m_ClearDepth;               //!< クリア深度です.
    uint8_t                     m_ClearStencil;             //!< クリアステンシルです.
    DeviceMgr                   m_DeviceMgr;                //!< デバイスマネージャです.
    CommandList                 m_CommandList;              //!< コマンドリストです.
    SwapChain                   m_SwapChain;                //!< スワップチェインです.
    VkFormat                    m_SwapChainFormat;          //!< スワップチェインフォーマットです.
    RenderBuffer                m_DepthBuffer;              //!< 深度バッファです.
    VkFormat                    m_DepthFormat;              //!< 深度フォーマットです.
    VkFramebuffer               m_FrameBuffer[ChainCount];  //!< フレームバッファです. 
    VkViewport                  m_Viewport;                 //!< ビューポートです.
    VkRect2D                    m_Scissor;                  //!< シザー矩形です.
    VkRenderPass                m_RenderPass;               //!< デフォルトのレンダーパスです.

    //=============================================================================================
    // protected methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化時の処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool OnInit();

    //---------------------------------------------------------------------------------------------
    //! @brief      終了時の処理です.
    //---------------------------------------------------------------------------------------------
    virtual void OnTerm();

    //---------------------------------------------------------------------------------------------
    //! @brief      フレーム遷移時の処理です.
    //!
    //! @param[in]      args            フレームイベント引数.
    //---------------------------------------------------------------------------------------------
    virtual void OnFrameMove( const FrameEventArgs& args );

    //---------------------------------------------------------------------------------------------
    //! @brief      フレーム描画時の処理です.
    //!
    //! @param[in]      args            フレームイベント引数.
    //---------------------------------------------------------------------------------------------
    virtual void OnFrameRender( const FrameEventArgs& args );

    //---------------------------------------------------------------------------------------------
    //! @brief      リサイズ時の処理です.
    //!
    //! @param[in]      args            リサイズイベント引数.
    //---------------------------------------------------------------------------------------------
    virtual void OnResize( const ResizeEventArgs& args );

    //---------------------------------------------------------------------------------------------
    //! @brief      キーの処理です.
    //!
    //! @param[in]      args            キーイベント引数.
    //---------------------------------------------------------------------------------------------
    virtual void OnKey( const KeyEventArgs& args );

    //---------------------------------------------------------------------------------------------
    //! @brief      マウスの処理です.
    //!
    //! @param[in]      args            マウスイベント引数.
    //---------------------------------------------------------------------------------------------
    virtual void OnMouse( const MouseEventArgs& args );

    //---------------------------------------------------------------------------------------------
    //! @brief      ドロップ時の処理です.
    //!
    //! @param[in]      args            ドロップイベント引数.
    //---------------------------------------------------------------------------------------------
    virtual void OnDrop( const DropEventArgs& args );

    //---------------------------------------------------------------------------------------------
    //! @brief      メッセージプロシージャの処理です.
    //!
    //! @param[in]      hWnd            ウィンドウハンドル.
    //! @param[in]      msg             メッセージ.
    //! @param[in]      wp              メッセージの追加情報.
    //! @param[in]      lp              メッセージの追加情報.
    //---------------------------------------------------------------------------------------------
    virtual void OnMsgProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp );

    //---------------------------------------------------------------------------------------------
    //! @brief      描画停止フラグを設定します.
    //!
    //! @param[in]      isStopDraw          描画を停止する場合は true を指定.
    //---------------------------------------------------------------------------------------------
    void StopDraw( bool isStopDraw );

    //---------------------------------------------------------------------------------------------
    //! @brief      描画停止フラグを取得します.
    //!
    //! @retval true    描画停止中です.
    //! @retval false   描画有効です.
    //---------------------------------------------------------------------------------------------
    bool IsStopDraw() const;    
    
    //---------------------------------------------------------------------------------------------
    //! @brief      フレームカウントを取得します.
    //!
    //! @return     フレームカウントを返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t GetFrameCount() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      0.5秒ごとのFPSを取得します.
    //!
    //! @return     0.5秒ごとのFPSを返却します.
    //! @note       各フレームにおける瞬間の FPS を取得する場合は FrameEventArgs から取得してください.
    //---------------------------------------------------------------------------------------------
    float GetFramePerSec() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_FrameCount;      //!< フレームカウント.
    std::atomic<float>      m_FramePerSec;     //!< 0.5秒ごとのFPS.
    std::atomic<bool>       m_IsStopDraw;      //!< 描画停止フラグです.
    std::atomic<bool>       m_IsStandByMode;   //!< スタンバイモードかどうか?
    double                  m_LastUpdateSec;   //!< 最後の更新時間.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      アプリケーションの初期化処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool InitApp();

    //---------------------------------------------------------------------------------------------
    //! @brief      アプリケーションの終了処理です.
    //---------------------------------------------------------------------------------------------
    void TermApp();

    //---------------------------------------------------------------------------------------------
    //! @brief      ウィンドウの初期化処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool InitWnd();

    //---------------------------------------------------------------------------------------------
    //! @brief      ウィンドウの終了処理です.
    //---------------------------------------------------------------------------------------------
    void TermWnd();

    //---------------------------------------------------------------------------------------------
    //! @brief      Vulkanの初期化処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool InitVulkan();

    //---------------------------------------------------------------------------------------------
    //! @brief      Vulkanの終了処理です.
    //---------------------------------------------------------------------------------------------
    void TermVulkan();

    //---------------------------------------------------------------------------------------------
    //! @brief      メインループ処理です.
    //---------------------------------------------------------------------------------------------
    void MainLoop();

    //---------------------------------------------------------------------------------------------
    //! @brief      キーイベントを処理します.
    //!
    //! @param[in]      param       キーイベント引数です.
    //! @note       このメソッドは内部処理で, OnKey() を呼び出します.
    //!             また，このメソッドはウィンドウプロシージャからのアクセス専用メソッドですので，
    //!             アプリケーション側で呼び出しを行わないでください.
    //---------------------------------------------------------------------------------------------
    void DoKeyEvent( const KeyEventArgs& param );

    //---------------------------------------------------------------------------------------------
    //! @brief      リサイズイベントを処理します.
    //!
    //! @param[in]      param       リサイズイベント引数です.
    //! @note       このメソッドは内部処理で, OnResize() を呼び出します.
    //!             また，このメソッドはウィンドウプロシージャからのアクセス専用メソッドですので，
    //!             アプリケーション側で呼び出しを行わないでください.
    //---------------------------------------------------------------------------------------------
    void DoResizeEvent( const ResizeEventArgs& param );

    //---------------------------------------------------------------------------------------------
    //! @brief      マウスイベントを処理します.
    //!
    //! @param[in]      param       マウスイベント引数です.
    //! @note       このメソッドは内部処理で, OnMouse() を呼び出します.
    //!             また，このメソッドはウィンドウプロシージャからのアクセス専用メソッドですので，
    //!             アプリケーション側で呼び出しを行わないでください.
    //---------------------------------------------------------------------------------------------
    void DoMouseEvent( const MouseEventArgs& param );

    //---------------------------------------------------------------------------------------------
    //! @brief      ドロップイベントを処理します.
    //!
    //! @param[in]      param       ドロップイベント引数です.
    //! @note       このメソッドは内部処理で, OnDrop() を呼び出します.
    //!             また，このメソッドはウィンドウプロシージャからのアクセス専用メソッドですので，
    //!             アプリケーション側で呼び出しを行わないでください.
    //---------------------------------------------------------------------------------------------
    void DoDropEvent( const DropEventArgs& param );

    //---------------------------------------------------------------------------------------------
    //! @brief      ウィンドウプロシージャです.
    //!
    //! @param[in]      hWnd        ウィンドウハンドル.
    //! @param[in]      msg         メッセージ.
    //! @param[in]      wp          メッセージの追加情報.
    //! @param[in]      lp          メッセージの追加情報.
    //---------------------------------------------------------------------------------------------
    static LRESULT CALLBACK MsgProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp );
};

} // namespace asvk
