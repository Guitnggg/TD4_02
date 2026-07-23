#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <cstdint>
#include <vector>

#include <3d/Camera.h>
#include <math/Matrix4x4.h>
#include <math/Vector3.h>
#include <math/Vector4.h>

// カメラ方向を向く板ポリゴンをインスタンシング描画するパーティクル。
// パーティクルの更新はCPU、頂点の展開と描画はGPUで行う。
class GpuParticle final {
public:
	struct Particle {
		KamataEngine::Vector3 position{};
		float scale = 0.0f;

		KamataEngine::Vector3 velocity{};
		float life = 0.0f;

		float age = 0.0f;
		float startScale = 0.0f;
		float endScale = 0.0f;
		float active = 0.0f;

		KamataEngine::Vector4 startColor{1.0f, 1.0f, 1.0f, 1.0f};
		KamataEngine::Vector4 endColor{1.0f, 1.0f, 1.0f, 0.0f};
	};

	GpuParticle() = default;
	~GpuParticle();
	GpuParticle(const GpuParticle&) = delete;
	GpuParticle& operator=(const GpuParticle&) = delete;

	bool Initialize(uint32_t maxParticles);
	void Update(float deltaTime);
	void Draw(const KamataEngine::Camera& camera) const;

	void Emit(const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, float life, float startScale, float endScale);
	void Emit(
	    const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, float life, float startScale, float endScale,
	    const KamataEngine::Vector4& startColor, const KamataEngine::Vector4& endColor);

	void Clear();
	[[nodiscard]] uint32_t GetCapacity() const { return maxParticles_; }
	[[nodiscard]] uint32_t GetActiveCount() const;

private:
	struct Constants {
		KamataEngine::Matrix4x4 view{};
		KamataEngine::Matrix4x4 projection{};
	};

	bool CreatePipeline();
	bool CreateVertexBuffer();
	bool CreateParticleBuffer();
	bool CreateConstantBuffer();

	uint32_t maxParticles_ = 0;
	uint32_t nextSpawnIndex_ = 0;
	std::vector<Particle> particles_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffer_;
	Particle* mappedParticles_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
	Constants* mappedConstants_ = nullptr;
};
