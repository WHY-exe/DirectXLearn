#include "VSBase.hlsli"

VSOut main(float3 pos : Position3D, float3 fn : Normal, float2 tc : Tex2D)
{
	VSOut vso;
	vso.tc = tc;	
	vso.ViewPos = (float3)mul(float4(pos, 1.0f), modelCamView);
    vso.viewNorm = mul(fn, (float3x3) modelCamView);
    vso.tan = float3(0.0f, 0.0f, 0.0f);
	vso.pos = mul(float4(pos, 1.0f), modelProjView);
	
	return vso;
}