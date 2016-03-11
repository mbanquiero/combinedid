#pragma once

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
extern LPDIRECT3DTEXTURE9      g_pShadowMap;    // Texture to which the shadow map is rendered
extern LPDIRECT3DSURFACE9      g_pDSShadow;     // Depth-stencil buffer for rendering to shadow map
extern LPDIRECT3DTEXTURE9      g_pXYZMap;		
extern LPDIRECT3DSURFACE9      g_pDSXYZ; 
extern LPDIRECT3DTEXTURE9      g_pDiapositiva;    
extern char fname_diapositiva[256];
extern int nro_diapo;
extern float diapo_elapsed_time;
extern float expo_elapsed_time;
extern float                   g_fLightFov;            // FOV of the spot light (in radian)
extern D3DXMATRIXA16           g_mShadowProj;          // Projection matrix for shadow map
extern bool                    g_bLuminance;   // If true, it renders only light intensity in gray scales
extern float					Zn;				// near plane
extern float					Zf;				// flar plane
extern float					zbias;			// zbias shadow map
extern float					EPSILON_FAR;	
extern float					EPSILON_NEAR;	
extern float					SHADOWMAP_SIZE;	// Tamaño del shadow map (512,1024,2048)

extern bool					g_bDebugShadowMap;	// Modo debug del shadow map
extern bool					g_bComparativa;		// modo comparativo entre ambos metodos
extern bool					g_bStandardShadowMap;	// shadow map standard
extern bool					g_bIDsShadowMap;	// shadow map solo ids
extern bool					g_bShowIDMap;		// Ver el mapa de ids
extern bool					g_bShowText;
extern bool					g_bAnimarSol;

extern float					g_fAmbient;


// Look from y look at auxiliar
extern D3DXVECTOR3 vFromPt2;
extern D3DXVECTOR3 vLookatPt2;
extern D3DXVECTOR3 vLightFromPt2;
extern D3DXVECTOR3 vLightLookatPt2;


// helpers
void DrawRect(IDirect3DDevice9* pd3dDevice,int x0,int y0,int x1,int y1,
				D3DCOLOR color=D3DCOLOR_XRGB(255,255,255));

void DrawDiapositiva(IDirect3DDevice9* pd3dDevice,char *fname,float elapsedtime);


// pasos de la demostracion
extern int demo_step;
extern int cant_steps;
