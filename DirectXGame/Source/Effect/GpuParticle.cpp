#include "GpuParticle.h"

#include <algorithm>
#include <cstring>

#include <base/DirectXCommon.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

using namespace KamataEngine;

namespace {
struct Vertex {
	float x;
	float y;
};

constexpr Vertex kVertices[] = {
    {-0.5f, -0.5f},
    {0.5f,  -0.5f},
    {0.5f,  0.5f },
    {-0.5f, -0.5f},
    {0.5f,  0.5f },
    {-0.5f, 0.5f },
};

constexpr DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
constexpr DXGI_FORMAT kDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
} // namespace

GpuParticle::~GpuParticle() {
	if (particleBuffer_ && mappedParticles_) {
		particleBuffer_->Unmap(0, nullptr);
	}
	if (constantBuffer_ && mappedConstants_) {
		constantBuffer_->Unmap(0, nullptr);
	}
}

bool GpuParticle::Initialize(uint32_t maxParticles) {
	// 固定長のリングバッファとして確保し、描画中の動的な再確保を避ける。
	maxParticles_ = (std::max)(maxParticles, 1u);
	nextSpawnIndex_ = 0;
	particles_.assign(maxParticles_, Particle{});
	return CreatePipeline() && CreateVertexBuffer() && CreateParticleBuffer() && CreateConstantBuffer();
}

void GpuParticle::Emit(const Vector3& position, const Vector3& velocity, float life, float startScale, float endScale) {
	Emit(position, velocity, life, startScale, endScale, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f});
}

void GpuParticle::Emit(const Vector3& position, const Vector3& velocity, float life, float startScale, float endScale, const Vector4& startColor, const Vector4& endColor) {
	if (particles_.empty()) {
		return;
	}

	Particle& particle = particles_[nextSpawnIndex_];
	particle.position = position;
	particle.velocity = velocity;
	particle.life = (std::max)(life, 0.01f);
	particle.age = 0.0f;
	particle.startScale = (std::max)(startScale, 0.0f);
	particle.endScale = (std::max)(endScale, 0.0f);
	particle.scale = particle.startScale;
	particle.active = 1.0f;
	particle.startColor = startColor;
	particle.endColor = endColor;
	// 上限に達した場合は最も古いスロットから再利用する。
	nextSpawnIndex_ = (nextSpawnIndex_ + 1) % maxParticles_;
}

void GpuParticle::Update(float deltaTime) {
	if (deltaTime <= 0.0f) {
		return;
	}

	for (Particle& particle : particles_) {
		if (particle.active < 0.5f) {
			continue;
		}
		particle.age += deltaTime;
		if (particle.age >= particle.life) {
			particle.active = 0.0f;
			continue;
		}
		particle.position.x += particle.velocity.x * deltaTime;
		particle.position.y += particle.velocity.y * deltaTime;
		particle.position.z += particle.velocity.z * deltaTime;
		const float t = std::clamp(particle.age / particle.life, 0.0f, 1.0f);
		particle.scale = particle.startScale + (particle.endScale - particle.startScale) * t;
	}

	// CPU側で更新した全パーティクルを、永続マップ済みバッファへ反映する。
	if (mappedParticles_) {
		std::memcpy(mappedParticles_, particles_.data(), sizeof(Particle) * particles_.size());
	}
}

void GpuParticle::Draw(const Camera& camera) const {
	if (!pipelineState_ || !rootSignature_ || !mappedConstants_) {
		return;
	}
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	if (!commandList) {
		return;
	}

	mappedConstants_->view = camera.matView;
	mappedConstants_->projection = camera.matProjection;
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineState_.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	ID3D12DescriptorHeap* heaps[] = {srvHeap_.Get()};
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, srvHeap_->GetGPUDescriptorHandleForHeapStart());
	commandList->DrawInstanced(6, maxParticles_, 0, 0);
}

void GpuParticle::Clear() {
	std::fill(particles_.begin(), particles_.end(), Particle{});
	nextSpawnIndex_ = 0;
	if (mappedParticles_ && !particles_.empty()) {
		std::memcpy(mappedParticles_, particles_.data(), sizeof(Particle) * particles_.size());
	}
}

uint32_t GpuParticle::GetActiveCount() const {
	return static_cast<uint32_t>(std::count_if(particles_.begin(), particles_.end(), [](const Particle& particle) { return particle.active >= 0.5f; }));
}

bool GpuParticle::CreatePipeline() {
	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	if (!device) {
		return false;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> errors;
	HRESULT result =
	    D3DCompileFromFile(L"./Resources/shaders/GpuParticleVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShader, &errors);
	if (FAILED(result)) {
		return false;
	}
	result = D3DCompileFromFile(L"./Resources/shaders/GpuParticlePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShader, &errors);
	if (FAILED(result)) {
		return false;
	}

	// 定数バッファとパーティクル配列を、それぞれCBVとSRVでシェーダーへ渡す。
	CD3DX12_DESCRIPTOR_RANGE srvRange;
	srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	CD3DX12_ROOT_PARAMETER parameters[2];
	parameters[0].InitAsConstantBufferView(0);
	parameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_VERTEX);
	CD3DX12_ROOT_SIGNATURE_DESC signatureDesc;
	signatureDesc.Init(_countof(parameters), parameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	result = D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);
	if (FAILED(result) || FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature_)))) {
		return false;
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
	D3D12_BLEND_DESC blend{};
	// 半透明の板ポリゴンを重ねるため、アルファブレンドを有効にする。
	blend.RenderTarget[0].BlendEnable = TRUE;
	blend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	D3D12_RASTERIZER_DESC rasterizer{};
	rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizer.CullMode = D3D12_CULL_MODE_NONE;
	rasterizer.DepthClipEnable = TRUE;
	D3D12_DEPTH_STENCIL_DESC depth{};
	// 他オブジェクトとの深度比較は行うが、パーティクル自身は深度を書き込まない。
	depth.DepthEnable = TRUE;
	depth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depth.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = rootSignature_.Get();
	desc.VS = {vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()};
	desc.PS = {pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()};
	desc.BlendState = blend;
	desc.SampleMask = UINT_MAX;
	desc.RasterizerState = rasterizer;
	desc.DepthStencilState = depth;
	desc.InputLayout = {inputLayout, _countof(inputLayout)};
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = kRenderTargetFormat;
	desc.DSVFormat = kDepthFormat;
	desc.SampleDesc.Count = 1;
	return SUCCEEDED(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState_)));
}

bool GpuParticle::CreateVertexBuffer() {
	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	const UINT size = sizeof(kVertices);
	CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	if (!device || FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer_)))) {
		return false;
	}
	void* mapped = nullptr;
	if (FAILED(vertexBuffer_->Map(0, nullptr, &mapped))) {
		return false;
	}
	std::memcpy(mapped, kVertices, size);
	vertexBuffer_->Unmap(0, nullptr);
	vertexBufferView_ = {vertexBuffer_->GetGPUVirtualAddress(), size, sizeof(Vertex)};
	return true;
}

bool GpuParticle::CreateParticleBuffer() {
	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	const UINT size = static_cast<UINT>(sizeof(Particle) * particles_.size());
	CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	if (!device || FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&particleBuffer_)))) {
		return false;
	}
	if (FAILED(particleBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParticles_)))) {
		return false;
	}
	std::memset(mappedParticles_, 0, size);

	// 構造化バッファを頂点シェーダーから参照するためのSRVを作成する。
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap_)))) {
		return false;
	}
	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv.Buffer.NumElements = maxParticles_;
	srv.Buffer.StructureByteStride = sizeof(Particle);
	srv.Format = DXGI_FORMAT_UNKNOWN;
	device->CreateShaderResourceView(particleBuffer_.Get(), &srv, srvHeap_->GetCPUDescriptorHandleForHeapStart());
	return true;
}

bool GpuParticle::CreateConstantBuffer() {
	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	const UINT size = (sizeof(Constants) + 0xffu) & ~0xffu;
	CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	if (!device || FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer_)))) {
		return false;
	}
	if (FAILED(constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedConstants_)))) {
		return false;
	}
	std::memset(mappedConstants_, 0, size);
	return true;
}
