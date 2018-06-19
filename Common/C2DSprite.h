#ifndef C2DSPRITE_H
#define C2DSPRITE_H

#include "../header/Angel.h"
#include "C2DShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define QUAD_NUM 6		// 2 faces, 2 triangles/face 

class C2DSprite : public C2DShape
{
public:
	C2DSprite();
	~C2DSprite();

	vec4  m_OriginalLL, m_OriginalTR;  // sprite 在 local 座標的左下與右上兩個頂點座標
	vec4  m_LL, m_TR;  // sprite 在世界座標的左上與右下兩個頂點座標
	vec4  m_DefaultColor;
	bool  m_bPushed;

	void SetTRSMatrix(mat4 &mat);
	void SetDefaultColor(vec4 vColor);
	bool getButtonStatus(){ return m_bPushed; }
	bool OnTouches(const vec2 &tp);
	void Update(float dt);

	GLuint GetShaderHandle() { return m_uiProgram; }

	void Draw();
	void DrawW();
};




#endif