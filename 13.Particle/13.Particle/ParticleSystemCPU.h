#pragma once

struct ParticleCPU
{
	DirectX::XMFLOAT3 position_;
	float age_;
	DirectX::XMFLOAT3 velocity_;
	float lifeTime_;
};

struct ParticleVertexCPU
{
	DirectX::XMFLOAT3 position_;
	float age_;
};

class ParticleSystemCPU final
{
public:
	ParticleSystemCPU();
	~ParticleSystemCPU();

	bool Initialize(ID3D11Device* device, int maxPrticles, ID3D11ShaderResourceView* particleTex);
	void EmitParticle();
	void Update(ID3D11DeviceContext* ctx, float delta);
	void Draw(ID3D11DeviceContext* ctx, const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraRight, const DirectX::XMFLOAT3& cameraUp);

private:
	void CleanUp();

	int maxParticles_;
	std::vector<ParticleCPU> particles_;
	unsigned int aliveCount_;

	ID3D11Buffer* vertexBuffer_;
	ID3D11VertexShader* vertexShader_;
	ID3D11GeometryShader* geometryShader_;
	ID3D11PixelShader* pixelShader_;
	ID3D11InputLayout* inputLayout_;
	ID3D11Buffer* constantBuffer_;
	ID3D11SamplerState* sampler_;
	ID3D11BlendState* blendState_;
	ID3D11ShaderResourceView* particleTexSRV_;
};