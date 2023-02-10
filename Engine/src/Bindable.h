#pragma once
#include "Graphics.h"
#include "StrTransf.h"
#include "BindableCodex.h"
#include "TechProbe.h"
#include <string>
class Bindable
{
public:
	virtual void Bind(Graphics& gfx) noexcept = 0;
	virtual ~Bindable() = default;
	virtual void InitializeParentReference(const class Drawable&) noexcept
	{}
	virtual void Accept(TechProbe& probe) noexcept {};
	virtual std::wstring GetUID() const noexcept
	{
		return L"";
	}
protected:
	static ID3D11Device* GetDevice(Graphics& gfx) noexcept;
	static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
	static DxgiInfoManager& GetInfoMan(Graphics& gfx);
};

