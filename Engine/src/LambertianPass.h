#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include "Camera.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "Stencil.h"
#include "ShadowCamCbuf.h"
namespace Rgph
{
	class LambertianPass : public RenderQueuePass
	{
	public:
		LambertianPass( Graphics& gfx,std::string name )
			:
			RenderQueuePass( std::move( name ) )
		{
			AddBind(m_pShadowCbuf);
			RegisterSink( DirectBufferSink<RenderTarget>::Make( "renderTarget",renderTarget ) );
			RegisterSink( DirectBufferSink<DepthStencil>::Make( "depthStencil",depthStencil ) );
			RegisterSink( DirectBindableSink<Bindable>::Make("shadowMap", m_pShadowMap));
			RegisterSource( DirectBufferSource<RenderTarget>::Make( "renderTarget",renderTarget ) );
			RegisterSource( DirectBufferSource<DepthStencil>::Make( "depthStencil",depthStencil ) );
			AddBind( Stencil::Resolve( gfx,Stencil::Mod::Off ) );
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			m_pCamera = &cam;
		}
		void BindShadowCamera(const Camera& cam) noexcept
		{
			m_pShadowCbuf->SetCamera(cam);
		}
		void Execute( Graphics& gfx ) const noexcept(!IS_DEBUG) override
		{
			assert(m_pCamera);
			m_pCamera->BindtoGFX(gfx);
			m_pShadowCbuf->Update(gfx);
			RenderQueuePass::Execute(gfx);
		}
	private:
		const Camera* m_pCamera = nullptr;
		std::shared_ptr<Bindable> m_pShadowMap;
		std::shared_ptr<ShadowCamCbuf> m_pShadowCbuf;
	};
}