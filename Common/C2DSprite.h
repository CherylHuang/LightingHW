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

	vec4  m_OriginalLL, m_OriginalTR;  // sprite �b local �y�Ъ����U�P�k�W��ӳ��I�y��
	vec4  m_LL, m_TR;  // sprite �b�@�ɮy�Ъ����W�P�k�U��ӳ��I�y��
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