//-------------------------------------------------------------------------------------------------
// File : asdxResTexture.cpp
// Desc : Resource Texture Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkResTexture.h>
#include <asvkLogger.h>
#include <asvkMisc.h>
#include "formats/asvkResTGA.h"
#include "formats/asvkResDDS.h"
#include "formats/asvkResHDR.h"
#include "formats/asvkResWIC.h"


namespace asvk {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextureFactory class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      テクスチャリソースを生成します.
//-------------------------------------------------------------------------------------------------
bool TextureFactory::Create( const wchar_t* filename, ResTexture* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto ext = GetExt( filename );

    if ( ext == L"tga" )
    { return LoadResTextureFromTGA( filename, pResult ); }
    else if ( ext == L"dds" )
    { return LoadResTextureFromDDS( filename, pResult ); }
    else if ( ext == L"hdr" )
    { return LoadResTextureFromHDR( filename, pResult ); }
    else if ( ext == L"bmp"  ||
              ext == L"jpg"  ||
              ext == L"jpeg" ||
              ext == L"tif"  ||
              ext == L"tiff" ||
              ext == L"gif"  ||
              ext == L"png"  ||
              ext == L"hdp")
    { return LoadResTextureFromWIC( filename, pResult ); }

    ELOG( "Error : Invalid File Format. Extension is %s", ext.c_str() );
    return false;
}

//-------------------------------------------------------------------------------------------------
//      テクスチャリソースを破棄します.
//-------------------------------------------------------------------------------------------------
void TextureFactory::Dispose( ResTexture& value )
{
    for( uint32_t i=0; i<value.DepthOrArraySize; ++i )
    {
        value.pSurfaces[i].Width      = 0;
        value.pSurfaces[i].Height     = 0;
        value.pSurfaces[i].RowPitch   = 0;
        value.pSurfaces[i].SlicePitch = 0;
        SafeDeleteArray( value.pSurfaces[i].pPixels );
    }

    SafeDeleteArray( value.pSurfaces );
}

} // namespace asvk

