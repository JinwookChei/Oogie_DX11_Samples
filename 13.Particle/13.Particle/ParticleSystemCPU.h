#pragma once


struct ParticleCPU
{
	DirectX::XMFLOAT3 pos_;
	float age_;
	DirectX::XMFLOAT3 vel_;
	float lifeTime_;
};

struct ParticleVertexCPU
{
	DirectX::XMFLOAT3 pos_;
	float age_;
};

class ParticleSystemCPU
{
public:
	ParticleSystemCPU();
	~ParticleSystemCPU();

	bool Init(ID3D11Device* pDevice, unsigned int maxParticleCnt, ID3D11ShaderResourceView* pTextureSRV);

	void Tick(ID3D11DeviceContext* pDeviceContext, float deltaTime);

	void Render(ID3D11DeviceContext* pDeviceContext, const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraRight, const DirectX::XMFLOAT3& cameraUp);

private:
	bool InitVertexBuffer(ID3D11Device* pDevice, unsigned int maxParticleCnt);

	bool InitShaders(ID3D11Device* pDevice);

	bool InitConstantBuffer(ID3D11Device* pDevice);

	bool InitBlendState(ID3D11Device* pDevice);

	bool InitSamplerState(ID3D11Device* pDevice);

	void EmitParticle();

	void CleanUp();

	unsigned int maxParticleCnt_;
	unsigned int aliveParticleCnt_;
	std::vector<ParticleCPU> particles_;

 
	ID3D11Buffer* pVertexBuffer_;
	ID3D11InputLayout* pInputLayout_;
	ID3D11VertexShader* pVertexShader_;
	ID3D11GeometryShader* pGeometryShader_;
	ID3D11PixelShader* pPixelShader_;
	ID3D11Buffer* pConstantBuffer_;
	ID3D11BlendState* pBlendState_;
	ID3D11SamplerState* pSamplerState_;
	ID3D11ShaderResourceView* pParticleTextureSRV_;
};



