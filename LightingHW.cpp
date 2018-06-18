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
CChecker      *g_pChecker;
CSolidCube    *g_pCube;
CSolidSphere  *g_pSphere;
CObjReader	  *g_pStarFruit;

CQuad         *g_LeftWall, *g_RightWall;
CQuad         *g_FrontWall, *g_BackWall;
CQuad         *g_TopWall;

// For View Point
GLfloat g_fRadius = 5.0;
GLfloat g_fTheta = 60.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;
GLfloat g_fCameraMoveX = 0.f;				// for camera movment
GLfloat g_fCameraMoveY = 6.0f;				// for camera movment
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
point4 g_vLight( 4.0f, 4.0f, 0.0f, 1.0f); // x = r cos(theta) = 3, z = r sin(theta) = 0
color4 g_fLightI( g_fLightR, g_fLightG, g_fLightB, 1.0f); 
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// 函式的原型宣告
extern void IdleProcess();

void init( void )
{
	mat4 mxT;
	vec4 vT, vColor;
	// 產生所需之 Model View 與 Projection Matrix

	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);

	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	// 產生物件的實體
	g_pChecker = new CChecker(GRID_SIZE);
// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pChecker->SetMaterials(vec4(0), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pChecker->	SetKaKdKsShini(0, 0.8f, 0.5f, 1);
#endif
	g_pChecker->SetShadingMode(GOURAUD_SHADING);
	g_pChecker->SetShader();

	//-------------WALLS--------------
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

	vT.x = 0.0f; vT.y = 20.0f; vT.z = 0.0f;
	mxT = Translate(vT);
	g_TopWall = new CQuad;
	g_TopWall->SetMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_TopWall->SetShadingMode(GOURAUD_SHADING);
	g_TopWall->SetShader();
	g_TopWall->SetColor(vec4(0.6f));
	g_TopWall->SetTRSMatrix(mxT*RotateX(180.0f)*Scale(20.0f, 1, 20.0f));
	g_TopWall->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	//-----------------------------------------
	g_pCube = new CSolidCube;
// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pCube->SetMaterials(vec4(0), vec4(0.85f, 0, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pCube->SetKaKdKsShini(0.15f, 0.8f, 0.2f, 2);
#endif
	g_pCube->SetShader();
	// 設定 Cube
	vT.x = 1.5; vT.y = 0.5; vT.z = -1.5;
	mxT = Translate(vT);
	g_pCube->SetTRSMatrix(mxT);
	g_pCube->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pSphere = new CSolidSphere(1, 16, 16);
// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pSphere->SetMaterials(vec4(0), vec4(0, 0, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pSphere->SetKaKdKsShini(0.15f, 0.53f, 0.78f, 4);
#endif
	g_pSphere->SetShader();
	// 設定 Sphere
	vT.x = -1.5; vT.y = 1.0; vT.z = 1.5;
	mxT = Translate(vT);
	g_pSphere->SetTRSMatrix(mxT);
	g_pSphere->SetShadingMode(GOURAUD_SHADING);

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
	vT.x = 3.0; vT.y = 0.5; vT.z = 3.0;
	mxT = Translate(vT);
	g_pStarFruit->SetTRSMatrix(mxT);
	//g_pStarFruit->SetShadingMode(GOURAUD_SHADING);

	//------------------------------------------
	// 設定 代表 Light 的 WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->SetShader();
	mxT = Translate(g_vLight);
	g_pLight->SetTRSMatrix(mxT);
	g_pLight->SetColor(g_fLightI);

#ifdef LIGHTING_WITHGPU
	g_pLight->SetLightingDisable();
#endif

	// 因為本範例不會動到 Projection Matrix 所以在這裡設定一次即可
	// 就不寫在 OnFrameMove 中每次都 Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pChecker->SetProjectionMatrix(mpx);
	g_pCube->SetProjectionMatrix(mpx);
	g_pSphere->SetProjectionMatrix(mpx);
	g_pStarFruit->SetProjectionMatrix(mpx);

	g_pLight->SetProjectionMatrix(mpx);
	g_LeftWall->SetProjectionMatrix(mpx);
	g_RightWall->SetProjectionMatrix(mpx);
	g_FrontWall->SetProjectionMatrix(mpx);
	g_BackWall->SetProjectionMatrix(mpx);
	g_TopWall->SetProjectionMatrix(mpx);
}

//----------------------------------------------------------------------------
void GL_Display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window

	g_pChecker->Draw();
	g_pSphere->Draw();
	g_pCube->Draw();
	g_pStarFruit->Draw();

	g_pLight->Draw();

	g_LeftWall->Draw();
	g_RightWall->Draw();
	g_FrontWall->Draw();
	g_BackWall->Draw();
	g_TopWall->Draw();

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
		g_pChecker->SetViewMatrix(mvx);
		g_pCube->SetViewMatrix(mvx);
		g_pSphere->SetViewMatrix(mvx);
		g_pStarFruit->SetViewMatrix(mvx);

		g_pLight->SetViewMatrix(mvx);

		g_LeftWall->SetViewMatrix(mvx);
		g_RightWall->SetViewMatrix(mvx);
		g_FrontWall->SetViewMatrix(mvx);
		g_BackWall->SetViewMatrix(mvx);
		g_TopWall->SetViewMatrix(mvx);
	}

	if( g_bAutoRotating ) { // Part 2 : 重新計算 Light 的位置
		UpdateLightPosition(delta);
	}
	// 如果需要重新計算時，在這邊計算每一個物件的顏色
	g_pChecker->Update(delta, g_vLight, g_fLightI);
	g_pCube->Update(delta, g_vLight, g_fLightI);
	g_pSphere->Update(delta, g_vLight, g_fLightI);
	g_pStarFruit->Update(delta, g_vLight, g_fLightI);

	g_pLight->Update(delta);

	g_LeftWall->Update(delta, g_vLight, g_fLightI);
	g_RightWall->Update(delta, g_vLight, g_fLightI);
	g_FrontWall->Update(delta, g_vLight, g_fLightI);
	g_BackWall->Update(delta, g_vLight, g_fLightI);
	g_TopWall->Update(delta, g_vLight, g_fLightI);

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
		g_pLight->SetColor(g_fLightI);
		break;
	case 114: // r key
		if( g_fLightR >= 0.05f ) g_fLightR -= 0.05f;
		g_fLightI.x = g_fLightR;
		g_pLight->SetColor(g_fLightI);
		break;
	case 71: // G key
		if( g_fLightG <= 0.95f ) g_fLightG += 0.05f;
		g_fLightI.y = g_fLightG;
		g_pLight->SetColor(g_fLightI);
		break;
	case 103: // g key
		if( g_fLightG >= 0.05f ) g_fLightG -= 0.05f;
		g_fLightI.y = g_fLightG;
		g_pLight->SetColor(g_fLightI);
		break;
	case 66: // B key
		if( g_fLightB <= 0.95f ) g_fLightB += 0.05f;
		g_fLightI.z = g_fLightB;
		g_pLight->SetColor(g_fLightI);
		break;
	case 98: // b key
		if( g_fLightB >= 0.05f ) g_fLightB -= 0.05f;
		g_fLightI.z = g_fLightB;
		g_pLight->SetColor(g_fLightI);
		break;
//---------------------------------------------------
    case 033:
		glutIdleFunc( NULL );
		delete g_pCube;
		delete g_pSphere;
		delete g_pChecker;
		delete g_pStarFruit;
		delete g_pLight;
		CCamera::getInstance()->destroyInstance();
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:   // 目前按下的是滑鼠左鍵
			//if ( state == GLUT_DOWN ) ; 
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
		case GLUT_KEY_LEFT:		// 目前按下的是向左方向鍵

			break;
		case GLUT_KEY_RIGHT:	// 目前按下的是向右方向鍵

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