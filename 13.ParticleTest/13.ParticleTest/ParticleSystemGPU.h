#pragma once

struct ParticleGPU
{
	DirectX::XMFLOAT3 position_;
	float age_;
	DirectX::XMFLOAT3 velocity_;
	float lifeTime_;
};

struct ComputeConstantBuffer
{
	float delta_;
	unsigned int maxParticles_;
	float time_;
	unsigned int spawnMode_;
};

class ParticleSystemGPU final
{
public:
	static float gTimeAcc_;
	static int gGpuPatternMode_;

	ParticleSystemGPU();

	~ParticleSystemGPU();

	bool Initialize(ID3D11Device* device, int maxPrticles, ID3D11ShaderResourceView* particleTex);
	void Update(ID3D11DeviceContext* ctx, float delta);
	void Draw(ID3D11DeviceContext* ctx, const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraRight, const DirectX::XMFLOAT3& cameraUp);

private:
	void CleanUp();

	int maxParticles_;

	ID3D11Buffer* particleBuffer_;
	ID3D11ShaderResourceView* particleSRV_;
	ID3D11UnorderedAccessView* particleUAV_;

	ID3D11VertexShader* vertexShader_;
	ID3D11GeometryShader* geometryShader_;
	ID3D11PixelShader* pixelShader_;
	ID3D11ComputeShader* computeShader_;

	ID3D11Buffer* constantBuffer_;
	ID3D11Buffer* computeConstantBuffer_;

	ID3D11SamplerState* sampler_;
	ID3D11BlendState* blendState_;
	ID3D11ShaderResourceView* particleTexSRV_;
};