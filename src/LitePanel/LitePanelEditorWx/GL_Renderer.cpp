#include "GL_Renderer.h"

#include <algorithm>
#include <cmath>

#include <wx/glcanvas.h>

namespace LitePanel::Gui
{
	GL_Renderer::GL_Renderer()
	{
		//empty
	}

	void GL_Renderer::DrawFilledQuad(const FloatPoint_t &p1, const FloatPoint_t &p2, const FloatPoint_t &p3, const FloatPoint_t &p4, LitePanel::Render::Color_t color)
	{
		glColor4ub(LP_COL_GETR(color), LP_COL_GETG(color), LP_COL_GETB(color), LP_COL_GETA(color));

		glBegin(GL_QUADS);
		glVertex2f(p1.m_tX, p1.m_tY);
		glVertex2f(p2.m_tX, p2.m_tY);
		glVertex2f(p3.m_tX, p3.m_tY);
		glVertex2f(p4.m_tX, p4.m_tY);
		glEnd();
	}

	void GL_Renderer::DrawLine(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2, LitePanel::Render::Color_t color, float thickness)
	{
		//line vector
		const auto delta = p2 - p1;

		//vector length
		const float length = std::sqrt(delta.m_tX * delta.m_tX + delta.m_tY * delta.m_tY);

		if (length == 0.0f)
			return;

		//make a perpendicular vector, just flip x and y and invert one of then... also normalize it and in the end, adjust it size for half line thickness
		const FloatPoint_t normal{
			-delta.m_tY / length * thickness * 0.5f,
			 delta.m_tX / length * thickness * 0.5f
		};

		DrawFilledQuad(
			p1 + normal,
			p2 + normal,
			p2 - normal,
			p1 - normal,
			color
		);
	}

	void GL_Renderer::DrawRect(LitePanel::FloatPoint_t p1, LitePanel::FloatSize_t size, LitePanel::Render::Color_t color)
	{
		constexpr float BORDER_WIDTH = 1.0f;

		const auto p2 = p1 + size;

		DrawFilledRect(
			p1,
			FloatSize_t{ size.m_tWidth, BORDER_WIDTH },
			color
		);

		DrawFilledRect(
			FloatPoint_t{ p1.m_tX, p2.m_tY - BORDER_WIDTH },
			FloatSize_t{ size.m_tWidth, BORDER_WIDTH },
			color
		);

		DrawFilledRect(
			p1,
			FloatSize_t{ BORDER_WIDTH, size.m_tHeight },
			color
		);

		DrawFilledRect(
			FloatPoint_t{ p2.m_tX - BORDER_WIDTH, p1.m_tY },
			FloatSize_t{ BORDER_WIDTH, size.m_tHeight },
			color
		);
	}

	void GL_Renderer::DrawFilledRect(LitePanel::FloatPoint_t p1, LitePanel::FloatSize_t size, LitePanel::Render::Color_t color)
	{
		glLineWidth(1.0f);
		glColor4ub(LP_COL_GETR(color), LP_COL_GETG(color), LP_COL_GETB(color), LP_COL_GETA(color));

		const auto p2 = p1 + size;

		glBegin(GL_QUADS);

		glVertex2f(p1.m_tX, p1.m_tY);
		glVertex2f(p2.m_tX, p1.m_tY);
		glVertex2f(p2.m_tX, p2.m_tY);
		glVertex2f(p1.m_tX, p2.m_tY);

		glEnd();
	}

	void GL_Renderer::DrawText(float fontSize, LitePanel::FloatPoint_t pos, LitePanel::Render::Color_t color, const char *textBegin, const char *textEnd)
	{

	}

	void GL_Renderer::PushClipRect(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2)
	{
		ClipRectState state{};

		glGetIntegerv(GL_SCISSOR_BOX, state.m_aiScissorBox);
		state.m_fScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);

		m_vecClipRectStack.push_back(state);

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		// Normalize rectangle coordinates.
		const auto minX = std::min(p1.m_tX, p2.m_tX);
		const auto minY = std::min(p1.m_tY, p2.m_tY);
		const auto maxX = std::max(p1.m_tX, p2.m_tX);
		const auto maxY = std::max(p1.m_tY, p2.m_tY);

		// Convert from top-left to OpenGL bottom-left coordinates.
		GLint left = viewport[0] + static_cast<GLint>(std::floor(minX));
		GLint right = viewport[0] + static_cast<GLint>(std::ceil(maxX));

		GLint bottom = viewport[1] + viewport[3] - static_cast<GLint>(std::ceil(maxY));
		GLint top = viewport[1] + viewport[3] - static_cast<GLint>(std::floor(minY));

		// Intersect with viewport.
		left = std::max(left, viewport[0]);
		right = std::min(right, viewport[0] + viewport[2]);

		bottom = std::max(bottom, viewport[1]);
		top = std::min(top, viewport[1] + viewport[3]);

		// Intersect with current scissor rectangle.
		if (state.m_fScissorEnabled)
		{
			const auto &current = state.m_aiScissorBox;

			left = std::max(left, current[0]);
			bottom = std::max(bottom, current[1]);

			right = std::min(right, current[0] + current[2]);
			top = std::min(top, current[1] + current[3]);
		}

		glEnable(GL_SCISSOR_TEST);

		glScissor(left, bottom, std::max(0, right - left), std::max(0, top - bottom));
	}

	void GL_Renderer::PopClipRect()
	{
		if (m_vecClipRectStack.empty())
			return;

		const auto state = m_vecClipRectStack.back();
		m_vecClipRectStack.pop_back();

		glScissor(
			state.m_aiScissorBox[0],
			state.m_aiScissorBox[1],
			state.m_aiScissorBox[2],
			state.m_aiScissorBox[3]
		);

		if (state.m_fScissorEnabled)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
	}
}
