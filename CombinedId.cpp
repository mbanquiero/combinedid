#include "common\dxstdafx.h"
#include "common\dxutmesh.h"
#include "resource.h"
#include "CombinedId.h"
#include "core.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

// Este file contiene la interface con el framework 
//--------------------------------------------------------------------------------------
// Parche para que el DXut funcione orientado a eventos
// Estan modificadas algunas rutinas del DXUT para que solo repinte si 
// el flag _redraw_window esta en true. Eso se hace solo cuando mueve el mouse
// Por ende no existe info de FPS
//--------------------------------------------------------------------------------------
int cant_frames = 0;
//--------------------------------------------------------------------------------------

#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )


D3DVERTEXELEMENT9 g_aVertDecl[] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END()
};

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXFont*              g_pFontSmall = NULL;    // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
ID3DXEffect*            g_pEffect = NULL;       // D3DX effect interface
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
CDXUTDialog             g_HUD,g_HDiapo;                  // dialog for standard controls
CFirstPersonCamera      g_VCamera;              // View camera
CFirstPersonCamera      g_LCamera;              // Camera obj to help adjust light
CObj                    g_Obj[MAX_OBJ];         // Scene object meshes
LPDIRECT3DVERTEXDECLARATION9 g_pVertDecl = NULL;// Vertex decl for the sample
LPDIRECT3DTEXTURE9      g_pTexDef = NULL;       // Default texture for objects
D3DLIGHT9               g_Light;                // The spot light in the scene
CDXUTMesh               g_LightMesh;
CDXUTMesh               g_SkyBoxMesh;
bool                    g_bRenderSkyBox = true;
bool                    g_bRightMouseDown = false;// Indicates whether right mouse button is held
int					   g_nCurrScene = 0;				// Current scene
int					   g_nLoadedScene = 0;				// Current scene

//--------------------------------------------------------------------------------------
//LPCWSTR g_aszMeshFile[MAX_OBJ];
//D3DXMATRIXA16 g_amInitObjWorld[MAX_OBJ];
int						NUM_OBJ = 0;			// Number of mesh in the scene

// pool de meshes
int cant_mesh = 0;
CDXUTMesh m_Mesh[MAX_MESH];

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Initialize the camera
    g_VCamera.SetScalers( 0.01f, 15.0f );
    g_LCamera.SetScalers( 0.01f, 8.0f );
    g_VCamera.SetRotateButtons( true, false, false );
    g_LCamera.SetRotateButtons( false, false, true );

	strcpy(fname_diapositiva,"");

	//InitializePOV();


    // Initialize the spot light
    g_fLightFov = D3DX_PI / 2.0f;

    g_Light.Diffuse.r = 1.0f;
    g_Light.Diffuse.g = 1.0f;
    g_Light.Diffuse.b = 1.0f;
    g_Light.Diffuse.a = 1.0f;
    g_Light.Position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    g_Light.Direction = D3DXVECTOR3( 1.0f, -1.0f, 0.0f );
    D3DXVec3Normalize( (D3DXVECTOR3*)&g_Light.Direction, (D3DXVECTOR3*)&g_Light.Direction );
    g_Light.Range = 10.0f;
    g_Light.Theta = g_fLightFov / 2.0f;
    g_Light.Phi = g_fLightFov / 2.0f;

    // Set the callback functions. These functions allow DXUT to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then DXUT won't be able to 
    // recreate your device resources.
    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackMouse( MouseProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

	NUM_OBJ = InitializeScence();


    InitializeDialogs();

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    // Initialize DXUT and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"Combined ID and Deph Shadows" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 800, 600, IsDeviceAcceptable, ModifyDeviceSettings );
		
	// Redraw Window first time
	_redraw_window = true;
	_modo_interactivo = false;		// modo fps

    // Pass control to DXUT for handling the message pump and 
    // dispatching render calls. DXUT will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Sets up the dialogs
//--------------------------------------------------------------------------------------
void InitializeDialogs()
{
    g_SettingsDlg.Init( &g_DialogResourceManager );
	g_HUD.Init( &g_DialogResourceManager,true,L"media\\UI\\dxutcontrols.PNG");

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10; 
    //g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddCheckBox( IDC_ONLY_LUMINANCE, L"Luminance (F4)", 35, iY += 24, 125, 22, false, VK_F4 );
	g_HUD.AddCheckBox( IDC_DEBUG_SMAP, L"Debug Shadow map (F5)", 35, iY += 24, 125, 22, false, VK_F5 );
	g_HUD.AddCheckBox( IDC_COMPARATIVA, L"Comparative (F6)", 35, iY += 24, 125, 22, false, VK_F6 );
	g_HUD.AddCheckBox( IDC_SMAP_STANDARD, L"Standard SMAP(F7)", 35, iY += 24, 125, 22, false, VK_F7 );
	g_HUD.AddCheckBox( IDC_SMAP_SOLO_IDS, L"IDs SMAP(F8)", 35, iY += 24, 125, 22, false, VK_F8 );
	g_HUD.AddCheckBox( IDC_SHOW_IDMAP, L"Show IDMAP(F9)", 35, iY += 24, 125, 22, false, VK_F9 );

	g_HUD.AddStatic( IDC_AMBIENTLABEL, L"Ambient", 45, iY += 30, 125, 22 );
	g_HUD.AddSlider( IDC_AMBIENT, 45, iY += 20, 100, 22, 0, 100, 33);

	WCHAR wszText[50];
    //StringCchPrintf( wszText, 50, L"Zbias: %.10f", zbias);
    g_HUD.AddStatic( IDC_ZBIASLABEL, L"Zbias", 45, iY += 30, 125, 22 );
	g_HUD.AddSlider( IDC_ZBIAS, 45, iY += 20, 100, 22, 0, 100, 30);

	StringCchPrintf( wszText, 50, L"Smap Size: %d", (int)SHADOWMAP_SIZE);
    g_HUD.AddStatic( IDC_SMAP_SIZE_LABEL, wszText, 45, iY += 30, 125, 22 );
	int t = (int)(SHADOWMAP_SIZE/256);
	if(t>3)
		t = 3;
	g_HUD.AddSlider( IDC_SMAP_SIZE, 45, iY += 20, 100, 22, 1, 4,t);

	// Botones de pasos
	g_HDiapo.Init( &g_DialogResourceManager );
	g_HDiapo.SetCallback( OnGUIEvent ); 
	for(int i=0;i<cant_steps;++i)
	{
		StringCchPrintf( wszText, 50, L"%d", i+1);
		g_HDiapo.AddButton( IDC_FIRST_STEP+i, wszText, 10+i*25, 0, 20, 30);
	}

}



//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    // Must support pixel shader 2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    // need to support D3DFMT_R32F render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_CUBETEXTURE, D3DFMT_R32F ) ) )
        return false;

    // need to support D3DFMT_A8R8G8B8 render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_CUBETEXTURE, D3DFMT_A8R8G8B8 ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    // Turn vsync off
    pDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    g_SettingsDlg.GetDialogControl()->GetComboBox( DXUTSETTINGSDLG_PRESENT_INTERVAL )->SetEnabled( false );

    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
#endif
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning();
    }

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;


    V_RETURN( g_DialogResourceManager.OnCreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnCreateDevice( pd3dDevice ) );
    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 25, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );
    V_RETURN( D3DXCreateFont( pd3dDevice, 12, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFontSmall ) );

    // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
    // shader debugger. Debugging vertex shaders requires either REF or software vertex 
    // processing, and debugging pixel shaders requires REF.  The 
    // D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
    // shader debugger.  It enables source level debugging, prevents instruction 
    // reordering, prevents dead code elimination, and forces the compiler to compile 
    // against the next higher available software target, which ensures that the 
    // unoptimized shaders do not exceed the shader model limitations.  Setting these 
    // flags will cause slower rendering since the shaders will be unoptimized and 
    // forced into software.  See the DirectX documentation for more information about 
    // using the shader debugger.
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    // Read the D3DX effect file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"CombinedId.fx" ) );

    // If this fails, there should be debug output as to 
    // they the .fx file failed to compile
	ID3DXBuffer *pBuffer = NULL;
    if( FAILED(D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect, &pBuffer ) ))
	{
			char *saux = (char *)pBuffer->GetBufferPointer();
			return hr;
	}

    // Create vertex declaration
    V_RETURN( pd3dDevice->CreateVertexDeclaration( g_aVertDecl, &g_pVertDecl ) );

	// Cargo los meshes de todas las escenas
	// que permanecen en memoria hasta que termina el programa o se resetea el device
	V_RETURN( LoadMeshes(pd3dDevice));

    // Initialize the light mesh
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"spotlight.x" ) );
    if( FAILED( g_LightMesh.Create( pd3dDevice, str ) ) )
        return DXUTERR_MEDIANOTFOUND;
    V_RETURN( g_LightMesh.SetVertexDecl( pd3dDevice, g_aVertDecl ) );

	// Initialize the skybox
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"acropolis_sky.x" ) );
	if( FAILED( g_SkyBoxMesh.Create( pd3dDevice, str ) ) )
		return DXUTERR_MEDIANOTFOUND;
	V_RETURN( g_SkyBoxMesh.SetVertexDecl( pd3dDevice, g_aVertDecl ) );


    // World transform to identity
    D3DXMATRIXA16 mIdent;
    D3DXMatrixIdentity( &mIdent );
    V_RETURN( pd3dDevice->SetTransform( D3DTS_WORLD, &mIdent ) );

    return S_OK;
}


HRESULT LoadMeshes(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr = S_OK;
	LPCWSTR g_aszMeshFile[MAX_OBJ];
	g_aszMeshFile[0] = L"plano.x";
	g_aszMeshFile[1] = L"columna.x";
	g_aszMeshFile[2] = L"Cube.x";
	g_aszMeshFile[3] = L"divan.x";
	g_aszMeshFile[4] = L"blackholeroom.x";
	g_aszMeshFile[5] = L"arbol.x";
	g_aszMeshFile[6] = L"tree.x";
	g_aszMeshFile[7] = L"molino.x";
	g_aszMeshFile[8] = L"molino_hastas.x";
	g_aszMeshFile[9] = L"palmera2.x";
	cant_mesh = 10;

	WCHAR str[MAX_PATH];
	for( int i = 0; i < cant_mesh; ++i )
	{
		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, g_aszMeshFile[i] ) );
		if( FAILED( m_Mesh[i].Create( pd3dDevice, str ) ) )
			return DXUTERR_MEDIANOTFOUND;
		V_RETURN( m_Mesh[i].SetVertexDecl( pd3dDevice, g_aVertDecl ) );
	}

	return hr;
}



//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnResetDevice() );
    V_RETURN( g_SettingsDlg.OnResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pFontSmall )
        V_RETURN( g_pFontSmall->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_VCamera.SetProjParams( D3DX_PI/4, fAspectRatio, Zn, Zf );
    g_LCamera.SetProjParams( D3DX_PI/4, fAspectRatio, Zn, Zf );

    // Create the default texture (used when a triangle does not use a texture)
    V_RETURN( pd3dDevice->CreateTexture( 1, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pTexDef, NULL ) );
    D3DLOCKED_RECT lr;
    V_RETURN( g_pTexDef->LockRect( 0, &lr, NULL, 0 ) );
    *(LPDWORD)lr.pBits = D3DCOLOR_RGBA( 255, 255, 255, 255 );
    V_RETURN( g_pTexDef->UnlockRect( 0 ) );

	MyResetDevice(pd3dDevice,pBackBufferSurfaceDesc);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_VCamera.FrameMove( fElapsedTime );
    g_LCamera.FrameMove( fElapsedTime );

}




//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	if(uMsg==WM_KEYDOWN || uMsg==WM_KEYUP)
		_redraw_window = true;

	if(uMsg==WM_KEYDOWN && (wParam==VK_NEXT || wParam==VK_PRIOR))
		return 0;

    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

	*pbNoFurtherProcessing = g_HDiapo.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

    // Pass all windows messages to camera and dialogs so they can respond to user input
    if( WM_KEYDOWN != uMsg || g_bRightMouseDown )
        g_LCamera.HandleMessages( hWnd, uMsg, wParam, lParam );

    if( WM_KEYDOWN != uMsg || !g_bRightMouseDown )
		g_VCamera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
			case VK_F2:	_redraw_window = true; break;

            case VK_F1: g_bShowText = !g_bShowText; break;

			case VK_PRIOR:
				if(demo_step)
				{
					demo_step--;
					InitStep();
				}
				break;

			case VK_NEXT:
				if(demo_step<cant_steps-1)
				{
					demo_step++;
					InitStep();
				}
				break;

			case 'l':
			case 'L':
				g_bAnimarSol = !g_bAnimarSol;
				break;

			case VK_SPACE:
				// cambia de punto de vista al pto de vista auxiliar
				{
					D3DXVECTOR3 vFromPt = *g_VCamera.GetEyePt();
					D3DXVECTOR3 vLookatPt = *g_VCamera.GetLookAtPt();
					g_VCamera.SetViewParams( &vFromPt2, &vLookatPt2);
					vFromPt2 = vFromPt;
					vLookatPt2 = vLookatPt;

					vFromPt = *g_LCamera.GetEyePt();
					vLookatPt = *g_LCamera.GetLookAtPt();
					g_LCamera.SetViewParams( &vLightFromPt2, &vLightLookatPt2);
					vLightFromPt2 = vFromPt;
					vLightLookatPt2 = vLookatPt;
				}
				break;
        }
    }

}


void CALLBACK MouseProc( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext )
{
    g_bRightMouseDown = bRightButtonDown;

	if(bLeftButtonDown || bRightButtonDown)
		_redraw_window = true;
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: 
			DXUTToggleFullScreen(); 
			break;
        case IDC_ONLY_LUMINANCE:
			{
	            CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
				g_bLuminance = pCheck->GetChecked();
			}
            break;

		case IDC_DEBUG_SMAP:
			{
				CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
				g_bDebugShadowMap = pCheck->GetChecked();
			}
			break;

		case IDC_COMPARATIVA:
			{
				CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
				g_bComparativa = pCheck->GetChecked();
			}
			break;

		case IDC_SMAP_STANDARD:
			{
				CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
				g_bStandardShadowMap = pCheck->GetChecked();
			}
			break;

		case IDC_SMAP_SOLO_IDS:
			{
				CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
				g_bIDsShadowMap = pCheck->GetChecked();
			}
			break;

		case IDC_SHOW_IDMAP:
			{
				CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
				g_bShowIDMap = pCheck->GetChecked();
			}
			break;

        case IDC_ZBIAS:
            if( nEvent == EVENT_SLIDER_VALUE_CHANGED )
            {
                float k = float( ((CDXUTSlider *)pControl)->GetValue())/ 100.0f;
				zbias = 0.0001f*k + 0.00000000001f*(1-k);

                //WCHAR wszText[50];
				//                StringCchPrintf( wszText, 50, L"Zbias: %12.10f", zbias);
                g_HUD.GetStatic( IDC_ZBIASLABEL )->SetText( L"Zbias");

            }
			break;

		case IDC_AMBIENT:
			if( nEvent == EVENT_SLIDER_VALUE_CHANGED )
				g_fAmbient = (((CDXUTSlider *)pControl)->GetValue())/ 100.0f;
			break;

        case IDC_SMAP_SIZE:
            if( nEvent == EVENT_SLIDER_VALUE_CHANGED )
            {
                int ndx = int( ((CDXUTSlider *)pControl)->GetValue());
				switch(ndx)
				{
					case 1:
						SHADOWMAP_SIZE = 256.f;
						break;
					case 2:
					default:
						SHADOWMAP_SIZE = 512.f;
						break;
					case 3:
						SHADOWMAP_SIZE = 1024.f;
						break;
					case 4:
						SHADOWMAP_SIZE = 2048.f;
						break;
				}
                WCHAR wszText[50];
				StringCchPrintf( wszText, 50, L"Smap Size: %d", (int)SHADOWMAP_SIZE);
                g_HUD.GetStatic( IDC_SMAP_SIZE_LABEL )->SetText( wszText );

				// re-Create the shadows maps
				SAFE_RELEASE( g_pDSShadow );
				SAFE_RELEASE( g_pShadowMap );
				CreateShadowMap(DXUTGetD3DDevice());
            }
			break;

		default:
			if(nControlID>=IDC_FIRST_STEP)
				demo_step = nControlID-IDC_FIRST_STEP;
			InitStep();
            break;

    }
			
	_redraw_window = true;
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFontSmall , g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	WCHAR wszText[150];

	D3DXVECTOR3 v = *g_LCamera.GetEyePt();
	D3DXVECTOR3 w = *g_LCamera.GetLookAtPt();

	StringCchPrintf( wszText, 150, L"FL= (%.3f,%.3f,%.3f)  D=(%.3f,%.3f,%.3f)", 
			v.x,v.y,v.z,
			w.x,w.y,w.z);
	//txtHelper.DrawTextLine(wszText);
	
	v = *g_VCamera.GetEyePt();
	w = *g_VCamera.GetLookAtPt();
	StringCchPrintf( wszText, 150, L"LF= (%.3f,%.3f,%.3f) LA=(%.3f,%.3f,%.3f)", 
		v.x,v.y,v.z,
		w.x,w.y,w.z);
	txtHelper.DrawTextLine(wszText);

    txtHelper.DrawTextLine( DXUTGetFrameStats(true) ); // Show FPS
    
	//txtHelper.DrawTextLine( DXUTGetDeviceStats() );
	//StringCchPrintf( wszText, 50, L"#frames :%d", cant_frames++);
    //txtHelper.DrawTextLine(wszText);

    txtHelper.End();

	{
		CDXUTTextHelper txtHelper2( g_pFont , g_pTextSprite, 15 );
		txtHelper2.Begin();
		const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();
		txtHelper2.SetInsertionPos( pd3dsdBackBuffer->Width-120, pd3dsdBackBuffer->Height-27 );
		txtHelper2.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		WCHAR wszText[50];
		StringCchPrintf( wszText, 50, L"%d / %d  %.1f", 1+demo_step , cant_steps,expo_elapsed_time/60.0);
		txtHelper2.DrawTextLine(wszText);
		txtHelper2.End();
	}

}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnLostDevice();
    g_SettingsDlg.OnLostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pFontSmall )
        g_pFontSmall->OnLostDevice();
    SAFE_RELEASE(g_pTextSprite);
    SAFE_RELEASE( g_pTexDef );

	MyLostDevice();
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnDestroyDevice();
    g_SettingsDlg.OnDestroyDevice();
    SAFE_RELEASE( g_pFont );
    SAFE_RELEASE( g_pFontSmall );
    SAFE_RELEASE( g_pVertDecl );

	MyDestroyDevice();
}


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void InitStep()
{
	IDirect3DDevice9* pd3dDevice = DXUTGetD3DDevice();


	switch(demo_step)
	{
		case 0:
			// Caratula 
			nro_diapo = 1;
			break;
		case 1:
			// Objetivos del paper
			nro_diapo = 2;
			break;
		case 2:
			// Shadow mapping
			nro_diapo = 3;
			break;
		case 3:
			// Shadow map demo
			nro_diapo = 0;
			g_nCurrScene = 0;
			g_bComparativa = false;
			g_bStandardShadowMap = false;
			g_bIDsShadowMap = false;
			g_bShowIDMap = false;
			g_bDebugShadowMap = true;
			break;

		case 4:
			// Standard Shadow map artifacts
			nro_diapo = 0;
			g_nCurrScene = 0;
			g_bComparativa = false;
			g_bStandardShadowMap = true;
			g_bIDsShadowMap = false;
			g_bShowIDMap = false;
			g_bDebugShadowMap = false;
			zbias = 0.00001f;
			break;

		case 5:
			// Mapa de Ids demo
			nro_diapo = 0;
			g_nCurrScene = 0;
			g_bComparativa = false;
			g_bStandardShadowMap = false;
			g_bIDsShadowMap = true;
			g_bShowIDMap = true;
			g_bDebugShadowMap = false;
			break;

		case 6:
			// Artifact del Mapa de IDS
			nro_diapo = 4;
			break;

		case 7:
			// Artifact mapa de ids acne en el borde demo
			nro_diapo = 0;
			g_nCurrScene = 4;
			g_bComparativa = false;
			g_bStandardShadowMap = false;
			g_bIDsShadowMap = true;
			g_bShowIDMap = false;
			g_bDebugShadowMap = false;
			break;

		case 8:
			// Artifact mapa de ids self shadow
			nro_diapo = 0;
			g_nCurrScene = 2;
			g_bComparativa = false;
			g_bStandardShadowMap = false;
			g_bIDsShadowMap = true;
			g_bShowIDMap = false;
			g_bDebugShadowMap = false;
			break;

		case 9:
			// Combined id Primer diapo
			nro_diapo = 5;
			break;
		case 10:
			// Combined id Segunda diapo
			nro_diapo = 6;
			break;
		case 11:
			// Combined id Segunda diapo
			nro_diapo = 7;
			break;

		case 12:
			// Combined id Comparativa contra el ID map standard 
			// en artifact de los bordes
			nro_diapo = 0;
			g_nCurrScene = 1;
			g_bComparativa = true;
			g_bStandardShadowMap = false;
			g_bIDsShadowMap = true;
			g_bShowIDMap = false;
			g_bDebugShadowMap = false;
			break;

		case 13:
			// Combined id Comparativa contra el ID map standard
			// en soporte del self shadow
			nro_diapo = 0;
			g_nCurrScene = 2;
			g_bComparativa = true;
			g_bStandardShadowMap = false;
			g_bIDsShadowMap = true;
			g_bShowIDMap = false;
			g_bDebugShadowMap = false;
			break;

		case 14:
			// Combined id Comparativa contra el standard shadow map standard demo final
			nro_diapo = 0;
			g_nCurrScene = 3;
			g_bComparativa = true;
			g_bStandardShadowMap = true;
			g_bIDsShadowMap = false;
			g_bShowIDMap = false;
			g_bDebugShadowMap = false;
			break;

		case 15:
			// Conclusiones
			nro_diapo = 8;
			break;

		case 16:
			// Combined id demo final, mientras hacen preguntas, etc
			nro_diapo = 0;
			g_nCurrScene = 3;
			g_bComparativa = false;
			g_bStandardShadowMap = false;
			g_bIDsShadowMap = false;
			g_bShowIDMap = false;
			g_bDebugShadowMap = false;
			break;
	}

	if(g_nLoadedScene!=g_nCurrScene)
	{
		g_nLoadedScene = g_nCurrScene;

		// Recargo los objetos de la escena (solo las referencias, los mesh ya estan cargados)
		NUM_OBJ = InitializeScence();

		// Actualizo el control del dialogo
		WCHAR wszText[50];
		StringCchPrintf( wszText, 50, L"Smap Size: %d", (int)SHADOWMAP_SIZE);
		g_HUD.GetStatic( IDC_SMAP_SIZE_LABEL )->SetText( wszText );

		CDXUTSlider * p = g_HUD.GetSlider(IDC_SMAP_SIZE);
		int t = (int)(SHADOWMAP_SIZE/256);
		if(t>4)
			t = 4;
		p->SetValue(t);

		// re-Create the shadows maps
		SAFE_RELEASE( g_pDSShadow );
		SAFE_RELEASE( g_pShadowMap );
		CreateShadowMap(DXUTGetD3DDevice());

	}
}
