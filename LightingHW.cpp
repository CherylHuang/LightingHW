// Gouraud shading with a single light source at (4, 4, 0);
// Part 1 : Per vertex lighting is computed by GPU
// Part 2 : The control of the light source (position and intensity (RGB) )
//			Light's position could be moved automatically with Key A/a 
//			Light's intensity could be changed with Key R/r G/g B/b
// Part 3 : Geometry objects' materials
//
// 執行前的準備工作
// 關閉 CShape.h 中的 #define LIGHTING_WITHCPU 
// 開啟 CShape.h 中的 #define LIGHTING_WITHGPU 
// 開啟 CShape.h 中的 #define PERVERTEX_LIGHTING 
//

#include "header/Angel.h"
#include "Common/CQuad.h"
#include "Common/CSolidCube.h"
#include "Common/CSolidSphere.h"
#include "Common/CWireSphere.h"
#include "Common/CWireCube.h"
#include "Common/CChecker.h"
#include "Common/CCamera.h"
#include "Common/CObjReader.h"
#include "Common/C2DSprite.h"

#define SPACE_KEY 32
#define SCREEN_SIZE 800
#define HALF_SIZE SCREEN_SIZE /2 
#define VP_HALFWIDTH  20.0f
#define VP_HALFHEIGHT 20.0f
#define GRID_SIZE 20 // must be an even number

#define SETTING_MATERIALS 

// For Model View and Projection Matrix
mat4 g_mxModelView(1.0f);
mat4 g_mxProjection;

// For Objects
CObjReader	  *g_pGemSweet;
CObjReader	  *g_pGemToy;
CObjReader	  *g_pGemGarden;
CObjReader	  *g_pStarFruit;

CQuad         *g_BottomWall, *g_TopWall;
CQuad         *g_LeftWall, *g_RightWall;
CQuad         *g_FrontWall, *g_BackWall;

// For View Point
GLfloat g_fRadius = 6.0;
GLfloat g_fTheta = 60.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;
GLfloat g_fCameraMoveX = 0.f;				// for camera movment
GLfloat g_fCameraMoveY = 7.0f;				// for camera movment
GLfloat g_fCameraMoveZ = 0.f;				// for camera movment
mat4	g_matMoveDir;		// 鏡頭移動方向
point4  g_MoveDir;
point4  g_at;				// 鏡頭觀看方向
point4  g_eye;				// 鏡頭位置

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 4;
float g_fLightTheta = 0;

float g_fLightR = 0.95f;
float g_fLightG = 0.95f;
float g_fLightB = 0.95f;

CWireSphere *g_pLight;
point4 g_vLight( 4.0f, 10.0f, 0.0f, 1.0f); // x = r cos(theta) = 3, z = r sin(theta) = 0
color4 g_fLightI( g_fLightR, g_fLightG, g_fLightB, 1.0f); 

LightSource g_Light_Point = {
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // ambient 
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // diffuse
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // specular
	g_vLight,   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),		  // spotTarget
	vec3(0.0f, 0.0f, 0.0f),		  // spotDirection，需重新計算
	0.0f,	// spotExponent(parameter e); cos^(e)(phi) , 當 exponent 為 0 時，光源的照明是平均分布
	180.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	-1.0f,	// spotCosCutoff = cos(spotCutoff) ; spot 的照明範圍取 cos
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0	// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

//----------------------------------------------------
// 2D 介面所需要的相關變數
// g_p2DBtn 為放在四個角落的代表按鈕的四邊形
// g_2DView 與  g_2DProj  則是這裡使用 opengl 預設的鏡頭 (0,0,0) 看向 -Z 方向
// 使用平行投影
C2DSprite *g_p2DBtn[4];
mat4  g_2DView;
mat4  g_2DProj;

//------SPOT LIGHT------//
color4 g_fLightI_crystalOff(0.0f, 0.0f, 0.0f, 1.0f);	//關燈
color4 g_fLightI_crystalRED(0.95f, 0.2f, 0.2f, 1.0f);	//spot light 顏色
color4 g_fLightI_crystalPURPLE(0.95f, 0.2f, 0.95f, 1.0f);
color4 g_fLightI_crystalBLUE(0.2f, 0.2f, 0.95f, 1.0f);

bool g_bLightOn_red = true;
CWireSphere *g_pLight_red;		//紅水晶
point4 g_vLight_red(6.0f, 5.0f, -6.0f, 1.0f);		// above red crystal
LightSource g_Light_Red = {
	color4(0, 0, 0, 0), // ambient 
	g_fLightI_crystalRED, // diffuse
	g_fLightI_crystalRED, // specular
	point4(10.0f, 3.0f, -10.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(10.0f, 0.0f, -10.0f),		  // spotTarget
	vec3(0.0f, 0.0f, 0.0f),		  // spotDirection，需重新計算
	1.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	10.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.98f,	// spotCosCutoff = cos(spotCutoff) ; spot 的照明範圍取 cos
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0	// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

bool g_bLightOn_purple = true;
CWireSphere *g_pLight_purple;	//紫水晶
point4 g_vLight_purple(6.0f, 5.0f, 6.0f, 1.0f);	// above purple crystal
LightSource g_Light_Purple = {
	color4(0, 0, 0, 0), // ambient 
	g_fLightI_crystalPURPLE, // diffuse
	g_fLightI_crystalPURPLE, // specular
	point4(10.0f, 3.0f, 10.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(10.0f, 0.0f, 10.0f),		  // spotTarget
	vec3(0.0f, 0.0f, 0.0f),		  // spotDirection，需重新計算
	1.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	10.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.98f,	// spotCosCutoff = cos(spotCutoff) ; spot 的照明範圍取 cos
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0	// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

bool g_bLightOn_blue = true;
CWireSphere *g_pLight_blue;		//藍水晶
point4 g_vLight_blue(-6.0f, 7.0f, 0.0f, 1.0f);		// above blue crystal
LightSource g_Light_Blue = {
	color4(0, 0, 0, 0), // ambient 
	g_fLightI_crystalBLUE, // diffuse
	g_fLightI_crystalBLUE, // specular
	point4(-10.0f, 5.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(-10.0f, 0.0f, 0.0f),		  // spotTarget
	vec3(0.0f, 0.0f, 0.0f),		  // spotDirection，需重新計算
	1.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	10.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.98f,	// spotCosCutoff = cos(spotCutoff) ; spot 的照明範圍取 cos
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0	// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

//----------------------------------------------------------------------------
// 函式的原型宣告
extern void IdleProcess();

void init( void )
{
	mat4 mxT, mxS;
	vec4 vT;
	vec3 vS;
	// 產生所需之 Model View 與 Projection Matrix

	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);

	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	// 產生物件的實體

	//-------------WALLS--------------
	vT.x = 0.0f; vT.y = 0.0f; vT.z = 0.0f;
	mxT = Translate(vT);
	g_BottomWall = new CQuad;
	g_BottomWall->SetMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0.0f, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_BottomWall->SetShadingMode(GOURAUD_SHADING);
	g_BottomWall->SetShader();
	g_BottomWall->SetColor(vec4(0.6f));
	g_BottomWall->SetTRSMatrix(mxT * Scale(20.0f, 1, 20.0f));
	g_BottomWall->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	vT.x = 0.0f; vT.y = 20.0f; vT.z = 0.0f;
	mxT = Translate(vT);
	g_TopWall = new CQuad;
	g_TopWall->SetMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_TopWall->SetShadingMode(GOURAUD_SHADING);
	g_TopWall->SetShader();
	g_TopWall->SetColor(vec4(0.6f));
	g_TopWall->SetTRSMatrix(mxT * RotateX(180.0f) * Scale(20.0f, 1, 20.0f));
	g_TopWall->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	vT.x = -10.0f; vT.y = 10.0f; vT.z = 0;
	mxT = Translate(vT);
	g_LeftWall = new CQuad;
	g_LeftWall->SetMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_LeftWall->SetShadingMode(GOURAUD_SHADING);
	g_LeftWall->SetShader();
	g_LeftWall->SetColor(vec4(0.6f));
	g_LeftWall->SetTRSMatrix(mxT*RotateZ(-90.0f)*Scale(20.0f, 1, 20.0f));
	g_LeftWall->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	vT.x = 10.0f; vT.y = 10.0f; vT.z = 0;
	mxT = Translate(vT);
	g_RightWall = new CQuad;
	g_RightWall->SetMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_RightWall->SetShadingMode(GOURAUD_SHADING);
	g_RightWall->SetShader();
	g_RightWall->SetColor(vec4(0.6f));
	g_RightWall->SetTRSMatrix(mxT*RotateZ(90.0f)*Scale(20.0f, 1, 20.0f));
	g_RightWall->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	vT.x = 0.0f; vT.y = 10.0f; vT.z = 10.0f;
	mxT = Translate(vT);
	g_FrontWall = new CQuad;
	g_FrontWall->SetMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_FrontWall->SetShadingMode(GOURAUD_SHADING);
	g_FrontWall->SetShader();
	g_FrontWall->SetColor(vec4(0.6f));
	g_FrontWall->SetTRSMatrix(mxT*RotateX(-90.0f)*Scale(20.0f, 1, 20.0f));
	g_FrontWall->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	vT.x = 0.0f; vT.y = 10.0f; vT.z = -10.0f;
	mxT = Translate(vT);
	g_BackWall = new CQuad;
	g_BackWall->SetMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_BackWall->SetShadingMode(GOURAUD_SHADING);
	g_BackWall->SetShader();
	g_BackWall->SetColor(vec4(0.6f));
	g_BackWall->SetTRSMatrix(mxT*RotateX(90.0f)*Scale(20.0f, 1, 20.0f));
	g_BackWall->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	//-----------------------------------------
	g_pGemSweet = new CObjReader("obj/gem_sweet.obj");			//紅水晶
// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pGemSweet->SetMaterials(vec4(0), vec4(0.85f, 0, 0, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemSweet->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
#endif
	g_pGemSweet->SetShader();
	// 設定 Cube
	vT.x = 6.0f; vT.y = 1.0f; vT.z = -6.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemSweet->SetTRSMatrix(mxT * mxS);
	g_pGemSweet->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pGemGarden = new CObjReader("obj/gem_garden.obj");		//紫水晶
	// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pGemGarden->SetMaterials(vec4(0), vec4(0.85f, 0.0f, 0.85f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemGarden->SetKaKdKsShini(0.15f, 0.95f, 0.8f, 5);
#endif
	g_pGemGarden->SetShader();
	// 設定 Cube
	vT.x = 6.0f; vT.y = 1.0f; vT.z = 6.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemGarden->SetTRSMatrix(mxT * mxS);
	g_pGemGarden->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pGemToy = new CObjReader("obj/gem_toy.obj");				//藍水晶
// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pGemToy->SetMaterials(vec4(0), vec4(0, 0, 0.85f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemToy->SetKaKdKsShini(0.15f, 0.95f, 0.95f, 5);
#endif
	g_pGemToy->SetShader();
	// 設定 Sphere
	vT.x = -6.0f; vT.y = 1.0f; vT.z = 0.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemToy->SetTRSMatrix(mxT * mxS);
	g_pGemToy->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	//g_pStarFruit
	g_pStarFruit = new CObjReader("obj/starfruit.obj");		//楊桃
	// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pStarFruit->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pStarFruit->SetKaKdKsShini(0.15f, 0.8f, 0.2f, 2);
#endif
	g_pStarFruit->SetShader();
	// 設定 StarFruit
	vT.x = 0.0f; vT.y = 1.0f; vT.z = 0.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 2.5f;				//Scale
	mxS = Scale(vS);
	g_pStarFruit->SetTRSMatrix(mxT * mxS);
	//g_pStarFruit->SetShadingMode(GOURAUD_SHADING);

	//------------------------------------------
	// 設定 代表 Light 的 WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->SetShader();
	mxT = Translate(g_vLight);
	g_pLight->SetTRSMatrix(mxT);
	g_pLight->SetColor(g_fLightI);

	// 設定 代表 Light_red 的 WireSphere
	g_pLight_red = new CWireSphere(0.2f, 6, 3);
	g_pLight_red->SetShader();
	mxT = Translate(g_vLight_red);
	g_pLight_red->SetTRSMatrix(mxT);
	g_pLight_red->SetColor(g_fLightI_crystalRED);

	// 設定 代表 Light_purple 的 WireSphere
	g_pLight_purple = new CWireSphere(0.2f, 6, 3);
	g_pLight_purple->SetShader();
	mxT = Translate(g_vLight_purple);
	g_pLight_purple->SetTRSMatrix(mxT);
	g_pLight_purple->SetColor(g_fLightI_crystalPURPLE);

	// 設定 代表 Light_blue 的 WireSphere
	g_pLight_blue = new CWireSphere(0.2f, 6, 3);
	g_pLight_blue->SetShader();
	mxT = Translate(g_vLight_blue);
	g_pLight_blue->SetTRSMatrix(mxT);
	g_pLight_blue->SetColor(g_fLightI_crystalBLUE);

	//------------------------------------------

#ifdef LIGHTING_WITHGPU
	g_pLight->SetLightingDisable();
	g_pLight_red->SetLightingDisable();
	g_pLight_purple->SetLightingDisable();
	g_pLight_blue->SetLightingDisable();
#endif
	//-------------------------------------------------------------------
	// 計算 SpotDirection Vector 同時正規化成單位向量
	g_Light_Point.UpdateDirection();
	g_Light_Red.UpdateDirection();
	g_Light_Purple.UpdateDirection();
	g_Light_Blue.UpdateDirection();

	//-------------------------------------------------------------------

	// 以下為利用平行投影產生 2D 的介面
	// 範圍在 X/Y 平面的  -1 到 1 之間，介面都放在 Z = 0 

	g_p2DBtn[0] = new C2DSprite; g_p2DBtn[0]->SetShader();
	g_p2DBtn[0]->SetDefaultColor(g_fLightI);
	mxS = Scale(0.1f, 0.1f, 1.0f);
	mxT = Translate(0.90f, 0.90f, 0);
	g_p2DBtn[0]->SetTRSMatrix(mxT*mxS);
	g_p2DBtn[0]->SetViewMatrix(g_2DView);
	g_p2DBtn[0]->SetViewMatrix(g_2DProj);

	g_p2DBtn[1] = new C2DSprite; g_p2DBtn[1]->SetShader();
	g_p2DBtn[1]->SetDefaultColor(g_fLightI_crystalRED);
	mxT = Translate(0.75f, 0.90f, 0);
	g_p2DBtn[1]->SetTRSMatrix(mxT*mxS);
	g_p2DBtn[1]->SetViewMatrix(g_2DView);
	g_p2DBtn[1]->SetViewMatrix(g_2DProj);

	g_p2DBtn[2] = new C2DSprite; g_p2DBtn[2]->SetShader();
	g_p2DBtn[2]->SetDefaultColor(g_fLightI_crystalPURPLE);
	mxT = Translate(0.6f, 0.90f, 0);
	g_p2DBtn[2]->SetTRSMatrix(mxT*mxS);
	g_p2DBtn[2]->SetViewMatrix(g_2DView);
	g_p2DBtn[2]->SetViewMatrix(g_2DProj);

	g_p2DBtn[3] = new C2DSprite; g_p2DBtn[3]->SetShader();
	g_p2DBtn[3]->SetDefaultColor(g_fLightI_crystalBLUE);
	mxT = Translate(0.45f, 0.90f, 0);
	g_p2DBtn[3]->SetTRSMatrix(mxT*mxS);
	g_p2DBtn[3]->SetViewMatrix(g_2DView);
	g_p2DBtn[3]->SetViewMatrix(g_2DProj);

	//-------------------------------------------------------------------
	// 不會動到 Projection Matrix ,設定一次即可, 不用在 OnFrameMove 中每次都 Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pGemSweet->SetProjectionMatrix(mpx);
	g_pGemToy->SetProjectionMatrix(mpx);
	g_pGemGarden->SetProjectionMatrix(mpx);
	g_pStarFruit->SetProjectionMatrix(mpx);

	g_pLight->SetProjectionMatrix(mpx);
	g_pLight_red->SetProjectionMatrix(mpx);
	g_pLight_purple->SetProjectionMatrix(mpx);
	g_pLight_blue->SetProjectionMatrix(mpx);

	g_LeftWall->SetProjectionMatrix(mpx);
	g_RightWall->SetProjectionMatrix(mpx);
	g_FrontWall->SetProjectionMatrix(mpx);
	g_BackWall->SetProjectionMatrix(mpx);
	g_TopWall->SetProjectionMatrix(mpx);
	g_BottomWall->SetProjectionMatrix(mpx);
}

//----------------------------------------------------------------------------
void GL_Display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window
	glEnable(GL_BLEND); //開透明度
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //計算透明度

	g_pLight->Draw();
	g_pLight_red->Draw();
	g_pLight_purple->Draw();
	g_pLight_blue->Draw();

	g_LeftWall->Draw();
	g_RightWall->Draw();
	g_FrontWall->Draw();
	g_BackWall->Draw();
	g_TopWall->Draw();
	g_BottomWall->Draw();

	g_pStarFruit->Draw();

	glDepthMask(GL_FALSE);
	g_pGemToy->Draw();
	g_pGemSweet->Draw();
	g_pGemGarden->Draw();

	glDisable(GL_BLEND);// 關閉 Blending
	glDepthMask(GL_TRUE);// 開啟對 Z-Buffer 的寫入操作

	for (int i = 0; i < 4; i++) g_p2DBtn[i]->Draw();

	glutSwapBuffers();	// 交換 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void UpdateLightPosition(float dt)
{
	mat4 mxT;
	// 每秒繞 Y 軸轉 90 度
	g_fElapsedTime += dt;
	g_fLightTheta = g_fElapsedTime*(float)M_PI_2;
	if( g_fLightTheta >= (float)M_PI*2.0f ) {
		g_fLightTheta -= (float)M_PI*2.0f;
		g_fElapsedTime -= 4.0f;
	}
	g_vLight.x = g_fLightRadius * cosf(g_fLightTheta);
	g_vLight.z = g_fLightRadius * sinf(g_fLightTheta);

	mxT = Translate(g_vLight);
	g_pLight->SetTRSMatrix(mxT);

	//------Update point light------
		g_Light_Point.position.x = g_vLight.x;
		g_Light_Point.position.z = g_vLight.z;
		g_Light_Point.spotDirection.x = g_Light_Point.spotTarget.x - g_Light_Point.position.x;
		g_Light_Point.spotDirection.y = g_Light_Point.spotTarget.y - g_Light_Point.position.y;
		g_Light_Point.spotDirection.z = g_Light_Point.spotTarget.z - g_Light_Point.position.z;
		g_Light_Point.spotDirection = normalize(g_Light_Point.spotDirection);

}
//----------------------------------------------------------------------------

void onFrameMove(float delta)
{
	// for camera
	g_at = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi) + g_fCameraMoveX,
		g_fRadius*cos(g_fTheta) + g_fCameraMoveY,
		g_fRadius*sin(g_fTheta)*cos(g_fPhi) + g_fCameraMoveZ,
		1.0f);
	g_eye = vec4(g_fCameraMoveX, g_fCameraMoveY, g_fCameraMoveZ, 1.0f);	//第一人稱視角
	auto camera = CCamera::getInstance();
	camera->updateViewLookAt(g_eye, g_at);

	//------------------------------------------------------------
	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view 與 projection matrix 是否需要更新給物件
	//auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pGemSweet->SetViewMatrix(mvx);
		g_pGemToy->SetViewMatrix(mvx);
		g_pGemGarden->SetViewMatrix(mvx);
		g_pStarFruit->SetViewMatrix(mvx);

		g_pLight->SetViewMatrix(mvx);
		g_pLight_red->SetViewMatrix(mvx);
		g_pLight_purple->SetViewMatrix(mvx);
		g_pLight_blue->SetViewMatrix(mvx);

		g_LeftWall->SetViewMatrix(mvx);
		g_RightWall->SetViewMatrix(mvx);
		g_FrontWall->SetViewMatrix(mvx);
		g_BackWall->SetViewMatrix(mvx);
		g_TopWall->SetViewMatrix(mvx);
		g_BottomWall->SetViewMatrix(mvx);
	}

	if( g_bAutoRotating ) { // Part 2 : 重新計算 Light 的位置
		UpdateLightPosition(delta);
	}

	//-------------------------------------------------------------
	if (g_bLightOn_red) {	// 紅水晶 spot light 開關
		g_Light_Red.diffuse = g_fLightI_crystalRED;
		g_Light_Red.specular = g_fLightI_crystalRED;
		g_pLight_red->SetColor(g_fLightI_crystalRED);	//wire sphere
	}
	else {
		g_Light_Red.diffuse = g_fLightI_crystalOff;
		g_Light_Red.specular = g_fLightI_crystalOff;
		g_pLight_red->SetColor(g_fLightI_crystalOff);	//wire sphere
	}
	if (g_bLightOn_purple) {	// 紫水晶 spot light 開關
		g_Light_Purple.diffuse = g_fLightI_crystalPURPLE;
		g_Light_Purple.specular = g_fLightI_crystalPURPLE;
		g_pLight_purple->SetColor(g_fLightI_crystalPURPLE);	//wire sphere
	}
	else {
		g_Light_Purple.diffuse = g_fLightI_crystalOff;
		g_Light_Purple.specular = g_fLightI_crystalOff;
		g_pLight_purple->SetColor(g_fLightI_crystalOff);	//wire sphere
	}
	if (g_bLightOn_blue) {	// 藍水晶 spot light 開關
		g_Light_Blue.diffuse = g_fLightI_crystalBLUE;
		g_Light_Blue.specular = g_fLightI_crystalBLUE;
		g_pLight_blue->SetColor(g_fLightI_crystalBLUE);	//wire sphere
	}
	else {
		g_Light_Blue.diffuse = g_fLightI_crystalOff;
		g_Light_Blue.specular = g_fLightI_crystalOff;
		g_pLight_blue->SetColor(g_fLightI_crystalOff);	//wire sphere
	}

	//-------------------------------------------------------------
	// 如果需要重新計算時，在這邊計算每一個物件的顏色
	g_pLight->Update(delta);
	g_pLight_red->Update(delta);
	g_pLight_purple->Update(delta);
	g_pLight_blue->Update(delta);

	g_LeftWall->Update(delta, g_Light_Point);		// walls
	g_RightWall->Update(delta, g_Light_Point);
	g_FrontWall->Update(delta, g_Light_Point);
	g_BackWall->Update(delta, g_Light_Point);
	g_TopWall->Update(delta, g_Light_Point);
	g_BottomWall->Update(delta, g_Light_Point, g_Light_Red, g_Light_Purple, g_Light_Blue);

	g_pGemSweet->Update(delta, g_Light_Point, g_Light_Red, g_Light_Purple, g_Light_Blue);		// gems
	g_pGemToy->Update(delta, g_Light_Point, g_Light_Red, g_Light_Purple, g_Light_Blue);
	g_pGemGarden->Update(delta, g_Light_Point, g_Light_Red, g_Light_Purple, g_Light_Blue);
	g_pStarFruit->Update(delta, g_Light_Point, g_Light_Red, g_Light_Purple, g_Light_Blue);		// center

	GL_Display();
}

//----------------------------------------------------------------------------

void Win_Keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	// Part 2 : for single light source
	case  SPACE_KEY:
		g_bAutoRotating = !g_bAutoRotating;
		break;
	case  'o':
		g_bLightOn_red = !g_bLightOn_red;		// spot light above red crystal 開關燈
		break;
	case  'i':
		g_bLightOn_purple = !g_bLightOn_purple;	// spot light above purple crystal 開關燈
		break;
	case  'u':
		g_bLightOn_blue = !g_bLightOn_blue;		// spot light above blue crystal 開關燈
		break;

	// ---------- for camera movment -----------
	case 'W':
	case 'w':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
			else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
			if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
			else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
		}
		break;
	case 'S':
	case 's':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
			else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
			if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
			else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
		}
		break;
	case 'A':
	case 'a':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
			else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
			if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
			else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
		}
		break;
	case 'D':
	case 'd':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
			else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
			if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
			else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
		}
		break;

	// --------- for light color ---------
	case 82: // R key
		if( g_fLightR <= 0.95f ) g_fLightR += 0.05f;
		g_fLightI.x = g_fLightR;
		g_Light_Point.diffuse.x = g_fLightI.x;		//更新光源diffuse color
		g_Light_Point.specular.x = g_fLightI.x;		//更新光源specular color
		g_pLight->SetColor(g_fLightI);
		break;
	case 114: // r key
		if( g_fLightR >= 0.05f ) g_fLightR -= 0.05f;
		g_fLightI.x = g_fLightR;
		g_Light_Point.diffuse.x = g_fLightI.x;
		g_Light_Point.specular.x = g_fLightI.x;
		g_pLight->SetColor(g_fLightI);
		break;
	case 71: // G key
		if( g_fLightG <= 0.95f ) g_fLightG += 0.05f;
		g_fLightI.y = g_fLightG;
		g_Light_Point.diffuse.y = g_fLightI.y;
		g_Light_Point.specular.y = g_fLightI.y;
		g_pLight->SetColor(g_fLightI);
		break;
	case 103: // g key
		if( g_fLightG >= 0.05f ) g_fLightG -= 0.05f;
		g_fLightI.y = g_fLightG;
		g_Light_Point.diffuse.y = g_fLightI.y;
		g_Light_Point.specular.y = g_fLightI.y;
		g_pLight->SetColor(g_fLightI);
		break;
	case 66: // B key
		if( g_fLightB <= 0.95f ) g_fLightB += 0.05f;
		g_fLightI.z = g_fLightB;
		g_Light_Point.diffuse.z = g_fLightI.z;
		g_Light_Point.specular.z = g_fLightI.z;
		g_pLight->SetColor(g_fLightI);
		break;
	case 98: // b key
		if( g_fLightB >= 0.05f ) g_fLightB -= 0.05f;
		g_fLightI.z = g_fLightB;
		g_Light_Point.diffuse.z = g_fLightI.z;
		g_Light_Point.specular.z = g_fLightI.z;
		g_pLight->SetColor(g_fLightI);
		break;
//---------------------------------------------------
    case 033:
		glutIdleFunc( NULL );
		delete g_FrontWall;
		delete g_BackWall;
		delete g_LeftWall;
		delete g_RightWall;
		delete g_TopWall;
		delete g_BottomWall;

		delete g_pGemSweet;
		delete g_pGemToy;
		delete g_pGemGarden;
		delete g_pStarFruit;

		delete g_pLight;
		delete g_pLight_red;
		delete g_pLight_purple;
		delete g_pLight_blue;
		for (int i = 0; i < 4; i++) delete g_p2DBtn[i];
		CCamera::getInstance()->destroyInstance();
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------

inline void ScreenToUICoordinate(int x, int y, vec2 &pt)
{
	pt.x = 2.0f*(float)x / SCREEN_SIZE - 1.0f;
	pt.y = 2.0f*(float)(SCREEN_SIZE - y) / SCREEN_SIZE - 1.0f;
}

//------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	vec2 pt;
	switch(button) {
		case GLUT_LEFT_BUTTON:   // 目前按下的是滑鼠左鍵
			if (state == GLUT_DOWN) {
				ScreenToUICoordinate(x, y, pt);
				if (g_p2DBtn[0]->OnTouches(pt)) {
					//if (g_p2DBtn[0]->getButtonStatus()) g_bShowZAxis = false;
					//else g_bShowZAxis = true;
					g_bAutoRotating = !g_bAutoRotating;
				}
				if (g_p2DBtn[1]->OnTouches(pt)) {
					g_bLightOn_red = !g_bLightOn_red;
				}
				if (g_p2DBtn[2]->OnTouches(pt)) {
					g_bLightOn_purple = !g_bLightOn_purple;
				}
				if (g_p2DBtn[3]->OnTouches(pt)) {
					g_bLightOn_blue = !g_bLightOn_blue;
				}
			}
			break;
		case GLUT_MIDDLE_BUTTON:  // 目前按下的是滑鼠中鍵 ，換成 Y 軸
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_RIGHT_BUTTON:   // 目前按下的是滑鼠右鍵
			//if ( state == GLUT_DOWN ) ;
			break;
		default:
			break;
	} 
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_UP:		// 目前按下的是向上方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
				g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
				else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
				if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
				else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
			}
			break;
		case GLUT_KEY_DOWN:		// 目前按下的是向下方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
				g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
				else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
				if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
				else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
			}
			break;
		case GLUT_KEY_LEFT:		// 目前按下的是向左方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
				g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
				else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
				if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
				else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
			}
			break;
		case GLUT_KEY_RIGHT:	// 目前按下的是向右方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//限制空間
				g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > 8.5) g_fCameraMoveX = 8.5f;
				else if (g_fCameraMoveX < -8.5) g_fCameraMoveX = -8.5f;
				if (g_fCameraMoveZ > 8.5) g_fCameraMoveZ = 8.5f;
				else if (g_fCameraMoveZ < -8.5) g_fCameraMoveZ = -8.5f;
			}
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {
	g_fPhi = (float)M_PI*(x - HALF_SIZE)/(HALF_SIZE); // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)-M_PI*(float)y/SCREEN_SIZE;
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)M_PI*(x - HALF_SIZE) / (HALF_SIZE); // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)-M_PI*(float)y / SCREEN_SIZE;
}
//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND); //開透明度
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //計算透明度
}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    
	glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( SCREEN_SIZE, SCREEN_SIZE );

	// If you use freeglut the two lines of code can be added to your application 
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

    glutCreateWindow("Shading Example 3");

	// The glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(), 
	// which ensures that all extensions with valid entry points will be exposed.
	glewExperimental = GL_TRUE; 
    glewInit();  

    init();

	glutMouseFunc(Win_Mouse);
	glutMotionFunc(Win_MouseMotion);
	glutPassiveMotionFunc(Win_PassiveMotion);  
    glutKeyboardFunc( Win_Keyboard );	// 處理 ASCI 按鍵如 A、a、ESC 鍵...等等
	glutSpecialFunc( Win_SpecialKeyboard);	// 處理 NON-ASCI 按鍵如 F1、Home、方向鍵...等等
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc( IdleProcess );
	
    glutMainLoop();
    return 0;
}