﻿//-------------------------------------------------------------------------------------------------
// File : asvkRandom.cpp
// Desc : Random Number Generater Moudle
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkMath.h>


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Random class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Random::Random( int seed )
{
    SetSeed( seed );
}

//-------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//-------------------------------------------------------------------------------------------------
Random::Random( const Random& random )
{
    m_X = random.m_X;
    m_Y = random.m_Y;
    m_Z = random.m_Z;
    m_W = random.m_W;
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Random::~Random()
{
    /* DO_NOTHING */
}

//-------------------------------------------------------------------------------------------------
//      ランダム種を設定します.
//-------------------------------------------------------------------------------------------------
void Random::SetSeed( int seed )
{
    m_X = 123456789;
    m_Y = 362436069;
    m_Z = 521288629;
    m_W = ( seed <= 0 ) ? 88675123 : seed;
}

//-------------------------------------------------------------------------------------------------
//      乱数をuint32_t型として取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Random::GetAsU32()
{
    uint32_t t = m_X ^ ( m_X << 11 );
    m_X = m_Y;
    m_Y = m_Z;
    m_Z = m_W;
    m_W = ( m_W ^ ( m_W >> 19 ) ) ^ ( t ^ ( t >> 8 ) );
    return m_W;
}

//-------------------------------------------------------------------------------------------------
//      乱数をint型として取得します.
//-------------------------------------------------------------------------------------------------
int Random::GetAsS32()
{
    int x = GetAsU32() & 0x7fffffff;
    return x;
}

//-------------------------------------------------------------------------------------------------
//      指定された値範囲までの乱数をint型として取得します.
//-------------------------------------------------------------------------------------------------
int Random::GetAsS32( int a )
{
    // 符号ビット切り捨て.
    int x = GetAsU32() & 0x7fffffff;
    x %= a;
    return x;
}

//-------------------------------------------------------------------------------------------------
//      指定された値範囲で乱数をint型として取得します.
//-------------------------------------------------------------------------------------------------
int Random::GetAsS32( int a, int b )
{
    int x = GetAsU32() & 0x7fffffff;
    x %= ( b - a );
    x += a;
    return x;
}

//-------------------------------------------------------------------------------------------------
//      乱数をfloat型として取得します.
//-------------------------------------------------------------------------------------------------
float Random::GetAsF32()
{
    return static_cast<float>( GetAsU32() ) / UINT32_MAX;
}

//-------------------------------------------------------------------------------------------------
//		指定された値範囲までの乱数をfloat型として取得します.
//-------------------------------------------------------------------------------------------------
float Random::GetAsF32( float a )
{
    float x = GetAsF32();
    x *= a;
    return x;
}

//-------------------------------------------------------------------------------------------------
//      指定された値範囲の乱数をfloat型として取得します.
//-------------------------------------------------------------------------------------------------
float Random::GetAsF32( float a, float b )
{
    float x = GetAsF32();
    x *= ( b - a );
    x += a;
    return x;
}

//-------------------------------------------------------------------------------------------------
//      乱数をdouble型として取得します.
//-------------------------------------------------------------------------------------------------
double Random::GetAsF64()
{
    return static_cast<double>( GetAsU32() ) / UINT32_MAX;
}

//-------------------------------------------------------------------------------------------------
//      指定された値範囲までの乱数をdouble型として取得します.
//-------------------------------------------------------------------------------------------------
double Random::GetAsF64( double a )
{
    double x = GetAsF64();
    x *= a;
    return x;
}

//-------------------------------------------------------------------------------------------------
//      指定された値範囲で乱数をdouble型として取得します.
//-------------------------------------------------------------------------------------------------
double Random::GetAsF64( double a, double b )
{
    double x = GetAsF64();
    x *= ( b - a );
    x *= a;
    return x;
}

//-------------------------------------------------------------------------------------------------
//      代入演算子です.
//-------------------------------------------------------------------------------------------------
Random& Random::operator = ( const Random& random )
{
    m_X = random.m_X;
    m_Y = random.m_Y;
    m_Z = random.m_Z;
    m_W = random.m_W;
    return (*this);
}

//-------------------------------------------------------------------------------------------------
//      等価演算子です.
//-------------------------------------------------------------------------------------------------
bool Random::operator == ( const Random& random ) const
{
    return ( this == &random );
}

//-------------------------------------------------------------------------------------------------
//      非等価演算子です.
//-------------------------------------------------------------------------------------------------
bool Random::operator != ( const Random& random ) const
{
    return  ( this != &random );
}

} // namespace asvk
