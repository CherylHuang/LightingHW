#ifndef CSHAPE_H
#define CSHAPE_H
#include "../Header/Angel.h"
#include "TypeDefine.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define FLAT_SHADING    0
#define GOURAUD_SHADING 1

// GPU 的計算必須傳更多的參數進入 Shader

//#define LIGHTING_WITHCPU
#define LIGHTING_WITHGPU
//#define PERVERTEX_LIGHTING

// 當模型有執行 non-uniform scale 的操作時，必須透過計算反矩陣來得到正確的 Normal 方向
// 開啟以下的定義即可，目前 CPU 計算的有提供
// GPU 的部分則是設定成註解

// #define GENERAL_CASE 1 

#define SPOT_LIGHT 1


class CShape 
{
protected:
	vec4 *m_pPoints;
	vec3 *m_pNormals;
	vec4 *m_pColors;
	vec2 *m_pTex;
	int  m_iNumVtx;

	GLfloat m_fColor[4]; // Object's color
	// For shaders' name
	char *m_pVXshader, *m_pFSshader;

	// For VAO
	GLuint m_uiVao;

	// For Shader
	GLuint  m_uiModelView, m_uiProjection, m_uiColor;
	GLuint  m_uiProgram;
	GLuint  m_uiBuffer;

#ifdef LIGHTING_WITHGPU

	LightSource m_Light1, m_Light2, m_Light3, m_Light4;	//四個光源

	point4  m_vLightInView, m_vLightInView2, m_vLightInView3, m_vLightInView4;				// 光源在世界座標的位置
	GLuint  m_uiLightInView, m_uiLightInView2, m_uiLightInView3, m_uiLightInView4;			// 光源在 shader 的位置
	GLuint  m_uiAmbient, m_uiAmbient2, m_uiAmbient3, m_uiAmbient4;					 // light's ambient  與 Object's ambient  與 ka 的乘積
	GLuint  m_uiDiffuse, m_uiDiffuse2, m_uiDiffuse3, m_uiDiffuse4;					 // light's diffuse  與 Object's diffuse  與 kd 的乘積
	GLuint  m_uiSpecular, m_uiSpecular2, m_uiSpecular3, m_uiSpecular4;				 // light's specular 與 Object's specular 與 ks 的乘積
	GLuint  m_uiShininess, m_uiShininess2, m_uiShininess3, m_uiShininess4;
	GLuint  m_uiLighting, m_uiLighting2, m_uiLighting3, m_uiLighting4;

	color4 m_AmbientProduct, m_AmbientProduct2, m_AmbientProduct3, m_AmbientProduct4;
	color4 m_DiffuseProduct, m_DiffuseProduct2, m_DiffuseProduct3, m_DiffuseProduct4;
	color4 m_SpecularProduct, m_SpecularProduct2, m_SpecularProduct3, m_SpecularProduct4;

	int    m_iLighting;	// 設定是否要打燈
#endif

	// For Matrices
	mat4    m_mxView, m_mxProjection, m_mxTRS;
	mat4    m_mxMVFinal;
	mat3    m_mxMV3X3Final, m_mxITMV;	// 使用在計算 物體旋轉後的新 Normal
	mat3		m_mxITView;		// View Matrix 的 Inverse Transport 
	bool    m_bProjUpdated, m_bViewUpdated, m_bTRSUpdated;

	// For materials
	Material m_Material;

	// For Shading Mode
	// 0: Flat shading, 1: Gouraud shading, 0 for default
	// 要變更上色模式，利用 SetShadingMode 來改變
	int m_iMode;		

	void		CreateBufferObject();
	void		DrawingSetShader();
	void		DrawingWithoutSetShader();

	//Multiple lights update (要更新的光源數量)
	void UpdateMultiLight(const int LightNum);

public:
	CShape();
	virtual ~CShape();
	virtual void Draw() = 0;
	virtual void DrawW() = 0; // Drawing without setting shaders
	virtual void Update(float dt, point4 vLightPos, color4 vLightI) = 0;
	virtual	void Update(float dt, const LightSource &Lights) = 0;
	virtual void Update(float dt) = 0;

	void SetShaderName(const char vxShader[], const char fsShader[]);
	void SetShader(GLuint uiShaderHandle = MAX_UNSIGNED_INT);
	void SetColor(vec4 vColor);
	void SetViewMatrix(mat4 &mat);
	void SetProjectionMatrix(mat4 &mat);
	void SetTRSMatrix(mat4 &mat);

	// For setting materials 
	void SetMaterials(color4 ambient, color4 diffuse, color4 specular);
	void SetKaKdKsShini(float ka, float kd, float ks, float shininess); // ka kd ks shininess

	// For Lighting Calculation
	void SetShadingMode(int iMode) {m_iMode = iMode;}
	vec4 PhongReflectionModel(vec4 vPoint, vec3 vNormal, vec4 vLightPos, color4 vLightI);
	vec4 PhongReflectionModel(vec4 vPoint, vec3 vNormal, const LightSource &Lights);

#ifdef LIGHTING_WITHGPU
	void SetLightingDisable() {m_iLighting = 0;}
#endif

};

#endif