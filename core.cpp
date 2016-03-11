//-----------------------------------------------------------------------------
// Este file contiene el Algorithm code pp dicho
//-----------------------------------------------------------------------------
#include "common\dxstdafx.h"
#include "common\dxutmesh.h"
#include "resource.h"
#include "CombinedId.h"
#include "core.h"

#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
LPDIRECT3DTEXTURE9      g_pShadowMap = NULL;    // Texture to which the shadow map is rendered
LPDIRECT3DSURFACE9      g_pDSShadow = NULL;     // Depth-stencil buffer for rendering to shadow map
LPDIRECT3DTEXTURE9      g_pXYZMap = NULL;    
LPDIRECT3DSURFACE9      g_pDSXYZ = NULL;     
float                   g_fLightFov;            // FOV of the spot light (in radian)
D3DXMATRIXA16           g_mShadowProj;          // Projection matrix for shadow map
bool                    g_bLuminance = false;   // If true, it renders only light intensity in gray scales
float					Zn = 0.1f;				// near plane
float					Zf = 1000.f;				// flar plane
float					zbias = 0.00003f;			// zbias shadow map
float					EPSILON_FAR = 0.001f;	
float					EPSILON_NEAR = 0.00001f;	
float					SHADOWMAP_SIZE = 512.f;	// Tamaño del shadow map (512,1024,2048)
D3DVIEWPORT9			viewport_shadowmap;
bool					g_bDebugShadowMap = false;	// Modo debug del shadow map
bool					g_bComparativa = false;		// modo comparativo entre ambos metodos
bool					g_bStandardShadowMap = false;	// shadow map standard
bool					g_bIDsShadowMap = false;	// shadow map solo ids
bool					g_bShowIDMap = false;		// Ver el mapa de ids
float					g_fAmbient = 0.3f;
bool					g_bShowText = true;			// Ver el texto
bool					g_bAnimarSol = true;

LPDIRECT3DTEXTURE9      g_pDiapositiva = NULL;    
char fname_diapositiva[256];
int nro_diapo = 1;
float diapo_elapsed_time = 0;
float expo_elapsed_time = 0;

int demo_step = 0;
int cant_steps = 17;

D3DXVECTOR3 vFromPt2 = D3DXVECTOR3( 0,0,0);
D3DXVECTOR3 vLookatPt2 = D3DXVECTOR3( 0,0,0);
D3DXVECTOR3 vLightFromPt2 = D3DXVECTOR3( 0,0,0);
D3DXVECTOR3 vLightLookatPt2 = D3DXVECTOR3( 0,0,0);


//-----------------------------------------------------------------------------
// Descripcion de la escena
// La escena se define como una lista de mallas 
// y su correspondiente matriz de view
//-----------------------------------------------------------------------------


int	TestScene0()
{
	// Planos con columnas para demostrar como funciona un shadow map
	int cant = 0;
	g_Obj[cant].nro_mesh = 0;		// "plano.x"
	g_Obj[cant++].m_mWorld =	D3DXMATRIXA16(	1,0, 0, 0, 
										0, 1, 0, 0, 
										0, 0, 1, 0, 
										0, 0, 0, 1 );

	double alfa = 3.1415/2;
	int cant_col = 10;
	double da = 3.1415/cant_col;
	double r = 15;
	for(int i=0;i<cant_col;++i)
	{
		g_Obj[cant].nro_mesh = 1;	// L"columna.x";
		g_Obj[cant].m_mWorld =
			D3DXMATRIXA16(	2,0, 0, 0, 
			0, 4, 0, 0, 
			0, 0, 2, 0, 
			r*sin(alfa), -4, r*cos(alfa), 1 );
		alfa+=da;
		++cant;
	}

	EPSILON_FAR = 0.001f;	
	EPSILON_NEAR = 0.00001f;	
	SHADOWMAP_SIZE = 512.f;

	D3DXVECTOR3 vFromPt   = D3DXVECTOR3( -4.651f, 14.758f, -45.389f );
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3( -4.379f, 14.447f, -44.479f );
	g_VCamera.SetViewParams( &vFromPt, &vLookatPt );
	vFromPt = D3DXVECTOR3( 6.607f, 9.337f, -32.558f );
	vLookatPt = D3DXVECTOR3( 6.706f, 8.949f, -31.642f );
	g_LCamera.SetViewParams( &vFromPt, &vLookatPt );

	vFromPt2   = D3DXVECTOR3( 9.631f, 22.386f, 22.793f );
	vLookatPt2 = D3DXVECTOR3( 9.604f, 22.009f, 21.867f );
	vLightFromPt2 = D3DXVECTOR3( 7.615f, 21.368f, -65.290f );
	vLightLookatPt2 = D3DXVECTOR3( 7.720f, 21.152f, -64.319f );


	g_bRenderSkyBox = false;

	return cant;
}

int	TestScene1()
{
	// Escena con cubos, artifact Selft Shadow en ID maps
	int cant = 0;
	g_Obj[cant].nro_mesh = 0;		// "plano.x"
	g_Obj[cant++].m_mWorld =	
		D3DXMATRIXA16(	1, 0, 0, 0,
		0, 1, 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1 );

	int t = 1;
	for(int j =0;j<5;++j)
	{
		for(int i =0;i<6-j;++i)
		{
			g_Obj[cant].nro_mesh = 2;		// "Cube.x"
			g_Obj[cant].m_mWorld =	
				D3DXMATRIXA16(	1, 0, 0, 0,
				0, 6-j, 0, 0, 
				0, 0, 1, 0, 
				i, 0, i+j, 1 );
			++t;
			++cant;
		}
	}

	EPSILON_FAR = 0.001f;	
	EPSILON_NEAR = 0.00001f;	
	SHADOWMAP_SIZE = 1024.f;

	D3DXVECTOR3 vFromPt   = D3DXVECTOR3( -2.075f, 4.167f, 6.401f );
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3( -1.353f, 3.728f, 5.866f );
	g_VCamera.SetViewParams( &vFromPt, &vLookatPt );

	vFromPt = D3DXVECTOR3( -7.735, 8.629f, 11.366f );
	vLookatPt = D3DXVECTOR3( -7.297f, 7.899f, 10.856f );
	g_LCamera.SetViewParams( &vFromPt, &vLookatPt );

	g_bRenderSkyBox = false;

	return cant;
}

int	TestScene2()
{
	// Divan que demuestra el Self Shadow
	int cant = 0;
	g_Obj[cant].nro_mesh = 3;		// "divan.x"
	g_Obj[cant++].m_mWorld =	
		D3DXMATRIXA16(	10, 0, 0, 0,
		0, 10, 0, 0, 
		0, 0, 10, 0, 
		0, 0, 0, 1 );

	g_Obj[cant].nro_mesh = 4;		// "blackholeroom.x"
	g_Obj[cant++].m_mWorld =	
		D3DXMATRIXA16(	4, 0, 0, 0,
		0, 4, 0, 0, 
		0, 0, 10, 0, 
		10, 0, -50, 1 );


	EPSILON_FAR = 0.001f;	
	EPSILON_NEAR = 0.00001f;	
	SHADOWMAP_SIZE = 512.f;

	D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 11.342f, 15.714f, -22.091f );
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 11.756f, 15.401f, -21.0f );
	g_VCamera.SetViewParams( &vFromPt, &vLookatPt );

	vFromPt = D3DXVECTOR3( 28.0f, 15.0f, 5.0f);
	vLookatPt = D3DXVECTOR3( 27.530f, 14.120f, 4.926f);
	g_LCamera.SetViewParams( &vFromPt, &vLookatPt );

	g_bRenderSkyBox = false;

	return cant;
}


int	TestScene1Bis()
{
	// idem test scena1, pero con el point of view y el tamaño del shadow map diferente
	int rta = TestScene1();
	SHADOWMAP_SIZE = 256.f;

	D3DXVECTOR3 vFromPt   = D3DXVECTOR3( -1.701f, 3.448f, 8.098f );
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3( -1.114f, 3.111f, 7.362f );
	g_VCamera.SetViewParams( &vFromPt, &vLookatPt );

	return rta;

}

int	TestScene3()
{
	// Escena con Arboles Demo final
	int cant = 0;
	g_Obj[cant].nro_mesh = 0;		// "plano.x"
	g_Obj[cant++].m_mWorld =	
		D3DXMATRIXA16(	2,0, 0, 0, 
		0, 1, 0, 0, 
		0, 0, 2, 0, 
		0, 0, 0, 1 );

	g_Obj[cant].nro_mesh = 7;		// molino
	g_Obj[cant++].m_mWorld =	
		D3DXMATRIXA16(	1,0, 0, 0, 
		0, 1.2, 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1 );

	g_Obj[cant].nro_mesh = 8;		// molino hastas
	g_Obj[cant++].m_mWorld =	
		D3DXMATRIXA16(	1,0, 0, 0, 
		0, 1, 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1 );

	double alfa = 3.1415/2;
	int cant_col = 10;
	double da = 0.5;
	double r = 35;
	double r2 = 20;
	double r3 = 23;
	float x = -30;

	float random[] = {
		16	,2	,14	,16,	1,
		20	,9	,8	,8	,8,
		12	,9	,19	,14	,19,
		10	,18	,10	,4	,16,
		18	,2	,1	,19	,5,
		16	,3	,11	,13	,9,
		19	,16 };

	for(int i=0;i<cant_col;++i)
	{

		float K = (random[cant%16]-10)/20;
		g_Obj[cant].nro_mesh = 5;		// "arbol.x"
		g_Obj[cant].m_mWorld =	
			D3DXMATRIXA16(	1,0, 0, 0, 
			0, 1.3*(1+K), 0, 0, 
			0, 0, 1, 0, 
			r*sin(alfa-2), 0, r*cos(alfa-2), 1 );
		++cant;

		K = (random[cant%16]-10)/100;
		g_Obj[cant].nro_mesh = 6;		// "tree.x"
		g_Obj[cant].m_mWorld =	
			D3DXMATRIXA16(	0.5,0, 0, 0, 
			0, .5*(1+K), 0, 0, 
			0, 0, 0.5, 0, 
			r2*sin(alfa), 0, r2*cos(alfa), 1 );
		++cant;

		K = random[cant%16]/20;
		g_Obj[cant].nro_mesh = 9;		// "palmera.x"
		g_Obj[cant].m_mWorld =	
			D3DXMATRIXA16(	3,0, 0, 0, 
			0, 3*(1+K), 0, 0, 
			0, 0, 3, 0, 
			r3*sin(alfa/2+4), 0.124824039638042, r3*cos(alfa/2+4), 1 );
		++cant;

		alfa+=da;
		x+=6;
	}

	zbias = 0.000001f;	
	EPSILON_FAR = 0.001f;	
	EPSILON_NEAR = 0.0000001f;	
	SHADOWMAP_SIZE = 1024.f;

	D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 42.871f, 15.233f, 11.806f );
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 41.877f, 15.122f, 11.800f );

	g_VCamera.SetViewParams( &vFromPt, &vLookatPt );

	vFromPt = D3DXVECTOR3( 0.0f, 40.0f, 60.0f );
	vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	g_LCamera.SetViewParams( &vFromPt, &vLookatPt );

	g_bRenderSkyBox = true;

	return cant;
}




int	InitializeScence()
{
	int rta = 0;
	switch(g_nCurrScene)
	{
		case 0:
			// plane, columns,sphere and ring test scene
			rta = TestScene0();
			break;
		case 1:
			rta = TestScene1();
			break;
		case 2:
			rta = TestScene2();
			break;
		case 3:
			rta = TestScene3();
			break;

		case 4:
			rta = TestScene1Bis();
			break;

	}

	return rta;
}


//--------------------------------------------------------------------------------------
// Render the objects
//--------------------------------------------------------------------------------------
void RenderObjects(const D3DXMATRIX *pmView)
{
	g_pEffect->SetInt("cant_obj", NUM_OBJ);
    for( int obj = 0; obj < NUM_OBJ; ++obj )
    {
		g_pEffect->SetMatrix( "g_mWorld", &g_Obj[obj].m_mWorld);
        D3DXMATRIXA16 mWorldView = g_Obj[obj].m_mWorld;
        D3DXMatrixMultiply( &mWorldView, &mWorldView, pmView );
        g_pEffect->SetMatrix( "g_mWorldView", &mWorldView );
		

		int nro_mesh = g_Obj[obj].nro_mesh;
        LPD3DXMESH pMesh = m_Mesh[nro_mesh].GetMesh();
        UINT cPass;
        g_pEffect->Begin( &cPass, 0 );
        for( UINT p = 0; p < cPass; ++p )
        {
            g_pEffect->BeginPass( p );

            for( DWORD i = 0; i < m_Mesh[nro_mesh].m_dwNumMaterials; ++i )
            {
                D3DXVECTOR4 vDif( m_Mesh[nro_mesh].m_pMaterials[i].Diffuse.r,
                                  m_Mesh[nro_mesh].m_pMaterials[i].Diffuse.g,
                                  m_Mesh[nro_mesh].m_pMaterials[i].Diffuse.b,
                                  m_Mesh[nro_mesh].m_pMaterials[i].Diffuse.a );
                g_pEffect->SetVector( "g_vMaterial", &vDif );
                if( m_Mesh[nro_mesh].m_pTextures[i] )
                    g_pEffect->SetTexture( "g_txScene", m_Mesh[nro_mesh].m_pTextures[i] );
                else
                    g_pEffect->SetTexture( "g_txScene", g_pTexDef );

				g_pEffect->SetInt("nro_face", obj*100 + i);

                g_pEffect->CommitChanges();
                pMesh->DrawSubset( i );
            }
            g_pEffect->EndPass();
        }
        g_pEffect->End();
    }
}


//--------------------------------------------------------------------------------------
// Update the light parameters in the effect
//--------------------------------------------------------------------------------------
void UpdateLightParameters(const D3DXMATRIX *pmView)
{
    D3DXVECTOR3 v = *g_LCamera.GetEyePt();
    D3DXVECTOR4 v4;
    D3DXVec3Transform( &v4, &v, pmView );
    g_pEffect->SetVector( "g_vLightPos", &v4 );
    *(D3DXVECTOR3*)&v4 = *g_LCamera.GetWorldAhead();
    v4.w = 0.0f;							// no hay traslacion
    D3DXVec4Transform( &v4, &v4, pmView );  // Direction in view space
    D3DXVec3Normalize( (D3DXVECTOR3*)&v4, (D3DXVECTOR3*)&v4 );
    g_pEffect->SetVector( "g_vLightDir", &v4);
}

//--------------------------------------------------------------------------------------
// Dibujo el artefacto del spotligth
//--------------------------------------------------------------------------------------
void RenderLight( const D3DXMATRIX *pmView, const D3DXMATRIX *pmProj )
{
	g_pEffect->SetTechnique( "RenderLight");

	// Set the projection matrix
	g_pEffect->SetMatrix( "g_mProj", pmProj );
	g_pEffect->SetMatrix( "g_mWorld", g_LCamera.GetWorldMatrix());
	D3DXMATRIXA16 mWorldView = *g_LCamera.GetWorldMatrix();
	D3DXMatrixMultiply( &mWorldView, &mWorldView, pmView );
	g_pEffect->SetMatrix( "g_mWorldView", &mWorldView);

	UINT cPass;
	LPD3DXMESH pMesh = g_LightMesh.GetMesh();
	g_pEffect->Begin( &cPass, 0);
	for( UINT p = 0; p < cPass; ++p )
	{
		g_pEffect->BeginPass( p );

		for( DWORD i = 0; i < g_LightMesh.m_dwNumMaterials; ++i )
		{                                     
			D3DXVECTOR4 vDif(	g_LightMesh.m_pMaterials[i].Diffuse.r,
								g_LightMesh.m_pMaterials[i].Diffuse.g,
								g_LightMesh.m_pMaterials[i].Diffuse.b,
								g_LightMesh.m_pMaterials[i].Diffuse.a );
			g_pEffect->SetVector( "g_vMaterial", &vDif );
			g_pEffect->SetTexture( "g_txScene", g_LightMesh.m_pTextures[i]);
			g_pEffect->CommitChanges();
			pMesh->DrawSubset( i );
		}
		g_pEffect->EndPass();
	}

	// Tambien dibujo el skybox
	if(g_bRenderSkyBox)
	{

		D3DXMATRIXA16 matWorld = D3DXMATRIXA16(	1, 0, 0, 0,
												0, 1, 0, 0, 
												0, 0, 1, 0, 
												0, 0, 0, 1 );

		g_pEffect->SetMatrix( "g_mWorld", &matWorld);
		D3DXMATRIXA16 mWorldView = matWorld;
		D3DXMatrixMultiply( &mWorldView, &mWorldView, pmView );
		g_pEffect->SetMatrix( "g_mWorldView", &mWorldView);
		LPD3DXMESH pMesh = g_SkyBoxMesh.GetMesh();
		for( UINT p = 0; p < cPass; ++p )
		{
			g_pEffect->BeginPass( p );
			for( DWORD i = 0; i < g_SkyBoxMesh.m_dwNumMaterials; ++i )
			{                                     
				D3DXVECTOR4 vDif(	g_SkyBoxMesh.m_pMaterials[i].Diffuse.r,
					g_SkyBoxMesh.m_pMaterials[i].Diffuse.g,
					g_SkyBoxMesh.m_pMaterials[i].Diffuse.b,
					g_SkyBoxMesh.m_pMaterials[i].Diffuse.a );
				g_pEffect->SetVector( "g_vMaterial", &vDif );
				g_pEffect->SetTexture( "g_txScene", g_SkyBoxMesh.m_pTextures[i]);
				g_pEffect->CommitChanges();
				pMesh->DrawSubset( i );
			}
			g_pEffect->EndPass();
		}
	}
	g_pEffect->End();
}

//--------------------------------------------------------------------------------------
// Dibujo el Shadow map
//--------------------------------------------------------------------------------------
void RenderShadowMapView(IDirect3DDevice9* pd3dDevice)
{
	// Compute the view matrix for the light
	D3DXMATRIXA16 mLightView = *g_LCamera.GetViewMatrix();

	// Set technique to render shadow
	g_pEffect->SetTechnique( "RenderShadowView" );
	// Set zbias value to avoid self shadow artifacts
	g_pEffect->SetFloat("EPSILON",zbias);
	g_pEffect->SetFloat("EPSILON_FAR",EPSILON_FAR);
	g_pEffect->SetFloat("EPSILON_NEAR",EPSILON_NEAR);
	g_pEffect->SetFloat("SMAP_SIZE",SHADOWMAP_SIZE);

	// Update the light parameters in the effect
	UpdateLightParameters(&mLightView);
	// Set the projection matrix
	g_pEffect->SetMatrix( "g_mProj", &g_mShadowProj);

	D3DVIEWPORT9 viewport_ant;
	pd3dDevice->GetViewport(&viewport_ant);
	viewport_shadowmap.X = 0;
	viewport_shadowmap.Y = 0;
	viewport_shadowmap.Height = 300;
	viewport_shadowmap.Width = 300;
	viewport_shadowmap.MinZ = 0;
	viewport_shadowmap.MaxZ = 1;
	pd3dDevice->SetViewport(&viewport_shadowmap);

	// Clear the render buffers
	pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0, 1.0f, 0L);
	// Begin the scene
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		// Render Objetcts
		RenderObjects(&mLightView);

		// Draw Frame
		DrawRect(pd3dDevice,0,0,viewport_shadowmap.Width-1,viewport_shadowmap.Height-1);

		// end scene
		pd3dDevice->EndScene();
	}
	pd3dDevice->SetViewport(&viewport_ant);
}

//--------------------------------------------------------------------------------------
// Dibujo un mapa de coordenadas XYZ
//--------------------------------------------------------------------------------------
void RenderSceneMap(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	const D3DXMATRIX *pmView = g_VCamera.GetViewMatrix();
	const D3DXMATRIX *pmProj = g_VCamera.GetProjMatrix();

	g_pEffect->SetTechnique( "RenderXYZMap" );
	// Set the projection matrix
	g_pEffect->SetMatrix( "g_mProj", pmProj );

	// Compute the view matrix for the light
	D3DXMATRIXA16 mLightView = *g_LCamera.GetViewMatrix();
	// Compute the matrix to transform from view space to
	// light projection space.  This consists of
	// the inverse of view matrix * view matrix of light * light projection matrix
	D3DXMATRIXA16 mViewToLightProj;
	mViewToLightProj = *pmView;
	D3DXMatrixInverse( &mViewToLightProj, NULL, &mViewToLightProj );
	D3DXMatrixMultiply( &mViewToLightProj, &mViewToLightProj, &mLightView );
	D3DXMatrixMultiply( &mViewToLightProj, &mViewToLightProj, &g_mShadowProj );
	g_pEffect->SetMatrix( "g_mViewToLightProj", &mViewToLightProj);


	// Render the xyz map
	LPDIRECT3DSURFACE9 pOldRT = NULL;
	V( pd3dDevice->GetRenderTarget( 0, &pOldRT ) );
	LPDIRECT3DSURFACE9 pXYZSurf;
	if( SUCCEEDED( g_pXYZMap->GetSurfaceLevel( 0, &pXYZSurf) ) )
		pd3dDevice->SetRenderTarget( 0, pXYZSurf);
	LPDIRECT3DSURFACE9 pOldDS = NULL;
	if( SUCCEEDED( pd3dDevice->GetDepthStencilSurface( &pOldDS ) ) )
		pd3dDevice->SetDepthStencilSurface( g_pDSXYZ);
	
	// Clear the render buffers
	pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L);
	// Begin the scene
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		RenderObjects(pmView);
		pd3dDevice->EndScene();
	}

	//D3DXSaveTextureToFile(L"test2.bmp",D3DXIFF_BMP,g_pXYZMap,NULL);

	SAFE_RELEASE( pXYZSurf);
	pd3dDevice->SetDepthStencilSurface( pOldDS );
	pd3dDevice->SetRenderTarget( 0, pOldRT );

	SAFE_RELEASE( pOldDS );
	SAFE_RELEASE( pOldRT );


}


//--------------------------------------------------------------------------------------
// Dibujo el ID map ( a efectos didacticos)
//--------------------------------------------------------------------------------------
void RenderIDView(IDirect3DDevice9* pd3dDevice)
{
	// Compute the view matrix for the light
	D3DXMATRIXA16 mLightView = *g_LCamera.GetViewMatrix();

	// Set technique to render shadow
	g_pEffect->SetTechnique( "RenderIDView" );
	// Set zbias value to avoid self shadow artifacts
	g_pEffect->SetFloat("EPSILON",zbias);
	g_pEffect->SetFloat("EPSILON_FAR",EPSILON_FAR);
	g_pEffect->SetFloat("EPSILON_NEAR",EPSILON_NEAR);
	g_pEffect->SetFloat("SMAP_SIZE",SHADOWMAP_SIZE);
	// Update the light parameters in the effect
	UpdateLightParameters(&mLightView);
	// Set the projection matrix
	g_pEffect->SetMatrix( "g_mProj", &g_mShadowProj);

	D3DVIEWPORT9 viewport_ant;
	pd3dDevice->GetViewport(&viewport_ant);
	viewport_shadowmap.X = 0;
	viewport_shadowmap.Y = g_bDebugShadowMap?300:0;
	viewport_shadowmap.Height = 300;
	viewport_shadowmap.Width = 300;
	viewport_shadowmap.MinZ = 0;
	viewport_shadowmap.MaxZ = 1;
	pd3dDevice->SetViewport(&viewport_shadowmap);

	// Clear the render buffers
	pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0, 1.0f, 0L);
	// Begin the scene
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		// Render Objetcts
		RenderObjects(&mLightView);
		// Draw Frame
		DrawRect(pd3dDevice,0,0,viewport_shadowmap.Width-1,viewport_shadowmap.Height-1);
		// end scene
		pd3dDevice->EndScene();
	}
	pd3dDevice->SetViewport(&viewport_ant);
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{

	expo_elapsed_time = fTime;

	// Parche par que el DXUT sea orientado a los eventos del mouse
	// solo dibuja cuando hay movimiento. 
	// Esto permite fijar un screen shoot (ya que los algoritmos no son en real time)
	if(_modo_interactivo && !_redraw_window)
		return;

    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

	Update( pd3dDevice, fTime, fElapsedTime);

    HRESULT hr;

	DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
	UINT sdx = d3dSettings.pp.BackBufferWidth;
	UINT sdy = d3dSettings.pp.BackBufferHeight;
	

    // Compute the view matrix for the light
    D3DXMATRIXA16 mLightView = *g_LCamera.GetViewMatrix();
    
    // Render the shadow map
    LPDIRECT3DSURFACE9 pOldRT = NULL;
    V( pd3dDevice->GetRenderTarget( 0, &pOldRT ) );
    LPDIRECT3DSURFACE9 pShadowSurf;
    if( SUCCEEDED( g_pShadowMap->GetSurfaceLevel( 0, &pShadowSurf ) ) )
    {
        pd3dDevice->SetRenderTarget( 0, pShadowSurf );
        SAFE_RELEASE( pShadowSurf );
    }
    LPDIRECT3DSURFACE9 pOldDS = NULL;
    if( SUCCEEDED( pd3dDevice->GetDepthStencilSurface( &pOldDS ) ) )
        pd3dDevice->SetDepthStencilSurface( g_pDSShadow );

    {
		// Set technique to render shadow
        g_pEffect->SetTechnique( "RenderShadow" );
		// Set zbias value to avoid self shadow artifacts
		g_pEffect->SetFloat("EPSILON",zbias);
		g_pEffect->SetFloat("EPSILON_FAR",EPSILON_FAR);
		g_pEffect->SetFloat("EPSILON_NEAR",EPSILON_NEAR);
		// Set shadow map size
		g_pEffect->SetFloat("SMAP_SIZE",SHADOWMAP_SIZE);
		// Update the light parameters in the effect
		UpdateLightParameters(&mLightView);
		// Set the projection matrix
		g_pEffect->SetMatrix( "g_mProj", &g_mShadowProj);
		// luz ambiente global
		g_pEffect->SetFloat("k_la",g_fAmbient);
		// Clear the render buffers
		pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0, 1.0f, 0L);
	    // Begin the scene
	    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	    {
			// Render Objetcts
			RenderObjects(&mLightView);
			// end scene
			pd3dDevice->EndScene();
		}
    }

    if( pOldDS )
    {
        pd3dDevice->SetDepthStencilSurface( pOldDS );
        pOldDS->Release();
    }
    pd3dDevice->SetRenderTarget( 0, pOldRT );
    SAFE_RELEASE( pOldRT );

	// Debug: render xyz map
	// --------------------------
	RenderSceneMap(pd3dDevice);
	
	// Now that we have the shadow map, render the scene.
	// --------------------------
    const D3DXMATRIX *pmView = g_VCamera.GetViewMatrix();
	D3DXMATRIX auxProj = *g_VCamera.GetProjMatrix();
	D3DXMATRIX *pmProj = &auxProj;

    // Initialize required parameter
    g_pEffect->SetTexture( "g_txShadow", g_pShadowMap);

    // Compute the matrix to transform from view space to
    // light projection space.  This consists of
    // the inverse of view matrix * view matrix of light * light projection matrix
    D3DXMATRIXA16 mViewToLightProj;
    mViewToLightProj = *pmView;
    D3DXMatrixInverse( &mViewToLightProj, NULL, &mViewToLightProj );
    D3DXMatrixMultiply( &mViewToLightProj, &mViewToLightProj, &mLightView );
    D3DXMatrixMultiply( &mViewToLightProj, &mViewToLightProj, &g_mShadowProj );
    g_pEffect->SetMatrix( "g_mViewToLightProj", &mViewToLightProj);

	// si hay comparativa, (siempre entre 2 metodos), cada metodo ocupa la mitad vertical
	// de la pantalla, para mantener el aspect ratio: 
	if(g_bComparativa)
		pmProj->_11 *= 2;


	// Clear the render buffers
	pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L);

	// la comparativa puede ser entre el standard shadow map y el combined id
	// o bien entre el id standard, y el combined id. 
	// El combined id va siempre a la derecha y el standard shadow map tiene prioridad sobre
	// el solo ids. 

	// Set the projection matrix
	g_pEffect->SetMatrix( "g_mProj", pmProj );

	g_pEffect->SetBool("g_bLuminance",g_bLuminance);
	// Update the light parameters in the effect
	UpdateLightParameters(pmView);

	D3DVIEWPORT9 viewport_ant,viewport;
	viewport_ant.X = 0;
	viewport_ant.Y = 0;
	viewport_ant.Height = sdy-35;
	viewport_ant.Width = sdx;
	viewport_ant.MinZ = 0;
	viewport_ant.MaxZ = 1;
	pd3dDevice->SetViewport(&viewport_ant);

	if(g_bStandardShadowMap || (g_bComparativa && !g_bIDsShadowMap))
	{
		// Standard Shadow map
		if(g_bComparativa)
		{
			viewport.X = 0;
			viewport.Y = 0;
			viewport.Height = viewport_ant.Height;
			viewport.Width = viewport_ant.Width/2;
			viewport.MinZ = 0;
			viewport.MaxZ = 1;
			pd3dDevice->SetViewport(&viewport);
		}
		else
			viewport = viewport_ant;

		g_pEffect->SetTechnique( "RenderSceneStd" );

		// Clear the render buffers
		//pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L);
		// Begin the scene
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			// Render Objetcts
			RenderObjects(pmView);
			// Render light
			RenderLight(pmView, pmProj );

			DrawRect(pd3dDevice,0,0,viewport.Width-1,viewport.Height-1);

			if(g_bShowText)
			{
				CDXUTTextHelper txtHelper( g_pFont , g_pTextSprite, 25 );
				txtHelper.Begin();
				txtHelper.SetInsertionPos( 5, viewport_ant.Height - 25 );
				txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
				txtHelper.DrawTextLine(L"Standard Shadow Map");
				txtHelper.End();
			}

			// end scene
			pd3dDevice->EndScene();
		}

	}


	// el standard shadow map tiene prioridad sobre el id shadow map
	if( !g_bStandardShadowMap && g_bIDsShadowMap)
	{
		// ID Shadow map
		if(g_bComparativa)
		{
			viewport.X = 0;
			viewport.Y = 0;
			viewport.Height = viewport_ant.Height;
			viewport.Width = viewport_ant.Width/2;
			viewport.MinZ = 0;
			viewport.MaxZ = 1;
			pd3dDevice->SetViewport(&viewport);
		}
		else
			viewport = viewport_ant;


		g_pEffect->SetTechnique( "RenderSceneSoloIDs" );
		// Clear the render buffers
		//pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L);
		// Begin the scene
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			// Render Objetcts
			RenderObjects(pmView);
			// Render light
			RenderLight(pmView, pmProj );
			DrawRect(pd3dDevice,0,0,viewport.Width-1,viewport.Height-1);

			if(g_bShowText)
			{
				CDXUTTextHelper txtHelper( g_pFont , g_pTextSprite, 25 );
				txtHelper.Begin();
				txtHelper.SetInsertionPos( 5, viewport_ant.Height - 25 );
				txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
				txtHelper.DrawTextLine(L"ID Shadow map");
				txtHelper.End();
			}

			// end scene
			pd3dDevice->EndScene();
		}
	}

	if(g_bComparativa || (!g_bStandardShadowMap && !g_bIDsShadowMap))
	{
		// Combined Id Shadow map
		g_pEffect->SetTechnique( "RenderScene" );
		int pos_x = 0;
		if(g_bComparativa)
		{
			pos_x = viewport.X = viewport_ant.Width/2;
			viewport.Y = 0;
			viewport.Height = viewport_ant.Height;
			viewport.Width = viewport_ant.Width/2;
			pd3dDevice->SetViewport(&viewport);
		}
		else
			viewport = viewport_ant;

		// Clear the render buffers
		//pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L);
		// Begin the scene
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			// Render Objetcts
			RenderObjects(pmView);

			// Render light
			RenderLight(pmView, pmProj );

			DrawRect(pd3dDevice,0,0,viewport.Width-1,viewport.Height-1);

			if(g_bShowText)
			{
				CDXUTTextHelper txtHelper( g_pFont , g_pTextSprite, 25 );
				txtHelper.Begin();
				txtHelper.SetInsertionPos( pos_x + 5, viewport_ant.Height - 25 );
				txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
				txtHelper.DrawTextLine(L"Combined ID");
				txtHelper.End();
			}

			// end scene
			pd3dDevice->EndScene();
		}
	}

	// uso toda la pantalla ahora
	viewport_ant.Height = sdy;
	pd3dDevice->SetViewport(&viewport_ant);

	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		// Render stats and help text
		if(g_bShowText)
		{
			RenderText();
			// Render the UI elements
			g_HUD.OnRender( fElapsedTime );
			g_HDiapo.OnRender( fElapsedTime );
		}
		// end scene
		pd3dDevice->EndScene();
	}

	// debug del shadow map
	// 
	if(g_bDebugShadowMap && !g_bComparativa)
	{

		// Render Shadow map preview
		//
		RenderShadowMapView(pd3dDevice);

		// Debug del pixel actual
		//
		LPDIRECT3DSURFACE9 pXYZSurface;
		g_pXYZMap->GetSurfaceLevel(0,&pXYZSurface);
		D3DSURFACE_DESC desc;
		pXYZSurface->GetDesc(&desc);
		// textura temporaria para leer las coordenadas
		IDirect3DTexture9* pTempTexture=NULL;
		pd3dDevice->CreateTexture(desc.Width,desc.Height,1,0,desc.Format,D3DPOOL_SYSTEMMEM,&pTempTexture,NULL);
		IDirect3DSurface9* pTempSurface=NULL;
		pTempTexture->GetSurfaceLevel(0,&pTempSurface);
		pd3dDevice->GetRenderTargetData(pXYZSurface,pTempSurface);
		D3DLOCKED_RECT rc;
		pTempSurface->LockRect(&rc,NULL,D3DLOCK_DISCARD);
		BYTE *bytes = (BYTE *)rc.pBits;			// xyz map
		POINT CursorPos;
		GetCursorPos(&CursorPos);
		ScreenToClient(d3dSettings.pp.hDeviceWindow,&CursorPos);

		float fil = (float)CursorPos.y;
		float col = (float)CursorPos.x;
		if(fil>=0 && fil<desc.Height-1 && col>=0 && col<desc.Width-150)		// 150 = margen de los controles
		{
			FLOAT *pixel = (FLOAT *)(bytes+(int)fil*rc.Pitch + (int)col*16);
			float X = (float)pixel[0];
			float Y = (float)pixel[1];
			float Z = (float)pixel[2];
			float W = (float)pixel[3];

			CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 25 );
			txtHelper.Begin();
			txtHelper.SetInsertionPos( 305, 5 );
			txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
			WCHAR wszText[50];
			StringCchPrintf( wszText, 50, L"X=%g Y=%g Z=%g",X,Y,Z);
			txtHelper.DrawTextLine(wszText);

			LPD3DXLINE ppLine;
			D3DXCreateLine(pd3dDevice,&ppLine);
			ppLine->Begin();
			D3DXVECTOR2 V[] = {
					D3DXVECTOR2(col-5,fil-5),
					D3DXVECTOR2(col+5,fil-5),
					D3DXVECTOR2(col+5,fil+5),
					D3DXVECTOR2(col-5,fil+5),
					D3DXVECTOR2(col-5,fil-5) };
			ppLine->Draw(V,5,D3DCOLOR_XRGB(255,0,255));

			// Busco este punto en el shadow map
			D3DXMATRIXA16 mLightView = *g_LCamera.GetViewMatrix();
			D3DXMATRIXA16 matWorld;
			D3DXMatrixIdentity(&matWorld);
			D3DXVECTOR3 pto_shadow;
			D3DXVECTOR3 pV(X,Y,Z);
			D3DXVec3Project(&pto_shadow,&pV,&viewport_shadowmap,&g_mShadowProj,&mLightView,&matWorld);
			col = pto_shadow.x;
			fil = pto_shadow.y;

			// dibujo el texel en el shadow map
			D3DXVECTOR2 V2[] = {
				D3DXVECTOR2(col-5,fil-5),
				D3DXVECTOR2(col+5,fil-5),
				D3DXVECTOR2(col+5,fil+5),
				D3DXVECTOR2(col-5,fil+5),
				D3DXVECTOR2(col-5,fil-5) };
			ppLine->Draw(V2,5,D3DCOLOR_XRGB(255,0,255));

			int f = (int) (fil - viewport_shadowmap.Y);
			int c = (int) (col - viewport_shadowmap.X);
			// ajusto a la textura
			c = (int)((float)c / (float)viewport_shadowmap.Width * SHADOWMAP_SIZE);
			f = (int)((float)f / (float)viewport_shadowmap.Height * SHADOWMAP_SIZE);
			
			if (c >= 0 && c < SHADOWMAP_SIZE && f >= 0 && f < SHADOWMAP_SIZE)
			{
				LPDIRECT3DSURFACE9 smap_surf;
				g_pShadowMap->GetSurfaceLevel(0,&smap_surf);
				LPDIRECT3DTEXTURE9 temp2;
				pd3dDevice->CreateTexture( (UINT)SHADOWMAP_SIZE, (UINT)SHADOWMAP_SIZE,
						1, 0 ,D3DFMT_G32R32F,D3DPOOL_SYSTEMMEM,&temp2,NULL);

				LPDIRECT3DSURFACE9 surf2;
				temp2->GetSurfaceLevel(0,&surf2);
				pd3dDevice->GetRenderTargetData(smap_surf, surf2);
				D3DLOCKED_RECT rc2;
				surf2->LockRect(&rc2,NULL,D3DLOCK_DISCARD);
				BYTE *bytes2 = (BYTE *)rc2.pBits;			// shadow map
				FLOAT *pixel2 = (FLOAT *)(bytes2+f*rc2.Pitch + c*8);
				float Wsmap = (float)pixel2[1];
				int id_face = (int)pixel2[0];

				float prof = Zn + (Zf- Zn) * W;
				float prof_smap = Zn + (Zf- Zn) * Wsmap;
				BOOL sombra = W > Wsmap + zbias ? TRUE:FALSE;
				StringCchPrintf( wszText, 50, L"Prof=%g Prof Smap=%g Id=%d %s",
						prof,
						prof_smap,(int)id_face,
						sombra?L"  en Sombra":L" Iluminado");
				txtHelper.DrawTextLine(wszText);

				surf2->UnlockRect();
				SAFE_RELEASE(surf2);
				SAFE_RELEASE(temp2);
				SAFE_RELEASE(smap_surf);


			}
			ppLine->End();
			txtHelper.End();
			SAFE_RELEASE(ppLine);


		}

		pTempSurface->UnlockRect();
		SAFE_RELEASE (pTempSurface);
		SAFE_RELEASE (pTempTexture);
		SAFE_RELEASE (pXYZSurface);

	}

	if(g_bShowIDMap)
		RenderIDView(pd3dDevice);


    g_pEffect->SetTexture( "g_txShadow", NULL );


	// dibujo la diapositiva actual
	if(nro_diapo)
	{
		char saux[255];
		sprintf(saux,"Diapositiva\\diapositiva%d.bmp",nro_diapo);
		DrawDiapositiva(pd3dDevice,saux,fElapsedTime);
	}
}

HRESULT CreateShadowMap(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
    // Create the shadow map texture D3DFMT_R32F o D3DFMT_G32R32F
	// this is NOT an standard shadow map: 
	// in the channel R we store the id of the face (nro_face)
	// in the channel G we store the usual depth. 
	// This allows for special comparison with the depth AND the obj id,
	// which prevents for self shadowing. 
    V_RETURN( pd3dDevice->CreateTexture( (UINT)SHADOWMAP_SIZE, (UINT)SHADOWMAP_SIZE,
                                         1, D3DUSAGE_RENDERTARGET,
                                         D3DFMT_G32R32F,
                                         D3DPOOL_DEFAULT,
                                         &g_pShadowMap,
                                         NULL ) );

    // Create the depth-stencil buffer to be used with the shadow map
	// No se puede usar el depth stencil automatico, porque no tiene la misma medida 
	// que el shadow map, ademas del problema del multisampling. 
	// Los render to texture en dx9 no andan con el multisampling. 
    DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
    V_RETURN( pd3dDevice->CreateDepthStencilSurface( (UINT)SHADOWMAP_SIZE,(UINT)SHADOWMAP_SIZE,
                                                     d3dSettings.pp.AutoDepthStencilFormat,
                                                     D3DMULTISAMPLE_NONE,
                                                     0,
                                                     TRUE,
                                                     &g_pDSShadow,
                                                     NULL ) );



	return S_OK;
}



HRESULT CreateXYZMap(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
	UINT sdx = d3dSettings.pp.BackBufferWidth;
	UINT sdy = d3dSettings.pp.BackBufferHeight - 35;
	// Create xyz map
	V_RETURN( pd3dDevice->CreateTexture(sdx,sdy,
			 1, D3DUSAGE_RENDERTARGET,
			 D3DFMT_A32B32G32R32F,
			 D3DPOOL_DEFAULT,
			 &g_pXYZMap,
			 NULL ) );

	V_RETURN( pd3dDevice->CreateDepthStencilSurface( sdx,sdy,
			d3dSettings.pp.AutoDepthStencilFormat,
			 D3DMULTISAMPLE_NONE,0,TRUE,
			 &g_pDSXYZ,
			 NULL ) );

	return S_OK;
}

HRESULT MyResetDevice(IDirect3DDevice9* pd3dDevice,const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
    // Restore the scene objects
    for( int i = 0; i < cant_mesh; ++i )
        m_Mesh[i].RestoreDeviceObjects( pd3dDevice);
	g_LightMesh.RestoreDeviceObjects( pd3dDevice);
	g_SkyBoxMesh.RestoreDeviceObjects( pd3dDevice);

    // Restore the effect variables
    g_pEffect->SetVector( "g_vLightDiffuse", (D3DXVECTOR4 *)&g_Light.Diffuse);
    g_pEffect->SetFloat( "g_fCosTheta", cosf( g_Light.Theta ) );

	// Create shadow maps
	CreateShadowMap(pd3dDevice);

	// Create xyz maps
	CreateXYZMap(pd3dDevice);

    // Initialize the shadow projection matrix
    D3DXMatrixPerspectiveFovLH( &g_mShadowProj, g_fLightFov, 1, Zn, Zf);

	// Relocate the gui controls to the right
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, pBackBufferSurfaceDesc->Height );

	g_HDiapo.SetLocation( 10,pBackBufferSurfaceDesc->Height-30 );
	g_HDiapo.SetSize( pBackBufferSurfaceDesc->Width-300,30);

	// Not backface culling 
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);

	return S_OK;
}


void MyLostDevice()
{
	// shadow map
    SAFE_RELEASE( g_pDSShadow );
    SAFE_RELEASE( g_pShadowMap );

	// xyz map
	SAFE_RELEASE( g_pDSXYZ );
	SAFE_RELEASE( g_pXYZMap );
	
	if( g_pEffect )
        g_pEffect->OnLostDevice();

    for( int i = 0; i < cant_mesh; ++i )
        m_Mesh[i].InvalidateDeviceObjects();
	g_LightMesh.InvalidateDeviceObjects();
	g_SkyBoxMesh.InvalidateDeviceObjects();

	// Diapo
	SAFE_RELEASE(g_pDiapositiva);

}


void MyDestroyDevice()
{
    SAFE_RELEASE( g_pDSShadow );
    SAFE_RELEASE( g_pShadowMap );
    SAFE_RELEASE( g_pEffect );

    for( int i = 0; i < cant_mesh; ++i )
        m_Mesh[i].Destroy();
    g_LightMesh.Destroy();
	g_SkyBoxMesh.Destroy();
}


// Helper
void DrawRect(IDirect3DDevice9* pd3dDevice,int x0,int y0,int x1,int y1,D3DCOLOR color)
{
	LPD3DXLINE ppLine;
	D3DXCreateLine(pd3dDevice,&ppLine);
	ppLine->Begin();
	D3DXVECTOR2 V[] = {
		D3DXVECTOR2(x0,y0),
		D3DXVECTOR2(x1,y0),
		D3DXVECTOR2(x1,y1),
		D3DXVECTOR2(x0,y1),
		D3DXVECTOR2(x0,y0) };
	
	ppLine->Draw(V,5,color);
	ppLine->End();
	SAFE_RELEASE(ppLine);


}


void Update( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime)
{
	switch(g_nCurrScene)
	{
		case 3:
			// Animo la fuente luminosa
			if(g_bAnimarSol)
			{
				float r = 120;
				float alfa = fTime * 0.7;
				D3DXVECTOR3 vFromPt = D3DXVECTOR3( r*cos(alfa), abs(r*sin(alfa)), 0);
				D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
				g_LCamera.SetViewParams( &vFromPt, &vLookatPt );
			}

			// Animo las hastas del molino
			// 
			if(g_bAnimarSol)
			{
				float desf_h = 2;
				float x0 = (6.41744756698608 + 5.93245124816895)/2;
				float y0 = (7.88684988021851 + 7.69079256057739)/2 + desf_h;
				float z0 = (7.19669389724731 + 7.51336574554443)/2;

				//D3DXMatrixIdentity(&g_Obj[2].m_mWorld);
				g_Obj[2].m_mWorld= D3DXMATRIX(
										1,	0,	0,	0,
										0,	1,	0,	0,
										0,	0,	1,	0,
										0,	desf_h,	0,	1);

				D3DXVECTOR3 v =  D3DXVECTOR3(x0,0.2,z0);
				D3DXVec3Normalize(&v,&v);

				D3DXVECTOR3 P0 =  D3DXVECTOR3(x0,y0,z0);

				D3DXMATRIXA16 T0;
				D3DXMatrixTranslation(&T0,-P0.x,-P0.y,-P0.z);
				D3DXMatrixMultiply(&g_Obj[2].m_mWorld,&g_Obj[2].m_mWorld,&T0);

				D3DXMATRIXA16 Rot;
				D3DXMatrixRotationAxis(&Rot,&v,fTime);
				D3DXMatrixMultiply(&g_Obj[2].m_mWorld,&g_Obj[2].m_mWorld,&Rot);

				P0 =  P0 + v*0.2f;

				D3DXMatrixTranslation(&T0,P0.x,P0.y,P0.z);
				D3DXMatrixMultiply(&g_Obj[2].m_mWorld,&g_Obj[2].m_mWorld,&T0);

			}

			break;
	}

}

void DrawDiapositiva(IDirect3DDevice9* pd3dDevice,char *fname,float elapsedtime)
{
	DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
	UINT sdx = d3dSettings.pp.BackBufferWidth;
	UINT sdy = d3dSettings.pp.BackBufferHeight;

	if(!strlen(fname_diapositiva) || strcmp(fname_diapositiva,fname)!=0 || !g_pDiapositiva)	
	{
		// Tengo que cargar la diapositiva en memoria
		// Primero libero la anterior
		SAFE_RELEASE(g_pDiapositiva);
		D3DXCreateTextureFromFileA(pd3dDevice, fname, &g_pDiapositiva);

		// Actualizo el file name
		strcpy(fname_diapositiva,fname);

		// Reseteo el elapsed time
		diapo_elapsed_time = 0;
	}

	// Dibujo la diapositiva pp dicha

	if(g_pDiapositiva)
	{
		D3DXMATRIXA16 trans,ant_trans;
		g_pTextSprite->GetTransform(&ant_trans);
		D3DXMatrixIdentity(&trans);
		// determino la escala para que entre justo en toda la pantalla
		D3DSURFACE_DESC pDesc;
		g_pDiapositiva->GetLevelDesc(0,&pDesc);
		trans._11 *=  (float)sdx / (float)pDesc.Width ;
		trans._22 *=  (float)sdy / (float)pDesc.Height * 0.95;
		D3DXVECTOR3 vPos((float)pDesc.Width * 0 , (float)pDesc.Height * 0 , 0);


		diapo_elapsed_time += elapsedtime;

		if(diapo_elapsed_time>1)
			diapo_elapsed_time = 1;

		g_pTextSprite->SetTransform(&trans);
		g_pTextSprite->Begin(D3DXSPRITE_ALPHABLEND);
		g_pTextSprite->Draw(g_pDiapositiva,NULL,NULL,&vPos,D3DCOLOR_RGBA(255,255,255,(BYTE)(255*diapo_elapsed_time)));
		g_pTextSprite->End();
		g_pTextSprite->SetTransform(&ant_trans);
	}
}

