#include "hud.h"
#include "triangleapi.h"

void CTextureRef::Draw2DQuad(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a) const noexcept
{
	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gEngfuncs.pTriAPI->Color4ub(r, g, b, a);
	Bind();
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->TexCoord2f(s1, t1);
	gEngfuncs.pTriAPI->Vertex3f(x1, y1, 0);
	gEngfuncs.pTriAPI->TexCoord2f(s1, t2);
	gEngfuncs.pTriAPI->Vertex3f(x1, y2, 0);
	gEngfuncs.pTriAPI->TexCoord2f(s2, t2);
	gEngfuncs.pTriAPI->Vertex3f(x2, y2, 0);
	gEngfuncs.pTriAPI->TexCoord2f(s2, t1);
	gEngfuncs.pTriAPI->Vertex3f(x2, y1, 0);
	gEngfuncs.pTriAPI->End();
}

void CTextureRef::Draw2DQuadScaled(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a) const noexcept
{
	const float scale = gHUD.m_flScale;
	Draw2DQuad(x1 * scale, y1 * scale, x2 * scale, y2 * scale, s1, t1, s2, t2, r, g, b, a);
}
