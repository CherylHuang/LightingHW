#ifndef COBJREADER_H
#define COBJREADER_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

//#define QUAD_NUM 6		// 2 faces, 2 triangles/face 

class CObjReader : public CShape
{
private:
	vec4 *_vecPoints; //����I

public:
	CObjReader(char *);
	~CObjReader();

	void Update(float dt, point4 vLightPos, color4 vLightI);
	void Update(float dt, const LightSource &Lights);
	void Update(float dt); // ���p��������ө�

	void RenderWithFlatShading(point4 vLightPos, color4 vLightI);
	void RenderWithGouraudShading(point4 vLightPos, color4 vLightI);
	void RenderWithFlatShading(const LightSource &Lights);
	void RenderWithGouraudShading(const LightSource &Lights);

	void Draw();
	void DrawW();
};

#endif