// Gouraud shading with a single light source at (4, 4, 0);
// Part 1 : Per vertex lighting is computed by GPU
// Part 2 : The control of the light source (position and intensity (RGB) )
//			Light's position could be moved automatically with Key A/a 
//			Light's intensity could be changed with Key R/r G/g B/b
// Part 3 : Geometry objects' materials
//
// ����e���ǳƤu�@
// ���� CShape.h ���� #define LIGHTING_WITHCPU 
// �}�� CShape.h ���� #define LIGHTING_WITHGPU 
// �}�� CShape.h ���� #define PERVERTEX_LIGHTING 
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
CObjReader	  *g_pGemSweet;
CObjReader	  *g_pGemToy;
CObjReader	  *g_pGemGarden;
CObjReader	  *g_pStarFruit;

CQuad         *g_LeftWall, *g_RightWall;
CQuad         *g_FrontWall, *g_BackWall;
CQuad         *g_TopWall;

// For View Point
GLfloat g_fRadius = 6.0;
GLfloat g_fTheta = 60.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;
GLfloat g_fCameraMoveX = 0.f;				// for camera movment
GLfloat g_fCameraMoveY = 7.0f;				// for camera movment
GLfloat g_fCameraMoveZ = 0.f;				// for camera movment
mat4	g_matMoveDir;		// ���Y���ʤ�V
point4  g_MoveDir;
point4  g_at;				// ���Y�[�ݤ�V
point4  g_eye;				// ���Y��m

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
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// �禡���쫬�ŧi
extern void IdleProcess();

void init( void )
{
	mat4 mxT, mxS;
	vec4 vT, vColor;
	vec3 vS;
	// ���ͩһݤ� Model View �P Projection Matrix

	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);

	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	// ���ͪ��󪺹���
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
	g_pGemSweet = new CObjReader("obj/gem_sweet.obj");		//������
// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pGemSweet->SetMaterials(vec4(0), vec4(0.85f, 0, 0, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemSweet->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
#endif
	g_pGemSweet->SetShader();
	// �]�w Cube
	vT.x = 6.0f; vT.y = 1.0f; vT.z = -6.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemSweet->SetTRSMatrix(mxT * mxS);
	g_pGemSweet->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pGemGarden = new CObjReader("obj/gem_garden.obj");		//������
	// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pGemGarden->SetMaterials(vec4(0), vec4(0.85f, 0.0f, 0.85f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemGarden->SetKaKdKsShini(0.15f, 0.95f, 0.8f, 5);
#endif
	g_pGemGarden->SetShader();
	// �]�w Cube
	vT.x = 6.0f; vT.y = 1.0f; vT.z = 6.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemGarden->SetTRSMatrix(mxT * mxS);
	g_pGemGarden->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pGemToy = new CObjReader("obj/gem_toy.obj");				//�Ť���
// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pGemToy->SetMaterials(vec4(0), vec4(0, 0, 0.85f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemToy->SetKaKdKsShini(0.15f, 0.95f, 0.95f, 5);
#endif
	g_pGemToy->SetShader();
	// �]�w Sphere
	vT.x = -6.0f; vT.y = 1.0f; vT.z = 0.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemToy->SetTRSMatrix(mxT * mxS);
	g_pGemToy->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	//g_pStarFruit
	g_pStarFruit = new CObjReader("obj/starfruit.obj");		//����
	// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pStarFruit->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pStarFruit->SetKaKdKsShini(0.15f, 0.8f, 0.2f, 2);
#endif
	g_pStarFruit->SetShader();
	// �]�w StarFruit
	vT.x = 0.0f; vT.y = 1.0f; vT.z = 0.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 2.5f;				//Scale
	mxS = Scale(vS);
	g_pStarFruit->SetTRSMatrix(mxT * mxS);
	//g_pStarFruit->SetShadingMode(GOURAUD_SHADING);

	//------------------------------------------
	// �]�w �N�� Light �� WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->SetShader();
	mxT = Translate(g_vLight);
	g_pLight->SetTRSMatrix(mxT);
	g_pLight->SetColor(g_fLightI);

#ifdef LIGHTING_WITHGPU
	g_pLight->SetLightingDisable();
#endif

	// �]�����d�Ҥ��|�ʨ� Projection Matrix �ҥH�b�o�̳]�w�@���Y�i
	// �N���g�b OnFrameMove ���C���� Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pChecker->SetProjectionMatrix(mpx);
	g_pGemSweet->SetProjectionMatrix(mpx);
	g_pGemToy->SetProjectionMatrix(mpx);
	g_pGemGarden->SetProjectionMatrix(mpx);
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
	glEnable(GL_BLEND); //�}�z����
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //�p��z����

	g_pLight->Draw();
	g_pChecker->Draw();
	g_LeftWall->Draw();
	g_RightWall->Draw();
	g_FrontWall->Draw();
	g_BackWall->Draw();
	g_TopWall->Draw();

	g_pStarFruit->Draw();

	glDepthMask(GL_FALSE);
	g_pGemToy->Draw();
	g_pGemSweet->Draw();
	g_pGemGarden->Draw();

	glDisable(GL_BLEND);// ���� Blending
	glDepthMask(GL_TRUE);// �}�ҹ� Z-Buffer ���g�J�ާ@

	glutSwapBuffers();	// �洫 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void UpdateLightPosition(float dt)
{
	mat4 mxT;
	// �C��¶ Y �b�� 90 ��
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
	g_eye = vec4(g_fCameraMoveX, g_fCameraMoveY, g_fCameraMoveZ, 1.0f);	//�Ĥ@�H�ٵ���
	auto camera = CCamera::getInstance();
	camera->updateViewLookAt(g_eye, g_at);

	//------------------------------------------------------------
	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view �P projection matrix �O�_�ݭn��s������
	//auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pChecker->SetViewMatrix(mvx);
		g_pGemSweet->SetViewMatrix(mvx);
		g_pGemToy->SetViewMatrix(mvx);
		g_pGemGarden->SetViewMatrix(mvx);
		g_pStarFruit->SetViewMatrix(mvx);

		g_pLight->SetViewMatrix(mvx);

		g_LeftWall->SetViewMatrix(mvx);
		g_RightWall->SetViewMatrix(mvx);
		g_FrontWall->SetViewMatrix(mvx);
		g_BackWall->SetViewMatrix(mvx);
		g_TopWall->SetViewMatrix(mvx);
	}

	if( g_bAutoRotating ) { // Part 2 : ���s�p�� Light ����m
		UpdateLightPosition(delta);
	}
	// �p�G�ݭn���s�p��ɡA�b�o��p��C�@�Ӫ����C��
	g_pChecker->Update(delta, g_vLight, g_fLightI);
	g_pGemSweet->Update(delta, g_vLight, g_fLightI);
	g_pGemToy->Update(delta, g_vLight, g_fLightI);
	g_pGemGarden->Update(delta, g_vLight, g_fLightI);
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
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//����Ŷ�
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// �ץ��d��
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
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//����Ŷ�
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// �ץ��d��
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
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//����Ŷ�
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// �ץ��d��
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
		if (g_fCameraMoveX <= 8.5f && g_fCameraMoveX >= -8.5f && g_fCameraMoveZ <= 8.5f && g_fCameraMoveZ >= -8.5f) {	//����Ŷ�
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// �ץ��d��
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
		delete g_pGemSweet;
		delete g_pGemToy;
		delete g_pGemGarden;
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
		case GLUT_LEFT_BUTTON:   // �ثe���U���O�ƹ�����
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_MIDDLE_BUTTON:  // �ثe���U���O�ƹ����� �A���� Y �b
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_RIGHT_BUTTON:   // �ثe���U���O�ƹ��k��
			//if ( state == GLUT_DOWN ) ;
			break;
		default:
			break;
	} 
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_LEFT:		// �ثe���U���O�V����V��

			break;
		case GLUT_KEY_RIGHT:	// �ثe���U���O�V�k��V��

			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {
	g_fPhi = (float)M_PI*(x - HALF_SIZE)/(HALF_SIZE); // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)-M_PI*(float)y/SCREEN_SIZE;
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)M_PI*(x - HALF_SIZE) / (HALF_SIZE); // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)-M_PI*(float)y / SCREEN_SIZE;
}
//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND); //�}�z����
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //�p��z����
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
    glutKeyboardFunc( Win_Keyboard );	// �B�z ASCI ����p A�Ba�BESC ��...����
	glutSpecialFunc( Win_SpecialKeyboard);	// �B�z NON-ASCI ����p F1�BHome�B��V��...����
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc( IdleProcess );
	
    glutMainLoop();
    return 0;
}