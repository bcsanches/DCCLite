#include "GL_Renderer.h"

#include <wx/glcanvas.h>

namespace LitePanel::Gui
{
	GL_Renderer::GL_Renderer()
	{
		//empty
	}

	void GL_Renderer::DrawLine(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2, LitePanel::Render::Color_t color, float thickness)
	{
		glColor4ub(LP_COL_GETR(color), LP_COL_GETG(color), LP_COL_GETB(color), LP_COL_GETA(color));
		glLineWidth(thickness);

		glBegin(GL_LINES);

		glVertex2f(p1.m_tX, p1.m_tY);
		glVertex2f(p2.m_tX, p2.m_tY);

		glEnd();
	}

	void GL_Renderer::DrawRect(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t size, LitePanel::Render::Color_t color)
	{
		glColor4ub(LP_COL_GETR(color), LP_COL_GETG(color), LP_COL_GETB(color), LP_COL_GETA(color));

		const auto x2 = p1.m_tX + size.m_tX;
		const auto y2 = p1.m_tY + size.m_tY;

		glBegin(GL_LINE_LOOP);

		glVertex2f(p1.m_tX, p1.m_tY);
		glVertex2f(x2, p1.m_tY);
		glVertex2f(x2, y2);
		glVertex2f(p1.m_tX, y2);

		glEnd();
	}

	void GL_Renderer::DrawFilledRect(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t size, LitePanel::Render::Color_t color)
	{
		glColor4ub(LP_COL_GETR(color), LP_COL_GETG(color), LP_COL_GETB(color), LP_COL_GETA(color));

		glBegin(GL_QUADS);

		glVertex2f(p1.m_tX, p1.m_tY);
		glVertex2f(p1.m_tX + size.m_tX, p1.m_tY);
		glVertex2f(p1.m_tX + size.m_tX, p1.m_tY + size.m_tY);
		glVertex2f(p1.m_tX, p1.m_tY + size.m_tY);

		glEnd();
	}

	void GL_Renderer::DrawText(float fontSize, LitePanel::FloatPoint_t pos, LitePanel::Render::Color_t color, const char *textBegin, const char *textEnd)
	{

	}

	void GL_Renderer::PushClipRect(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2)
	{

	}

	void GL_Renderer::PopClipRect()
	{

	}
}
