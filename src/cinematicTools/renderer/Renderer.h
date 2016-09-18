#include "d3dx11.h"
#include "D3D11.h"
#include "DXErr.h" 
#include "D3DX11async.h"
#include "D3Dcompiler.h" 
#include "D3dx11effect.h"
#include "D3D11Shader.h"
//#include "D3DX10math.h"
#include "FW1FontWrapper.h" 


#ifndef D3DCOLOR_ABGR 
#define D3DCOLOR_ABGR(a,b,g,r) \
    ((DWORD)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))
#endif 

#ifndef D3DCOLOR_ARGB 
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((DWORD)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif 

#ifndef D3DX_PI 
#define D3DX_PI ((FLOAT)3.141592654f) 
#endif 

#ifndef DEGTORAD 
#define DEGTORAD(degree) ((PI / 180.0f) * (degree)) 
#endif 

#ifndef DEGTORAD 
#define RADTODEG(radian) ((180.0f / PI) * (radian)) 
#endif 

#ifndef D3DXFLOAT16 
typedef struct D3DXFLOAT16 
{ 
public: 
    D3DXFLOAT16() {}; 
    D3DXFLOAT16( FLOAT ); 
    D3DXFLOAT16( CONST D3DXFLOAT16& ); 

    // casting 
    operator FLOAT (); 

    // binary operators 
    BOOL operator == ( CONST D3DXFLOAT16& ) const; 
    BOOL operator != ( CONST D3DXFLOAT16& ) const; 

protected: 
    WORD value; 
} D3DXFLOAT16, *LPD3DXFLOAT16; 
#endif 

#ifndef _D3DVECTOR 
typedef struct _D3DVECTOR { 
    float x; 
    float y; 
    float z; 
} D3DVECTOR; 
#endif 

#ifndef D3DXVECTOR3 
typedef struct D3DXVECTOR3 : public D3DVECTOR 
{ 
public: 
    D3DXVECTOR3() {}; 
    D3DXVECTOR3( CONST FLOAT * ); 
    D3DXVECTOR3( CONST D3DVECTOR& ); 
    D3DXVECTOR3( CONST D3DXFLOAT16 * ); 
    D3DXVECTOR3( FLOAT x, FLOAT y, FLOAT z ); 

    // casting 
    operator FLOAT* (); 
    operator CONST FLOAT* () const; 

    // assignment operators 
    D3DXVECTOR3& operator += ( CONST D3DXVECTOR3& ); 
    D3DXVECTOR3& operator -= ( CONST D3DXVECTOR3& ); 
    D3DXVECTOR3& operator *= ( FLOAT ); 
    D3DXVECTOR3& operator /= ( FLOAT ); 

    // unary operators 
    D3DXVECTOR3 operator + () const; 
    D3DXVECTOR3 operator - () const; 

    // binary operators 
    D3DXVECTOR3 operator + ( CONST D3DXVECTOR3& ) const; 
    D3DXVECTOR3 operator - ( CONST D3DXVECTOR3& ) const; 
    D3DXVECTOR3 operator * ( FLOAT ) const; 
    D3DXVECTOR3 operator / ( FLOAT ) const; 

    friend D3DXVECTOR3 operator * ( FLOAT, CONST struct D3DXVECTOR3& ); 

    BOOL operator == ( CONST D3DXVECTOR3& ) const; 
    BOOL operator != ( CONST D3DXVECTOR3& ) const; 

} D3DXVECTOR3, *LPD3DXVECTOR3; 
#endif 

#ifndef D3DXCOLOR 
typedef struct D3DXCOLOR 
{ 
public: 
    D3DXCOLOR() {}; 
    D3DXCOLOR( UINT  argb ); 
    D3DXCOLOR( CONST FLOAT * ); 
    D3DXCOLOR( CONST D3DXFLOAT16 * ); 
    D3DXCOLOR( FLOAT r, FLOAT g, FLOAT b, FLOAT a ); 

    // casting 
    operator UINT  () const; 

    operator FLOAT* (); 
    operator CONST FLOAT* () const; 

    // assignment operators 
    D3DXCOLOR& operator += ( CONST D3DXCOLOR& ); 
    D3DXCOLOR& operator -= ( CONST D3DXCOLOR& ); 
    D3DXCOLOR& operator *= ( FLOAT ); 
    D3DXCOLOR& operator /= ( FLOAT ); 

    // unary operators 
    D3DXCOLOR operator + () const; 
    D3DXCOLOR operator - () const; 

    // binary operators 
    D3DXCOLOR operator + ( CONST D3DXCOLOR& ) const; 
    D3DXCOLOR operator - ( CONST D3DXCOLOR& ) const; 
    D3DXCOLOR operator * ( FLOAT ) const; 
    D3DXCOLOR operator / ( FLOAT ) const; 

    friend D3DXCOLOR operator * ( FLOAT, CONST D3DXCOLOR& ); 

    BOOL operator == ( CONST D3DXCOLOR& ) const; 
    BOOL operator != ( CONST D3DXCOLOR& ) const; 

    FLOAT r, g, b, a; 
} D3DXCOLOR, *LPD3DXCOLOR; 
#endif 

struct COLOR_VERTEX 
{ 
    D3DXVECTOR3 Position; 
    D3DXCOLOR    Color; 
}; 

#define CIRCLE_NUMPOINTS 30 
#define MAX_VERTEX_COUNT ( CIRCLE_NUMPOINTS + 1 ) 

#ifndef DX11_RENDERER_H 
#define DX11_RENDERER_H 

class Dx11Renderer 
{ 
public: 
    Dx11Renderer( ); 
    ~Dx11Renderer( ); 

    bool IsRenderClassInitialized( ); 
    bool InitializeRenderClass( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, float DefaultFontSize = 14.0f, char *DefaultFont = "Arial", UINT DefaultFontFlags = 0x0 ); 
    void RenderText( int x, int y, DWORD color/*ARGB*/, char *szText, ... ); 
    void _RenderText( int x, int y, DWORD color/*ARGB*/, float Size, UINT Flags, char *szText, ... ); 
    void FillARGB( int x, int y, int w, int h, DWORD color/*ARGB*/ ); 
    void DrawLine( int x, int y, int x1, int y1, DWORD color/*ARGB*/ ); 
    void DrawCircle( int x, int y, int radius, DWORD color/*ARGB*/ ); 
    void DrawBorder( int x, int y, int w, int h, int px, DWORD BorderColor ); 
    void DrawBox( int x, int y, int w, int h, DWORD BoxColor, DWORD BorderColor ); 
    void DrawHealthBox( int x, int y, DWORD m_dColorOut, DWORD m_dColorIn, int m_iHealth, int m_iMaxHealth ); 
    void DrawRadar( int x, int y, int size ); 
    void DrawPoint( int x, int y, DWORD color ); 
    void DrawPixel( int x, int y, DWORD color ); 

private: 
    void DrawRectInternal( int x0, int y0, int x1, int y1, int r, int g, int b, int a ); 
    void DrawLineInternal( int x0, int y0, int x1, int y1, int r, int g, int b, int a ); 
    void DrawCircleInternal( int x0, int y0, int radius, int r, int g, int b, int a ); 

    ID3D11Device*            m_pDevice; 
    ID3D11DeviceContext*    m_pDeviceContext; 
    IFW1Factory*            m_pFW1Factory; 
    IFW1FontWrapper*        m_pFontWrapper; 
    ID3DX11Effect*            m_pEffect; 
    ID3DX11EffectTechnique* m_pTechnique; 
    ID3D11InputLayout*        m_pInputLayout; 
    ID3D11Buffer*            m_pVertexBuffer; 
    bool                    bIsInitialized; 
    float                    m_fDefaultFontSize; 
    UINT                    m_uiDefaultFontFlags; 
}; 


const DWORD txtRed            = D3DCOLOR_ARGB( 255,    255,    0,        0    ); 
const DWORD txtGreen        = D3DCOLOR_ARGB( 255,    0,        255,    0    ); 
const DWORD txtBlue            = D3DCOLOR_ARGB( 255,    0,        0,        255    ); 
const DWORD txtYellow        = D3DCOLOR_ARGB( 255,    255,    255,    0    ); 
const DWORD txtOrange        = D3DCOLOR_ARGB( 255,    255,    100,    0    ); 
const DWORD txtWhite        = D3DCOLOR_ARGB( 255,    255,    255,    255 ); 
const DWORD txtBlack        = D3DCOLOR_ARGB( 255,    0,        0,        0    ); 
const DWORD txtGrey            = D3DCOLOR_ARGB( 255,    86,        86,        86    ); 
const DWORD SpecialBlue        = D3DCOLOR_ARGB( 255,    51,        102,    255 ); 
const DWORD txtPink            = D3DCOLOR_ARGB( 255,    255,    0,        255    ); 
const DWORD txtTur            = D3DCOLOR_ARGB( 255,    150,    0,        255    ); 

const DWORD EspGreen        = D3DCOLOR_ARGB( 255,    15,        200,    15    ); 
const DWORD EspGrey            = D3DCOLOR_ARGB( 255,    80,        80,        80    ); 

#endif 

extern Dx11Renderer m_pDx11Renderer;  