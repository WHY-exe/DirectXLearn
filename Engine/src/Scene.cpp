#include "Scene.h"
#include "Topology.h"
#include "TransformCbuf.h"
#include "IndexBuffer.h"
#include "Vertex.h"
#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "Texture.h"
#include "Sampler.h"
#include "Surface.h"
#include "ConstantBuffer.h"
#include <stdexcept>
#include "imgui.h"
#include "StrTransf.h"
#include "VS_PS_TFCB.h"
#include "Blender.h"
#include "Rasterizer.h"
#include "ConstantBufferEx.h"
#include "MathTool.h"
#include "MeshData.h"
#include <filesystem>
#ifndef NDEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif // !_Debug

Scene::Mesh::Mesh(Graphics& gfx, MeshData& mesh_data, const std::string& mesh_name) noexcept
{
	DirectX::XMStoreFloat4x4(&m_transform, DirectX::XMMatrixIdentity());
	AddEssentialBind(
		VertexBuffer::Resolve(
			gfx,
			ANSI_TO_UTF8_STR(mesh_name),
			*mesh_data.GetVertecies().get())
	);

	AddEssentialBind(
		IndexBuffer::Resolve(
			gfx,
			ANSI_TO_UTF8_STR(mesh_name),
			mesh_data.GetIndicies()
		)
	);
	AddEssentialBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	AddEssentialBind(std::make_shared<TransformCbuf>(gfx, *this));
	{
		Technique shade("shade");
		{
			Step normalDraw(0);

			auto texs = mesh_data.GetTextures();
			if (texs.m_amTex.get())
			{
				normalDraw.AddBind(texs.m_amTex);
			}
			if (texs.m_difTex.get())
			{
				normalDraw.AddBind(texs.m_difTex);
			}
			if (texs.m_specTex.get())
			{
				normalDraw.AddBind(texs.m_specTex);
			}
			if (texs.m_normTex.get())
			{
				normalDraw.AddBind(texs.m_normTex);
			}
			normalDraw.AddBind(
				Rasterizer::Resolve(
					gfx,
					mesh_data.GetTextures().m_hasAlpha)
			);
			auto pvs = VertexShader::Resolve(gfx, mesh_data.GetVSPath());
			auto pvsbc = static_cast<VertexShader&>(*pvs).GetByteCode();
			normalDraw.AddBind(std::move(pvs));
			normalDraw.AddBind(InputLayout::Resolve(gfx, mesh_data.GetVertecies()->GetLayout(), pvsbc));
			normalDraw.AddBind(PixelShader::Resolve(gfx, mesh_data.GetPSPath()));

			normalDraw.AddBind(Sampler::Resolve(gfx));
			normalDraw.AddBind(
				std::make_shared<CachingPixelConstantBuffer>(
					gfx,
					mesh_data.GetConstData(),
					2u)
			);
			shade.AddStep(normalDraw);
		}
		AddTechnique(shade);
	}
}
void Scene::Mesh::Submit(FrameCommander& fc, DirectX::FXMMATRIX accumulateTransform) const noexcept(!IS_DEBUG)
{
	DirectX::XMStoreFloat4x4(&m_transform, accumulateTransform);
	Drawable::Submit(fc);
}

DirectX::XMMATRIX Scene::Mesh::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat4x4(&m_transform);
}

int Scene::Node::GetId() const noexcept
{
	return m_id;
}

const DirectX::XMFLOAT4X4& Scene::Node::GetAppliedTransform() const noexcept
{
	return m_AppliedTransform;
}


void Scene::Node::Submit(FrameCommander& fc, DirectX::FXMMATRIX accumulateTransform) const noexcept
{
	const auto build =
		DirectX::XMLoadFloat4x4(&m_AppliedTransform) *
		DirectX::XMLoadFloat4x4(&m_BaseTransform) * 
		accumulateTransform;
	for (const auto pm : m_pMeshes)
	{
		pm->Submit(fc, build);
	}
	for (const auto& pc : m_pChilds)
	{
		pc->Submit(fc, build);
	}

}

Scene::Node::Node(int id, const std::wstring& NodeName, std::vector<Mesh*> pMeshes, const DirectX::XMMATRIX& transform)
	:
	m_id(id),
	m_pMeshes(std::move(pMeshes)),
	m_szNodeName(UTF8_TO_ANSI_STR(NodeName))
{
	DirectX::XMStoreFloat4x4(&m_BaseTransform, transform);
	DirectX::XMStoreFloat4x4(&m_AppliedTransform, DirectX::XMMatrixIdentity());
}

void Scene::Node::AddChild(std::unique_ptr<Node> child) noexcept(!IS_DEBUG)
{
	assert(child);
	m_pChilds.emplace_back(std::move(child));
}

void Scene::Node::ShowTree(Node*& pSelectedNode) const noexcept(!IS_DEBUG)
{
	const int selected_id = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();
	const auto imgui_flags = ImGuiTreeNodeFlags_OpenOnArrow 
		| ((m_id == selected_id) ? ImGuiTreeNodeFlags_Selected : 0)
		| ((m_pChilds.empty() ? ImGuiTreeNodeFlags_Leaf : 0));
	// render the node
	const auto expand = ImGui::TreeNodeEx((void*)(intptr_t)m_id, imgui_flags, m_szNodeName.c_str());
	if (ImGui::IsItemClicked() || ImGui::IsItemActivated())
	{
		pSelectedNode = const_cast<Node*>(this);
	}
	// render the child node if root expanded
	if (expand)
	{
		for (auto& child : m_pChilds)
		{
			child->ShowTree(pSelectedNode);
		}
		ImGui::TreePop();
	}
}

void Scene::Node::SetAppliedTransform(DirectX::XMMATRIX transform)
{
	DirectX::XMStoreFloat4x4(&m_AppliedTransform, transform);
}

void Scene::Model::ModelWindow::Show(const char* WindowName, const Node& node) noexcept(!IS_DEBUG)
{
	if (ImGui::Begin(WindowName))
	{
		ImGui::Columns(2, nullptr, true);
		node.ShowTree(m_pSelectedNode);

		ImGui::NextColumn();
		auto selected_node_id = m_pSelectedNode->GetId();
		if (m_pSelectedNode)
		{
			
			ImGui::End();
		}
	}
}

void Scene::Model::ModelWindow::AppliedParameters() noexcept
{
	if (auto& trans_d = m_WindowData[m_pSelectedNode->GetId()].transformDirty)
	{
		m_pSelectedNode->SetAppliedTransform(GetTransform());
		trans_d = false;
	}
}

DirectX::XMMATRIX Scene::Model::ModelWindow::GetTransform() noexcept
{
	const auto& transform = m_WindowData[m_pSelectedNode->GetId()].transform_data;
	return
		DirectX::XMMatrixRotationRollPitchYaw(transform.roll, transform.yaw, transform.pitch) *
		DirectX::XMMatrixScaling(transform.scale, transform.scale, transform.scale) *
		DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);
}

std::optional<DCBuf::Buffer>& Scene::Model::ModelWindow::GetMaterialConstant() noexcept
{
	return m_WindowData[m_pSelectedNode->GetId()].constant_data;
}

Scene::Node* Scene::Model::ModelWindow::GetSelectedNode() const noexcept
{
	return m_pSelectedNode;
}

Scene::Model::Model(Graphics& gfx,ModelSetting& option)
	:
	m_szModelName(option.szModelName),
	m_pWindow(std::make_unique<ModelWindow>())
{
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(
		option.szModelPath,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace
	);
	if (pScene == nullptr)
	{
		throw std::logic_error(importer.GetErrorString());
	}

	for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
	{
		m_pMeshes.emplace_back(ParseMesh(gfx, *pScene->mMeshes[i], option, pScene->mMaterials));
	}
	int next_id = 0;
	m_pRoot = ParseNode(next_id, *pScene->mRootNode);
	m_pWindow->m_pSelectedNode = const_cast<Node*>(m_pRoot.get());
}

std::unique_ptr<Scene::Mesh> Scene::Model::ParseMesh(Graphics& gfx, const aiMesh& mesh, ModelSetting& option, const aiMaterial* const* pMaterial)
{
	MeshData mesh_data = MeshData(gfx, option.szModelPath, mesh, pMaterial);
	return std::make_unique<Mesh>(gfx, mesh_data, option.szModelName + std::string(mesh.mName.C_Str()));
}

std::unique_ptr<Scene::Node> Scene::Model::ParseNode(int& next_id, const aiNode& node)
{
	const auto transform = DirectX::XMMatrixTranspose(
		DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation))
	);
	std::vector<Mesh*> curMeshPtrs;
	curMeshPtrs.reserve(node.mNumMeshes);
	for (UINT i = 0; i < node.mNumMeshes; i++)
	{
		UINT meshIdx = node.mMeshes[i];
		curMeshPtrs.emplace_back(m_pMeshes.at(meshIdx).get());
	}
	std::unique_ptr<Node> pNode = std::make_unique<Node>(
		next_id,
		ANSI_TO_UTF8_STR(std::string(node.mName.C_Str())), 
		std::move(curMeshPtrs), transform);
	for (UINT i = 0 ; i < node.mNumChildren; i++)
	{
		pNode->AddChild(ParseNode(++next_id, *node.mChildren[i]));
	}
	return pNode;
}

void Scene::Model::SpwanControlWindow() noexcept
{
	m_pWindow->Show(m_szModelName.c_str(), *m_pRoot);
}

void Scene::Model::Submit(FrameCommander& fc) const noexcept(!IS_DEBUG)
{
	m_pRoot->Submit(fc, DirectX::XMMatrixIdentity());
}

void Scene::Model::SetPos(float x, float y, float z) noexcept
{
	auto& transform = m_pWindow->m_WindowData[m_pWindow->m_pSelectedNode->m_id].transform_data;
	m_pWindow->m_WindowData[m_pWindow->m_pSelectedNode->m_id].transformDirty = true;
	transform.x = x;
	transform.y = y;
	transform.z = z;
}

void Scene::Model::Scale(float scale) noexcept
{
	auto& transform = m_pWindow->m_WindowData[m_pWindow->m_pSelectedNode->m_id].transform_data;
	m_pWindow->m_WindowData[m_pWindow->m_pSelectedNode->m_id].transformDirty = true;
	transform.scale = scale;
}

void Scene::Model::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
	auto& transform = m_pWindow->m_WindowData[m_pWindow->m_pSelectedNode->m_id].transform_data;
	m_pWindow->m_WindowData[m_pWindow->m_pSelectedNode->m_id].transformDirty = true;
	transform.x = pos.x;
	transform.y = pos.y;
	transform.z = pos.z;
}

