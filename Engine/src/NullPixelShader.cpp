#include "NullPixelShader.h"
#include "BindableCodex.h"
#include <typeinfo>
#include "StrTransf.h"
NullPixelShader::NullPixelShader(Graphics& gfx)
{
}

void NullPixelShader::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->PSSetShader(nullptr, nullptr, 0u);
}

std::shared_ptr<NullPixelShader> NullPixelShader::Resolve(Graphics& gfx) noexcept(!IS_DEBUG)
{
	return CodeX::Resolve<NullPixelShader>(gfx);
}

std::wstring NullPixelShader::GenUID() noexcept
{
	return ANSI_TO_UTF8_STR(typeid(NullPixelShader).name());
}

std::wstring NullPixelShader::GetUID() const noexcept
{
	return GenUID();
}
