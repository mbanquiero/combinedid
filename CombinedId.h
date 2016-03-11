#pragma once

extern bool _redraw_window;
extern bool _modo_interactivo;
extern int cant_frames;

#define HELPTEXTCOLOR D3DXCOLOR( 0.0f, 1.0f, 0.3f, 1.0f )
#define MAX_OBJ				500
#define MAX_MESH			50
//-----------------------------------------------------------------------------
// Descripcion de la escena
// La escena se define como una lista de mallas 
// y su correspondiente matriz de view
//-----------------------------------------------------------------------------
//extern LPCWSTR g_aszMeshFile[MAX_OBJ];
//extern D3DXMATRIXA16 g_amInitObjWorld[MAX_OBJ];
extern int NUM_OBJ;

extern int cant_mesh;
extern CDXUTMesh m_Mesh[MAX_MESH];

struct CObj
{
    int nro_mesh;
    D3DXMATRIXA16 m_mWorld;
};

HRESULT LoadMeshes(IDirect3DDevice9* pd3dDevice);


//-----------------------------------------------------------------------------
// Name: class CViewCamera
// Desc: A camera class derived from CFirstPersonCamera.  The arrow keys and
//       numpad keys are disabled for this type of camera.
//-----------------------------------------------------------------------------
class CViewCamera : public CFirstPersonCamera
{
protected:
    virtual D3DUtil_CameraKeys MapKey( UINT nKey )
    {
        // Provide custom mapping here.
        // Same as default mapping but disable arrow keys.
        switch( nKey )
        {
            case 'A':      return CAM_STRAFE_LEFT;
            case 'D':      return CAM_STRAFE_RIGHT;
            case 'W':      return CAM_MOVE_FORWARD;
            case 'S':      return CAM_MOVE_BACKWARD;
            case 'Q':      return CAM_MOVE_DOWN;
            case 'E':      return CAM_MOVE_UP;

            case VK_HOME:   return CAM_RESET;
        }

        return CAM_UNKNOWN;
    }
};




//-----------------------------------------------------------------------------
// Name: class CLightCamera
// Desc: A camera class derived from CFirstPersonCamera.  The letter keys
//       are disabled for this type of camera.  This class is intended for use
//       by the spot light.
//-----------------------------------------------------------------------------
class CLightCamera : public CFirstPersonCamera
{
protected:
    virtual D3DUtil_CameraKeys MapKey( UINT nKey )
    {
        // Provide custom mapping here.
        // Same as default mapping but disable arrow keys.
        switch( nKey )
        {
            case VK_LEFT:  return CAM_STRAFE_LEFT;
            case VK_RIGHT: return CAM_STRAFE_RIGHT;
            case VK_UP:    return CAM_MOVE_FORWARD;
            case VK_DOWN:  return CAM_MOVE_BACKWARD;
            case VK_PRIOR: return CAM_MOVE_UP;        // pgup
            case VK_NEXT:  return CAM_MOVE_DOWN;      // pgdn

            case VK_NUMPAD4: return CAM_STRAFE_LEFT;
            case VK_NUMPAD6: return CAM_STRAFE_RIGHT;
            case VK_NUMPAD8: return CAM_MOVE_FORWARD;
            case VK_NUMPAD2: return CAM_MOVE_BACKWARD;
            case VK_NUMPAD9: return CAM_MOVE_UP;        
            case VK_NUMPAD3: return CAM_MOVE_DOWN;      

            case VK_HOME:   return CAM_RESET;
        }

        return CAM_UNKNOWN;
    }
};


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
extern ID3DXFont*              g_pFont;         // Font for drawing text
extern ID3DXFont*              g_pFontSmall;    // Font for drawing text
extern ID3DXSprite*            g_pTextSprite;   // Sprite for batching draw text calls
extern ID3DXEffect*            g_pEffect;       // D3DX effect interface
extern bool                    g_bShowHelp;     // If true, it renders the UI control text
extern CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
extern CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
extern CDXUTDialog             g_HUD,g_HDiapo;                  // dialog for standard controls
extern CFirstPersonCamera      g_VCamera;              // View camera
extern CFirstPersonCamera      g_LCamera;              // Camera obj to help adjust light
extern CObj                    g_Obj[MAX_OBJ];         // Scene object meshes
extern LPDIRECT3DVERTEXDECLARATION9 g_pVertDecl;// Vertex decl for the sample
extern LPDIRECT3DTEXTURE9      g_pTexDef;       // Default texture for objects
extern D3DLIGHT9               g_Light;                // The spot light in the scene
extern CDXUTMesh               g_LightMesh;
extern CDXUTMesh               g_SkyBoxMesh;
extern bool                    g_bRenderSkyBox;
extern bool                    g_bRightMouseDown;// Indicates whether right mouse button is held
extern int					   g_nCurrScene;		// scene
extern int					   g_nLoadedScene;		// scene

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN			101
#define IDC_RENDERTYPE_DRAFT			106
#define IDC_RENDERTYPE_PCSS				107
#define IDC_RENDERTYPE_SOFT_SHADOWS		108
#define IDC_ONLY_LUMINANCE				109
#define IDC_ZBIAS						110
#define IDC_ZBIASLABEL					111
#define IDC_LWIDTH						112
#define IDC_LWIDTHLABEL					113
#define IDC_SMAP_SIZE					114
#define IDC_SMAP_SIZE_LABEL				115
#define IDC_DEBUG_SMAP					116
#define IDC_COMPARATIVA					117
#define IDC_SMAP_STANDARD				118
#define IDC_SHOW_IDMAP					119
#define IDC_SMAP_SOLO_IDS				120
#define IDC_AMBIENT						121
#define IDC_AMBIENTLABEL				122

// First Scence button
#define IDC_FIRST_SCENE					150
// First Diapo button
#define IDC_FIRST_STEP					200


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
void             InitializeDialogs();
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void             RenderText();
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK MouseProc( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );
int		InitializeScence();
HRESULT MyResetDevice(IDirect3DDevice9* pd3dDevice,const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
void	MyLostDevice();
void	MyDestroyDevice();
HRESULT CreateShadowMap(IDirect3DDevice9* pd3dDevice);
//void	InitializePOV();
void    Update( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime);

void	InitStep();
