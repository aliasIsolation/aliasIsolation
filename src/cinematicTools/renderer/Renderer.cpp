/* 
Credits: @ 2011 
            s0beit 
            NeoIII 
*/ 

#include <stdio.h> 
#include <string> 
#include <sstream> 
#include <iostream> 
//#include "Tools.h" 
#include "Renderer.h" 
#include "shader_fx.h" 

#pragma comment (lib, "legacy_stdio_definitions.lib")
#pragma comment (lib, "DXErr")
#pragma comment (lib, "D3D11.lib")
#pragma comment (lib, "D3DX11.lib") 
#pragma comment (lib, "d3dcompiler.lib") 
//#pragma comment (lib, "Effects11.lib") 

#pragma warning ( disable: 4102 ) 
#pragma warning ( disable: 4311 ) 
#pragma warning ( disable: 4312 ) 

#pragma warning ( disable: 4244 ) 
#pragma warning ( disable: 4996 ) 

static std::string asciiEncode( const std::wstring & w ); 
static std::wstring asciiDecode( const std::string & s ); 

static std::string asciiEncode( const std::wstring & w ) 
{ 
    std::ostringstream  s; 
    char *              c; 
    int                    r; 

    c = new char[ MB_CUR_MAX + 1 ]; 
    for( size_t i = 0; i < w.length(); i++ ) { 
        r = wctomb( c, w[i] ); 
        c[r] = '\0'; 
        if( r <= 1 && c[0] > 0 ) { 
            s << c; 
        } 
    } 
    return s.str(); 
} 

static std::wstring asciiDecode( const std::string & s ) 
{ 
    std::wostringstream    w; 
    wchar_t                c; 

    for( size_t i = 0; i < s.length(); i++ ) { 
        mbtowc( &c, &s[i], 1 ); 
        w << c; 
    } 
    return w.str(); 
} 

Dx11Renderer::Dx11Renderer( ) 
{ 
    m_pDevice        = NULL; 
    m_pDeviceContext= NULL; 
    m_pFW1Factory    = NULL; 
    m_pFontWrapper    = NULL; 
    m_pEffect        = NULL; 
    m_pTechnique    = NULL; 
    m_pInputLayout    = NULL; 
    m_pVertexBuffer = NULL; 
    bIsInitialized    = false; 
} 

Dx11Renderer::~Dx11Renderer( ) 
{ 
    m_pDevice        = NULL; 
    m_pDeviceContext= NULL; 
    m_pFW1Factory    = NULL; 
    m_pFontWrapper    = NULL; 
    m_pEffect        = NULL; 
    m_pTechnique    = NULL; 
    m_pInputLayout    = NULL; 
    m_pVertexBuffer = NULL; 
    bIsInitialized    = false; 
} 

bool Dx11Renderer::IsRenderClassInitialized( ) 
{ 
    return this->bIsInitialized; 
} 

bool Dx11Renderer::InitializeRenderClass( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, float DefaultFontSize, char *DefaultFont, UINT DefaultFontFlags ) 
{ 
    if( !pDevice || !pDeviceContext ) 
        return false; 

    if( this->bIsInitialized == true ) 
        return this->bIsInitialized; 

    this->m_pDevice                = pDevice; 
    this->m_pDeviceContext        = pDeviceContext; 
    this->m_fDefaultFontSize    = DefaultFontSize; 
    this->m_uiDefaultFontFlags    = DefaultFontFlags; 

    if( FAILED( FW1CreateFactory( FW1_VERSION, &m_pFW1Factory ) ) ) 
        return false; 

    if( FAILED( m_pFW1Factory->CreateFontWrapper( this->m_pDevice, asciiDecode( DefaultFont ).c_str(), &m_pFontWrapper ) ) ) 
        return false; 

    this->m_pFW1Factory->Release(); 

    /* ********** At this point Font is Initialized :) ********** */ 

    ID3D10Blob *compiledFX = NULL, *errorMsgs = NULL; 

    if( FAILED( D3DX11CompileFromMemory( shaderRaw, strlen( shaderRaw ), "FillTechFx", NULL, NULL, "FillTech", "fx_5_0", NULL, NULL, NULL, &compiledFX, &errorMsgs, NULL ) ) ) 
        return false; 

    if( FAILED( D3DX11CreateEffectFromMemory( compiledFX->GetBufferPointer(), compiledFX->GetBufferSize(), 0, this->m_pDevice, &m_pEffect ) ) ) 
        return false; 

    compiledFX->Release(); 

    m_pTechnique = m_pEffect->GetTechniqueByName( "FillTech" ); 

    if( m_pTechnique == NULL ) 
        return false; 

    D3D11_INPUT_ELEMENT_DESC lineRectLayout[] = 
    { 
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },   
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 } 
    }; 

    D3DX11_PASS_DESC passDesc; 

    if( FAILED( m_pTechnique->GetPassByIndex( 0 )->GetDesc( &passDesc ) ) ) 
        return false; 

    if( FAILED( this->m_pDevice->CreateInputLayout( lineRectLayout, sizeof( lineRectLayout ) / sizeof( lineRectLayout[0] ), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout ) ) ) 
        return false; 

    D3D11_BUFFER_DESC bufferDesc; 

    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC; 
    bufferDesc.ByteWidth        = MAX_VERTEX_COUNT * sizeof( COLOR_VERTEX ); 
    bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER; 
    bufferDesc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE; 
    bufferDesc.MiscFlags        = 0; 

    if( FAILED( this->m_pDevice->CreateBuffer( &bufferDesc, NULL, &m_pVertexBuffer ) ) ) 
        return false; 

    this->bIsInitialized = true; 

    return this->bIsInitialized; 
} 

void Dx11Renderer::RenderText( int x, int y, DWORD color, char *szText, ... ) 
{ 
    if( this->bIsInitialized == false ) 
        return; 

    if( m_pDevice == NULL || m_pDeviceContext == NULL )  
        return; 

    va_list va_alist; 
    char logbuf[256]; 

    va_start (va_alist, szText); 
    _vsnprintf(logbuf, sizeof(logbuf), szText, va_alist); 
    va_end (va_alist); 

    int a = (color >> 24) & 0xff; 
    int r = (color >> 16) & 0xff; 
    int g = (color >> 8) & 0xff; 
    int b = (color) & 0xff; 

    DWORD dwABGR    = D3DCOLOR_ABGR( a, b, g, r ); 
    DWORD dwBlack    = D3DCOLOR_ABGR( 255, 0, 0, 0 ); 

    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), m_fDefaultFontSize, x+1, y, dwBlack, m_uiDefaultFontFlags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), m_fDefaultFontSize, x-1, y, dwBlack, m_uiDefaultFontFlags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), m_fDefaultFontSize, x, y+1, dwBlack, m_uiDefaultFontFlags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), m_fDefaultFontSize, x, y-1, dwBlack, m_uiDefaultFontFlags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), m_fDefaultFontSize, x, y, dwABGR, m_uiDefaultFontFlags );             
} 

void Dx11Renderer::_RenderText( int x, int y, DWORD color, float Size, UINT Flags, char *szText, ... ) 
{ 
    if( this->bIsInitialized == false ) 
        return; 

    if( m_pDevice == NULL || m_pDeviceContext == NULL )  
        return; 

    va_list va_alist; 
    char logbuf[256]; 

    va_start (va_alist, szText); 
    _vsnprintf(logbuf, sizeof(logbuf), szText, va_alist); 
    va_end (va_alist); 

    int a = (color >> 24) & 0xff; 
    int r = (color >> 16) & 0xff; 
    int g = (color >> 8) & 0xff; 
    int b = (color) & 0xff; 

    DWORD dwABGR    = D3DCOLOR_ABGR( a, b, g, r ); 
    DWORD dwBlack    = D3DCOLOR_ABGR( 255, 0, 0, 0 ); 

    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), Size, x+1, y, dwBlack, Flags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), Size, x-1, y, dwBlack, Flags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), Size, x, y+1, dwBlack, Flags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), Size, x, y-1, dwBlack, Flags ); 
    m_pFontWrapper->DrawString( m_pDeviceContext, asciiDecode( logbuf ).c_str(), Size, x, y, dwABGR, Flags );         
} 

void Dx11Renderer::FillARGB( int x, int y, int w, int h, DWORD color/*ARGB*/ ) 
{ 
    int a = (color >> 24) & 0xff; 
    int r = (color >> 16) & 0xff; 
    int g = (color >> 8) & 0xff; 
    int b = (color) & 0xff; 
    this->DrawRectInternal( x, y, x + w, y + h, r, g, b, a ); 
} 

void Dx11Renderer::DrawLine( int x, int y, int x1, int y1, DWORD color/*ARGB*/ ) 
{ 
    int a = (color >> 24) & 0xff; 
    int r = (color >> 16) & 0xff; 
    int g = (color >> 8) & 0xff; 
    int b = (color) & 0xff; 
    this->DrawLineInternal( x, y, x1, y1, r, g, b, a ); 
} 

void Dx11Renderer::DrawCircle( int x, int y, int radius, DWORD color/*ARGB*/ ) 
{ 
    int a = (color >> 24) & 0xff; 
    int r = (color >> 16) & 0xff; 
    int g = (color >> 8) & 0xff; 
    int b = (color) & 0xff; 
    this->DrawCircleInternal( x, y, radius, r, g, b, a ); 
} 

void Dx11Renderer::DrawBorder( int x, int y, int w, int h, int px, DWORD BorderColor ) 
{ 
    FillARGB( x, (y + h - px), w, px,    BorderColor ); 
    FillARGB( x, y, px, h,                BorderColor ); 
    FillARGB( x, y, w, px,                BorderColor ); 
    FillARGB( (x + w - px), y, px, h,    BorderColor ); 
} 

void Dx11Renderer::DrawBox( int x, int y, int w, int h, DWORD BoxColor, DWORD BorderColor ) 
{ 
    FillARGB( x, y, w, h,        BoxColor ); 
    DrawBorder( x, y, w, h, 1,    BorderColor ); 
} 

void Dx11Renderer::DrawHealthBox( int x, int y, DWORD m_dColorOut, DWORD m_dColorIn, int m_iHealth, int m_iMaxHealth ) 
{ 
    float mx = ( float )m_iMaxHealth / 4; 
    float w = ( float )m_iHealth / 4; 
    x -= ( ( int )mx / 2 ); 

    //background 
    FillARGB( x, y, ( int )mx, 4, m_dColorOut ); 
    //inside 
    FillARGB( x, y, ( int )w, 4, m_dColorIn ); 
    //outline 
    DrawBorder( x - 1, y - 1, ( int )mx + 2, 6, 1,    D3DCOLOR_ARGB( 255, 30, 30, 30 ) ); 
} 

void Dx11Renderer::DrawRadar( int x, int y, int size ) 
{ 
    //int x = 300, y = 300, size = 150; 
    DrawBox( x, y, size, size, D3DCOLOR_ARGB( 150, 186, 186, 186 ), D3DCOLOR_ARGB( 250, 25, 180, 255 ) ); 
    //    DrawBox( (x+5), (y+5), (size-10), (size-10), DWORD_ARGB( 255, 186, 186, 186 ), DWORD_ARGB( 255, 184, 184, 184 ), pDevice ); 
    FillARGB( (x+(size/2)), (y+1/*+6*/), 1, (size-2/*-12*/), D3DCOLOR_ARGB( 255, 0, 0, 0 ) ); 
    FillARGB( (x+1/*+6*/), (y+(size/2)), (size-2/*-12*/), 1, D3DCOLOR_ARGB( 255, 0, 0, 0 ) ); 
} 

void Dx11Renderer::DrawPoint( int x, int y, DWORD color ) 
{ 
    FillARGB(x-1,y-1,2,2,color); 
} 

void Dx11Renderer::DrawPixel( int x, int y, DWORD color ) 
{ 
    FillARGB(x,y,1,1,color); 
} 

void Dx11Renderer::DrawRectInternal( int x0, int y0, int x1, int y1, int r, int g, int b, int a ) 
{ 
    if( this->bIsInitialized == false ) 
        return; 

    if( m_pDevice == NULL || m_pDeviceContext == NULL )  
        return; 

    UINT viewportNumber = 1; 

    D3D11_VIEWPORT vp; 

    this->m_pDeviceContext->RSGetViewports( &viewportNumber, &vp ); 

    float xx0 = 2.0f * ( x0 - 0.5f ) / vp.Width - 1.0f; 
    float yy0 = 1.0f - 2.0f * ( y0 - 0.5f ) / vp.Height; 
    float xx1 = 2.0f * ( x1 - 0.5f ) / vp.Width - 1.0f; 
    float yy1 = 1.0f - 2.0f * ( y1 - 0.5f ) / vp.Height; 

    COLOR_VERTEX* v = NULL; 

    D3D11_MAPPED_SUBRESOURCE mapData; 

    if( FAILED( this->m_pDeviceContext->Map( m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData ) ) ) 
        return; 

    v = ( COLOR_VERTEX* ) mapData.pData; 

    v[0].Position.x = xx0; 
    v[0].Position.y = yy0; 
    v[0].Position.z = 0; 
    v[0].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[0].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[0].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[0].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[1].Position.x = xx1; 
    v[1].Position.y = yy0; 
    v[1].Position.z = 0; 
    v[1].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[1].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[1].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[1].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[2].Position.x = xx0; 
    v[2].Position.y = yy1; 
    v[2].Position.z = 0; 
    v[2].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[2].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[2].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[2].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[3].Position.x = xx1; 
    v[3].Position.y = yy1; 
    v[3].Position.z = 0; 
    v[3].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[3].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[3].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[3].Color.a = ( ( FLOAT ) a / 255.0f ); 

    this->m_pDeviceContext->Unmap( m_pVertexBuffer, NULL ); 

    this->m_pDeviceContext->IASetInputLayout( m_pInputLayout ); 

    UINT Stride = sizeof( COLOR_VERTEX ); 
    UINT Offset = 0; 

    this->m_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &Stride, &Offset ); 

    this->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP ); 

    D3DX11_TECHNIQUE_DESC techDesc; 

    if( FAILED( m_pTechnique->GetDesc( &techDesc ) ) ) 
        return; 

    for( UINT p = 0; p < techDesc.Passes; ++p ) 
    { 
        m_pTechnique->GetPassByIndex( p )->Apply( 0, this->m_pDeviceContext ); 

        this->m_pDeviceContext->Draw( 4, 0 ); 
    } 
}  

void Dx11Renderer::DrawLineInternal( int x0, int y0, int x1, int y1, int r, int g, int b, int a ) 
{ 
    if( this->bIsInitialized == false ) 
        return; 

    if( m_pDevice == NULL || m_pDeviceContext == NULL )  
        return; 

    UINT viewportNumber = 1; 

    D3D11_VIEWPORT vp; 

    this->m_pDeviceContext->RSGetViewports( &viewportNumber, &vp ); 

    float xx0 = 2.0f * ( x0 - 0.5f ) / vp.Width - 1.0f; 
    float yy0 = 1.0f - 2.0f * ( y0 - 0.5f ) / vp.Height; 
    float xx1 = 2.0f * ( x1 - 0.5f ) / vp.Width - 1.0f; 
    float yy1 = 1.0f - 2.0f * ( y1 - 0.5f ) / vp.Height; 

    COLOR_VERTEX* v = NULL; 

    D3D11_MAPPED_SUBRESOURCE mapData; 

    if( FAILED( this->m_pDeviceContext->Map( m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData ) ) ) 
        return; 

    v = ( COLOR_VERTEX* ) mapData.pData; 

    v[0].Position.x = xx0; 
    v[0].Position.y = yy0; 
    v[0].Position.z = 0; 
    v[0].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[0].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[0].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[0].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[1].Position.x = xx1; 
    v[1].Position.y = yy1; 
    v[1].Position.z = 0; 
    v[1].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[1].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[1].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[1].Color.a = ( ( FLOAT ) a / 255.0f ); 

    this->m_pDeviceContext->Unmap( m_pVertexBuffer, NULL ); 

    this->m_pDeviceContext->IASetInputLayout( m_pInputLayout ); 

    UINT Stride = sizeof( COLOR_VERTEX ); 
    UINT Offset = 0; 

    this->m_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &Stride, &Offset ); 

    this->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP ); 

    D3DX11_TECHNIQUE_DESC techDesc; 

    if( FAILED( m_pTechnique->GetDesc( &techDesc ) ) ) 
        return; 

    for( UINT p = 0; p < techDesc.Passes; ++p ) 
    { 
        m_pTechnique->GetPassByIndex( p )->Apply( 0, this->m_pDeviceContext ); 

        this->m_pDeviceContext->Draw( 2, 0 ); 
    } 
} 

void Dx11Renderer::DrawCircleInternal( int x0, int y0, int radius, int r, int g, int b, int a ) 
{ 
    if( this->bIsInitialized == false ) 
        return; 

    if( m_pDevice == NULL || m_pDeviceContext == NULL )  
        return; 

    const int NUMPOINTS = CIRCLE_NUMPOINTS; 
    UINT viewportNumber = 1; 

    D3D11_VIEWPORT vp; 

    this->m_pDeviceContext->RSGetViewports( &viewportNumber, &vp ); 

    COLOR_VERTEX* v = NULL; 

    D3D11_MAPPED_SUBRESOURCE mapData; 

    if( FAILED( this->m_pDeviceContext->Map( m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData ) ) ) 
        return; 

    v = ( COLOR_VERTEX* ) mapData.pData; 

    float WedgeAngle = ( float )( ( 2 * D3DX_PI ) / NUMPOINTS ); 

    for( int i = 0; i <= NUMPOINTS; i++ ) 
    { 
        float Theta        = ( float )( i * WedgeAngle ); 
        float x            = (float)( x0 + radius * cos( Theta ) ); 
        float y            = (float)( y0 - radius * sin( Theta ) ); 

        v[i].Position.x    = 2.0f * ( x - 0.5f ) / vp.Width - 1.0f; 
        v[i].Position.y    = 1.0f - 2.0f * ( y - 0.5f ) / vp.Height; 
        v[i].Position.z    = 0.0f; 
        v[i].Color.r    = ( ( FLOAT ) r / 255.0f ); 
        v[i].Color.g    = ( ( FLOAT ) g / 255.0f ); 
        v[i].Color.b    = ( ( FLOAT ) b / 255.0f ); 
        v[i].Color.a    = ( ( FLOAT ) a / 255.0f ); 
    } 

    this->m_pDeviceContext->Unmap( m_pVertexBuffer, NULL ); 

    this->m_pDeviceContext->IASetInputLayout( m_pInputLayout ); 

    UINT Stride = sizeof( COLOR_VERTEX ); 
    UINT Offset = 0; 

    this->m_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &Stride, &Offset ); 

    this->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP ); 
    //this->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP ); 

    D3DX11_TECHNIQUE_DESC techDesc; 

    if( FAILED( m_pTechnique->GetDesc( &techDesc ) ) ) 
        return; 

    for( UINT p = 0; p < techDesc.Passes; ++p ) 
    { 
        m_pTechnique->GetPassByIndex( p )->Apply( 0, this->m_pDeviceContext ); 

        this->m_pDeviceContext->Draw( NUMPOINTS+1, 0 ); 
    } 
}  