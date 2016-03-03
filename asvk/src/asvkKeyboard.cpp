//-------------------------------------------------------------------------------------------------
// File : asvkKeyboard.cpp
// Desc : Keybaord Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkHid.h>
#include <Windows.h>
#include <cstring>
#include <cassert>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Keyboard::Keyboard()
: m_Index( 0 )
{ memset( m_Keys, 0, sizeof( bool ) * MAX_KEYS * 2 ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Keyboard::~Keyboard()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      キーの状態を更新します.
//-------------------------------------------------------------------------------------------------
void Keyboard::UpdateState()
{
    m_Index = 1 - m_Index;

    uint8_t keys[ MAX_KEYS ];
    GetKeyboardState( keys );
    for( uint32_t i=0; i<MAX_KEYS; ++i )
    { m_Keys[ m_Index ][ i ] = ( ( keys[ i ] & 0x80 ) != 0 ) ? true : false; }
}

//-------------------------------------------------------------------------------------------------
//      キーが押されっぱなしかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Keyboard::IsHold( const uint32_t keyCode ) const
{
    assert( keyCode < MAX_KEYS );
    auto idx = m_Index;
    auto key = ConvertKey( keyCode );
    return m_Keys[ idx ][ key ];
}

//-------------------------------------------------------------------------------------------------
//      キーが押されたかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Keyboard::IsDown( const uint32_t keyCode ) const
{
    assert( keyCode < MAX_KEYS );
    auto idx = m_Index;
    auto key = ConvertKey( keyCode );
    return m_Keys[ idx ][ key ] & ( !m_Keys[ 1 - idx ][ key ] );
}

//-------------------------------------------------------------------------------------------------
//      キーコードを変換します.
//-------------------------------------------------------------------------------------------------
uint32_t Keyboard::ConvertKey( const uint32_t keyCode ) const
{
    uint32_t key = keyCode;
    uint32_t result = 0;

    bool isAscii = ( 0x20 <= key && key <= 0x7e );
    if ( isAscii )
    { return key; }

    switch( key )
    {
        case asvk::KEY_RETURN:  { result = VK_RETURN; }     break;
        case asvk::KEY_TAB:     { result = VK_TAB; }        break;
        case asvk::KEY_ESC:     { result = VK_ESCAPE; }     break;
        case asvk::KEY_BACK:    { result = VK_BACK; }       break;
        case asvk::KEY_SHIFT:   { result = VK_SHIFT; }      break;
        case asvk::KEY_CONTROL: { result = VK_CONTROL; }    break;
        case asvk::KEY_ALT:     { result = VK_MENU; }       break;
        case asvk::KEY_F1:      { result = VK_F1; }         break;
        case asvk::KEY_F2:      { result = VK_F2; }         break;
        case asvk::KEY_F3:      { result = VK_F3; }         break;
        case asvk::KEY_F4:      { result = VK_F4; }         break;
        case asvk::KEY_F5:      { result = VK_F5; }         break;
        case asvk::KEY_F6:      { result = VK_F6; }         break;
        case asvk::KEY_F7:      { result = VK_F7; }         break;
        case asvk::KEY_F8:      { result = VK_F8; }         break;
        case asvk::KEY_F9:      { result = VK_F9; }         break;
        case asvk::KEY_F10:     { result = VK_F10; }        break;
        case asvk::KEY_F11:     { result = VK_F11; }        break;
        case asvk::KEY_F12:     { result = VK_F12; }        break;
        case asvk::KEY_UP:      { result = VK_UP; }         break;
        case asvk::KEY_DOWN:    { result = VK_DOWN; }       break;
        case asvk::KEY_LEFT:    { result = VK_LEFT; }       break;
        case asvk::KEY_RIGHT:   { result = VK_RIGHT; }      break;
        case asvk::KEY_NUM0:    { result = VK_NUMPAD0; }    break;
        case asvk::KEY_NUM1:    { result = VK_NUMPAD1; }    break;
        case asvk::KEY_NUM2:    { result = VK_NUMPAD2; }    break;
        case asvk::KEY_NUM3:    { result = VK_NUMPAD3; }    break;
        case asvk::KEY_NUM4:    { result = VK_NUMPAD4; }    break;
        case asvk::KEY_NUM5:    { result = VK_NUMPAD5; }    break;
        case asvk::KEY_NUM6:    { result = VK_NUMPAD6; }    break;
        case asvk::KEY_NUM7:    { result = VK_NUMPAD7; }    break;
        case asvk::KEY_NUM8:    { result = VK_NUMPAD8; }    break;
        case asvk::KEY_NUM9:    { result = VK_NUMPAD9; }    break;
        case asvk::KEY_INSERT:  { result = VK_INSERT; }     break;
        case asvk::KEY_DELETE:  { result = VK_DELETE; }     break;
        case asvk::KEY_HOME:    { result = VK_HOME; }       break;
        case asvk::KEY_END:     { result = VK_END; }        break;
    }

    return result;
}

} // namespace asvk
