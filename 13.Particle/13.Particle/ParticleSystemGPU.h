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
	float deltaTime_;
	unsigned int maxParticles_;
	float time_;
	unsigned int spawnMode_;
};

class ParticleSystemGPU final
{
public:
	float gTimeAcc_;
	int gGpuPatternMode_;

	ParticleSystemGPU();
	~ParticleSystemGPU();

	bool Init(ID3D11Device* pDevice, unsigned int maxParticleCnt, ID3D11ShaderResourceView* pTextureSRV);

	void Tick(ID3D11DeviceContext* pDeviceContext, float deltaTime);

	void Render(ID3D11DeviceContext* pDeviceContext, const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraRight, const DirectX::XMFLOAT3& cameraUp);

private:
	bool InitParticleBuffer(ID3D11Device* pDevice, unsigned int maxParticleCnt);

	bool InitShaders(ID3D11Device* pDevice);

	bool InitConstantBuffer(ID3D11Device* pDevice);

	bool InitBlendState(ID3D11Device* pDevice);

	bool InitSamplerState(ID3D11Device* pDevice);

	void CleanUp();

	unsigned int maxParticleCnt_;

	ID3D11Buffer* pParticleBuffer_;
	ID3D11ShaderResourceView* pParticleSRV_;
	ID3D11UnorderedAccessView* pParticleUAV_;

	ID3D11VertexShader* pVertexShader_;
	ID3D11GeometryShader* pGeometryShader_;
	ID3D11PixelShader* pPixelShader_;
	ID3D11ComputeShader* pComputeShader_;

	ID3D11Buffer* pConstantBuffer_;
	ID3D11Buffer* pComputeConstantBuffer_;

	ID3D11BlendState* pBlendState_;
	ID3D11SamplerState* pSamplerState_;

	ID3D11ShaderResourceView* pParticleTextureSRV_;
};