
float SMAP_SIZE = 512;
float EPSILON		= 0.00001f;
float EPSILON_NEAR	= 0.00001f;
float EPSILON_FAR	= 0.001f;

//-----------------------------------------------------------------------------
float4x4 g_mWorld;
float4x4 g_mWorldView;
float4x4 g_mProj;
float4x4 g_mViewToLightProj;  // Transform from view space to light projection space
float4   g_vMaterial;
texture  g_txScene;
texture  g_txShadow;
float3   g_vLightPos;  // Light position in view space
float3   g_vLightDir;  // Light direccion in view space
float4   g_vLightDiffuse = float4( 1.0f, 1.0f, 1.0f, 1.0f );  // Light diffuse color
float k_la = 0.3f;
float    g_fCosTheta;  // Cosine of theta of the spot light
int		 nro_face;		// Help to avoid the self shadow artifact
bool	 g_bLuminance = false;		// Draw only Light intesity 
int		 cant_obj;		// Cantidad total de objetos


//-----------------------------------------------------------------------------
sampler2D g_samScene =
sampler_state
{
    Texture = <g_txScene>;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};

sampler2D g_samShadow =
sampler_state
{
    Texture = <g_txShadow>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
    AddressU = Clamp;
    AddressV = Clamp;
};


//-----------------------------------------------------------------------------
// Vertex Shader: VertScene
// Desc: Process vertex for scene
//-----------------------------------------------------------------------------
void VertScene( float4 iPos : POSITION,
                float3 iNormal : NORMAL,
                float2 iTex : TEXCOORD0,
                out float4 oPos : POSITION,
                out float2 Tex : TEXCOORD0,
                out float4 vPos : TEXCOORD1,
                out float3 vNormal : TEXCOORD2,
                out float4 vPosLight : TEXCOORD3)
{
    vPos = mul( iPos, g_mWorldView );					// Transform position to view space
    oPos = mul( vPos, g_mProj );						// Transform to screen coord
    vNormal = mul( iNormal, (float3x3)g_mWorldView );	// Compute view space normal
    Tex = iTex;											// Propagate texture coord
    vPosLight = mul( vPos, g_mViewToLightProj );		// Transform the position to light projection space
}


//-----------------------------------------------------------------------------
// Standard Shadow map
//-----------------------------------------------------------------------------
float4 PixSceneStd( float2 Tex : TEXCOORD0,
                 float4 vPos : TEXCOORD1,
                 float3 vNormal : TEXCOORD2,
                 float4 vPosLight : TEXCOORD3 ) : COLOR
{
	float4   g_vLightAmbient = float4( k_la, k_la, k_la, 1.0f );  
	float4 color_base = tex2D( g_samScene, Tex );
	//if(color_base.r==1 && color_base.g==0 && color_base.b==1)
		//discard;
    float4 Diffuse;
    float3 vLight = normalize( float3( vPos - g_vLightPos ) );		// vLight is the unit vector from the light to this pixel
	float I = 0;
	
    // Compute diffuse from the light
    if( dot( vLight, g_vLightDir) > g_fCosTheta ) // Light must face the pixel (within Theta)
    {
        //transform from RT space to texture space.
        float2 CT = 0.5 * vPosLight.xy / vPosLight.w + float2( 0.5, 0.5 );
        CT.y = 1.0f - CT.y;

        // transform to texel space
        float2 texelpos = SMAP_SIZE * CT;
        
        // Determine the lerp amounts           
        float2 lerps = frac( texelpos );

        //read in bilerp stamp, doing the shadow checks
        float sourcevals[4];
        sourcevals[0] = (tex2D( g_samShadow, CT ).g + EPSILON < vPosLight.z / vPosLight.w)? 0.0f: 1.0f;  
        sourcevals[1] = (tex2D( g_samShadow, CT+ float2(1.0/SMAP_SIZE, 0) ).g + EPSILON < vPosLight.z / vPosLight.w)? 0.0f: 1.0f;  
        sourcevals[2] = (tex2D( g_samShadow, CT+ float2(0, 1.0/SMAP_SIZE) ).g + EPSILON < vPosLight.z / vPosLight.w)? 0.0f: 1.0f;  
        sourcevals[3] = (tex2D( g_samShadow, CT + float2(1.0/SMAP_SIZE, 1.0/SMAP_SIZE) ).g + EPSILON < vPosLight.z / vPosLight.w)? 0.0f: 1.0f;  
        // lerp between the shadow values to calculate our light amount
        I = lerp( lerp( sourcevals[0], sourcevals[1], lerps.x ),
                                  lerp( sourcevals[2], sourcevals[3], lerps.x ),
                                  lerps.y );
        // Light it
        Diffuse = ( saturate( dot( -vLight, normalize( vNormal)) ) * I * ( 1 - g_vLightAmbient ) + g_vLightAmbient )
                  * g_vMaterial;
		       
    } 
    else
    {
        Diffuse = g_vLightAmbient * g_vMaterial;
    }

	return g_bLuminance?float4(I,I,I,1):color_base * Diffuse;
}



//-----------------------------------------------------------------------------
// Solo IDs
//-----------------------------------------------------------------------------
float4 PixSceneSoloIDs( float2 Tex : TEXCOORD0,
                 float4 vPos : TEXCOORD1,
                 float3 vNormal : TEXCOORD2,
                 float4 vPosLight : TEXCOORD3 ) : COLOR
{
	float4   g_vLightAmbient = float4( k_la, k_la, k_la, 1.0f );  
	float4 color_base = tex2D( g_samScene, Tex );
	if(color_base.r==1 && color_base.g==0 && color_base.b==1)
		discard;
    float4 Diffuse;
    float3 vLight = normalize( float3( vPos - g_vLightPos ) );		// vLight is the unit vector from the light to this pixel
	float I = 0;
	
    // Compute diffuse from the light
    if( dot( vLight, g_vLightDir) > g_fCosTheta ) // Light must face the pixel (within Theta)
    {
        //transform from RT space to texture space.
        float2 CT = 0.5 * vPosLight.xy / vPosLight.w + float2( 0.5, 0.5 );
        CT.y = 1.0f - CT.y;
		float2 vecino = frac( CT*SMAP_SIZE);
		
		int cara_0 = round(tex2D(g_samShadow,float2(CT)).r);
		int cara_1 = round(tex2D(g_samShadow,float2(CT) + float2(1.0/SMAP_SIZE,0)).r);
		int cara_2 = round(tex2D(g_samShadow,float2(CT) + float2(0,1.0/SMAP_SIZE)).r);
		int cara_3 = round(tex2D(g_samShadow,float2(CT) + float2(1.0/SMAP_SIZE,1.0/SMAP_SIZE)).r);

		float s0 = cara_0==nro_face? 1 : 0.0f;
		float s1 = cara_1==nro_face? 1 : 0.0f;
		float s2 = cara_2==nro_face? 1 : 0.0f;
		float s3 = cara_3==nro_face? 1 : 0.0f;

		I = lerp( lerp( s0, s1, vecino.x ),lerp( s2, s3, vecino.x ),vecino.y);		
        
        // Light it
        Diffuse = ( saturate( dot( -vLight, normalize( vNormal) ) ) * I * ( 1 - g_vLightAmbient ) + g_vLightAmbient )
                  * g_vMaterial;
		       
    } 
    else
    {
        Diffuse = g_vLightAmbient * g_vMaterial;
    }

	
	return g_bLuminance?float4(I,I,I,1):color_base * Diffuse;
}


//-----------------------------------------------------------------------------
// Combined ID
//-----------------------------------------------------------------------------
float4 PixScene( float2 Tex : TEXCOORD0,
                 float4 vPos : TEXCOORD1,
                 float3 vNormal : TEXCOORD2,
                 float4 vPosLight : TEXCOORD3 ) : COLOR
{
	float4   g_vLightAmbient = float4( k_la, k_la, k_la, 1.0f );  
	float4 color_base = tex2D( g_samScene, Tex );
	if(color_base.r==1 && color_base.g==0 && color_base.b==1)
		discard;
    float4 Diffuse;
    float3 vLight = normalize( float3( vPos - g_vLightPos ) );		// vLight is the unit vector from the light to this pixel
	float I = 0;
	
    // Compute diffuse from the light
    if( dot( vLight, g_vLightDir) > g_fCosTheta ) // Light must face the pixel (within Theta)
    {
        //transform from RT space to texture space.
        float2 CT = 0.5 * vPosLight.xy / vPosLight.w + float2( 0.5, 0.5 );
        CT.y = 1.0f - CT.y;
		float2 vecino = frac( CT*SMAP_SIZE);
		
		int cara_0 = round(tex2D(g_samShadow,float2(CT)).r);
		int cara_1 = round(tex2D(g_samShadow,float2(CT) + float2(1.0/SMAP_SIZE,0)).r);
		int cara_2 = round(tex2D(g_samShadow,float2(CT) + float2(0,1.0/SMAP_SIZE)).r);
		int cara_3 = round(tex2D(g_samShadow,float2(CT) + float2(1.0/SMAP_SIZE,1.0/SMAP_SIZE)).r);

		float prof_0 = tex2D( g_samShadow, float2(CT)).g;
		float prof_1 = tex2D( g_samShadow, float2(CT) + float2(1.0/SMAP_SIZE,0)).g;
		float prof_2 = tex2D( g_samShadow, float2(CT) + float2(0,1.0/SMAP_SIZE)).g;
		float prof_3 = tex2D( g_samShadow, float2(CT) + float2(1.0/SMAP_SIZE,1.0/SMAP_SIZE)).g;

		float prof = vPosLight.z / vPosLight.w;
		
		// soporte para self shadows. Hay mallas que tienen muchos layers todos con el mismo id, o layers grandes
		// que si tienen que producir sombra sobre si mismo. 
		// Entonces se definen 2 epsilon:
		// 1-un epsilon Far, que representa una distancia grande, Si la prof desde la perspectiva de la luz
		// es bastante inferior a la prof. de referencia, es porque la luz esta viendo evidentemente otro objeto.
		// aun cuando tenga el mismo ID. Es probable que sea otro poligono MUY alejado del que estoy rasterizando.
		// El punto esta en sombras, aun cuando este viendo el mismo ID.
		// 2- un epsilon pequeño, que representa el zbias standard. Dentro de ese parametro tiene prioridad el ID
		// de las caras, es decir cuando la distancias son muy parecidas, si la cara es la misma efecticamente 
		// esta iluminado. Probablemente se trate del mismo poligono, aun cuando no sea exactamente el mismo pixel
		// Si las caras son diferentes, pero la profundidad es muy similar, se interpreta como dentro del error de
		// sampling del shadow map y se considera iluminado. Esto resuelve los problemas de acne y self shadows 
		// en las juntas entre poligonos diferentes.

		float s0 = prof_0 + EPSILON_FAR < prof? 0.0f: cara_0==nro_face? 1 : (prof_0 + EPSILON_NEAR < prof)? 0.0f: 1.0f;  
		float s1 = prof_1 + EPSILON_FAR < prof? 0.0f: cara_1==nro_face? 1 : (prof_1	+ EPSILON_NEAR < prof)? 0.0f: 1.0f;  
		float s2 = prof_2 + EPSILON_FAR < prof? 0.0f: cara_2==nro_face? 1 : (prof_2 + EPSILON_NEAR < prof)? 0.0f: 1.0f;  
		float s3 = prof_3 + EPSILON_FAR < prof? 0.0f: cara_3==nro_face? 1 : (prof_3 + EPSILON_NEAR < prof)? 0.0f: 1.0f;  

		I = lerp( lerp( s0, s1, vecino.x ),lerp( s2, s3, vecino.x ),vecino.y);		
        
        // Light it
        Diffuse = ( saturate( dot( -vLight, normalize( vNormal) ) ) * I * ( 1 - g_vLightAmbient ) + g_vLightAmbient )
                  * g_vMaterial;
		       
    } 
    else
    {
        Diffuse = g_vLightAmbient * g_vMaterial;
    }

	
	return g_bLuminance?float4(I,I,I,1):color_base * Diffuse;
}


//-----------------------------------------------------------------------------
// Vertex Shader: VertLight
// Desc: Process vertex for the light object
//-----------------------------------------------------------------------------
void VertLight( float4 iPos : POSITION,
                float3 iNormal : NORMAL,
                float2 iTex : TEXCOORD0,
                out float4 oPos : POSITION,
                out float2 Tex : TEXCOORD0 )
{
    oPos = mul( iPos, g_mWorldView );		// Transform position to view space
    oPos = mul( oPos, g_mProj );			// Transform to screen coord
    Tex = iTex;								// Propagate texture coord
}


//-----------------------------------------------------------------------------
// Pixel Shader: PixLight
// Desc: Process pixel for the light object
//-----------------------------------------------------------------------------
float4 PixLight( float2 Tex : TEXCOORD0,
                 float4 vPos : TEXCOORD1 ) : COLOR
{
    return tex2D( g_samScene, Tex );
}



//-----------------------------------------------------------------------------
// Vertex Shader: VertShadow
// Desc: Process vertex for the shadow map
//-----------------------------------------------------------------------------
void VertShadow( float4 Pos : POSITION,
                 float3 Normal : NORMAL,
				 float2 iTex : TEXCOORD0,
                 out float4 oPos : POSITION,
                 out float2 Depth : TEXCOORD0,
                 out float idFace: TEXCOORD1,
				 out float2 Tex : TEXCOORD2)
{
    // Compute the projected coordinates
    oPos = mul( Pos, g_mWorldView );
    oPos = mul( oPos, g_mProj );
    
    // propago las coord. de textura
    Tex = iTex;

    // Store z and w in our spare texcoord
    Depth.xy = oPos.zw;
    
    // Propagate id face (resuelve en parte el self shadow artifact)
	idFace = nro_face;
}




//-----------------------------------------------------------------------------
// Pixel Shader: PixShadow
// Desc: Process pixel for the shadow map
//-----------------------------------------------------------------------------
void PixShadow( float2 Depth : TEXCOORD0,float idFace: TEXCOORD1,
				float2 Tex : TEXCOORD2,
                out float4 Color : COLOR )
{
	float4 color_base = tex2D( g_samScene, Tex );
	if(color_base.r==1 && color_base.g==0 && color_base.b==1)
		discard;
    // Depth is z / w
    // Standard shadow map 
    // Color = Depth.x / Depth.y;
    
	// el formato del shadow map deberia ser D3DFMT_G32R32F
	// En el canal R devuelvo el id del objeto (o nro de face, u otro id relativo al face)
    Color.r = idFace;
	// en el canal G devuelvo la prof. pp dicha
    Color.g = Depth.x / Depth.y;
    
    // el canal b y el a no se usan
	Color.ba = 1;		// no se usa
}

void PixShadowView( float2 Depth : TEXCOORD0,out float4 Color : COLOR )
{
	// parche para ver el shadow map
	float k = Depth.x/Depth.y;
	Color = (1-k)*50;
}

void PixIDView( float idFace: TEXCOORD1,out float4 Color : COLOR )
{
	Color.a = 1;
	int nro_obj = round(idFace/100);
	int i = round(fmod(nro_obj,18));
	float3 c[] = { {1,0,0} , {0,1,0} , {0,0,1} , 
				   {0,1,1} , {1,1,0} , {0,1,1} , 
				   {1,0,1} , {1,1,1} , {0,0,0} ,
				   {0.5,0,0} , {0,0.5,0} , {0,0,0.5} , 
				   {0,0.5,0.5} , {0.5,0.5,0} , {0,0.5,0.5} , 
				   {0.5,0,0.5} , {0.5,0.5,0.5} , {0.3,0.5,0.3}
				   };
				   
	Color.rgb = c[i];
	
	//Color.r = 1 - idFace/100.0/(float)cant_obj;		// nro de objeto
	//Color.g = 0;		//fmod(idFace,100.0);		// nro de layer
	//Color.b = 0;
}


//-----------------------------------------------------------------------------
// Technique: RenderScene
// Desc: Renders scene objects
//-----------------------------------------------------------------------------
// shadow map combined ID
technique RenderScene
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertScene();
        PixelShader = compile ps_3_0 PixScene();
    }
}

// shadow map standard
technique RenderSceneStd
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertScene();
        PixelShader = compile ps_3_0 PixSceneStd();
    }
}

// shadow map solo ids
technique RenderSceneSoloIDs
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertScene();
        PixelShader = compile ps_3_0 PixSceneSoloIDs();
    }
}



//-----------------------------------------------------------------------------
// Technique: RenderLight
// Desc: Renders the light object
//-----------------------------------------------------------------------------
technique RenderLight
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertLight();
        PixelShader = compile ps_3_0 PixLight();
    }
}




//-----------------------------------------------------------------------------
// Technique: RenderShadow
// Desc: Renders the shadow map
//-----------------------------------------------------------------------------
technique RenderShadow
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertShadow();
        PixelShader = compile ps_3_0 PixShadow();
    }
}


//-----------------------------------------------------------------------------
// solo para ver el shadow map a efectos didacticos
//-----------------------------------------------------------------------------
technique RenderShadowView
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertShadow();
        PixelShader = compile ps_3_0 PixShadowView();
    }
}
//-----------------------------------------------------------------------------
// solo para ver el id  map a efectos didacticos
//-----------------------------------------------------------------------------
technique RenderIDView
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertShadow();
        PixelShader = compile ps_3_0 PixIDView();
    }
}


//-----------------------------------------------------------------------------
// solo para ver el mapa de coordenadas xyz a efectos didacticos
//-----------------------------------------------------------------------------

void VertSceneXYZ( float4 iPos : POSITION,
					float2 iTex : TEXCOORD0,
					out float4 oPos : POSITION,                
					out float2 Tex : TEXCOORD0,
					out float4 wPos : TEXCOORD1,
					out float4 sPos : TEXCOORD2,
					out float4 vPosLight : TEXCOORD3 
                )
{
    // transformo al screen space
    oPos = mul( iPos, g_mWorldView );
    // aprovecho para clacular y propagar la posicion del vertice en el espacio de proyeccion de la luz
    vPosLight = mul( oPos, g_mViewToLightProj );
    // sigo con el screen space pos
    oPos = mul( oPos, g_mProj);
	sPos = oPos;
	
	// propago coordenadas de textura 
    Tex = iTex;
    
    // propago la posicion del vertice en World space
    wPos = mul( iPos, g_mWorld);
    
    // propago la posicion del vertice en el espacio de proyeccion de la luz
    //vPosLight = mul( wPos, g_mViewLightProj );
	
}



float4 PixSceneXYZ(	float2 Tex : TEXCOORD0,
					float4 wPos : TEXCOORD1,
					float4 sPos : TEXCOORD2,
					float4 vPosLight : TEXCOORD3
					):COLOR
{
	// Mapa con las posiciones de los vertices en WorldSpace
	return float4(wPos.xyz,vPosLight.z/vPosLight.w);	
}	

technique RenderXYZMap
{
    pass p0
    {
        VertexShader = compile vs_3_0 VertSceneXYZ();
        PixelShader = compile ps_3_0 PixSceneXYZ();
    }
}
