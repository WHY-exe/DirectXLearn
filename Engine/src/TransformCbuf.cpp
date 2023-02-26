#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
	:
	m_parent(parent)
{
	if (!s_vcbuf)
	{
		s_vcbuf = std::make_unique<VertexConstantBuffer<Transform>>(gfx, slot);
	}
}

void TransformCbuf::SetScalin(float scale_in) noexcept
{
	m_scalein = scale_in;
}

void TransformCbuf::Bind(Graphics& gfx) noexcept
{
	UpdateBindImpl(gfx, GetTransform(gfx));
	s_vcbuf->Bind(gfx);
}
TransformCbuf::Transform TransformCbuf::GetTransform(Graphics& gfx) const noexcept
{
	const auto scale_mtx = DirectX::XMMatrixScaling(m_scalein, m_scalein, m_scalein);
	return
	{
		DirectX::XMMatrixTranspose(m_parent.GetTransformXM() * gfx.GetCamera()),
		DirectX::XMMatrixTranspose(m_parent.GetTransformXM() * gfx.GetCamera() * gfx.GetProjection()),
		scale_mtx,
		m_parent.GetCenterPoint()
	};
}
void TransformCbuf::UpdateBindImpl(Graphics& gfx, const Transform& tf) const noexcept
{
	s_vcbuf->Update(gfx, tf);
}
std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transform>> TransformCbuf::s_vcbuf;