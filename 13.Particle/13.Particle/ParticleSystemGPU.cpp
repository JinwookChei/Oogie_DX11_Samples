#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>

#include "Defines.h"
#include "ParticleSystemGPU.h"

static HRESULT LoadCSO(const wchar_t* filePath, ID3DBlob** ppBlob)
{
	return D3DReadFileToBlob(filePath, ppBlob);
}

float ParticleSystemGPU::gTimeAcc_ = 0.0f;
int ParticleSystemGPU::gGpuPatternMode_ = 0;

ParticleSystemGPU::ParticleSystemGPU()
	:maxParticles_(0),
	particleBuffer_(nullptr),
	particleSRV_(nullptr),
	particleUAV_(nullptr),
	vertexShader_(nullptr),
	geometryShader_(nullptr),
	pixelShader_(nullptr),
	computeShader_(nullptr),
	constantBuffer_(nullptr),
	computeConstantBuffer_(nullptr),
	sampler_(nullptr),
	blendState_(nullptr),
	particleTexSRV_(nullptr)
{
}

ParticleSystemGPU::~ParticleSystemGPU()
{
	if (particleBuffer_)
	{
		particleBuffer_->Release();
		particleBuffer_ = nullptr;
	}
	if (particleSRV_)
	{
		particleSRV_->Release();
		particleSRV_ = nullptr;
	}
	if (particleUAV_)
	{
		particleUAV_->Release();
		particleUAV_ = nullptr;
	}
	if (vertexShader_)
	{
		vertexShader_->Release();
		vertexShader_ = nullptr;
	}
	if (geometryShader_)
	{
		geometryShader_->Release();
		geometryShader_ = nullptr;
	}
	if (pixelShader_)
	{
		pixelShader_->Release();
		pixelShader_ = nullptr;
	}
	if (computeShader_)
	{
		computeShader_->Release();
		computeShader_ = nullptr;
	}
	if (constantBuffer_)
	{
		constantBuffer_->Release();
		constantBuffer_ = nullptr;
	}
	if (computeConstantBuffer_)
	{
		computeConstantBuffer_->Release();
		computeConstantBuffer_ = nullptr;
	}
	if (sampler_)
	{
		sampler_->Release();
		sampler_ = nullptr;
	}
	if (blendState_)
	{
		blendState_->Release();
		blendState_ = nullptr;
	}
	if (particleTexSRV_)
	{
		particleTexSRV_->Release();
		particleTexSRV_ = nullptr;
	}
}

bool ParticleSystemGPU::Initialize(ID3D11Device* device, int maxPrticles, ID3D11ShaderResourceView* particleTex)
{
	maxParticles_ = maxPrticles;
	particleTexSRV_ = particleTex;
	particleTexSRV_->AddRef();

	std::vector<ParticleGPU> initData(maxParticles_);
	for (ParticleGPU& particle : initData)
	{
		particle.position_ = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		particle.velocity_ = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		particle.lifeTime_ = 1.0f;
		particle.age_ = 1.0f;
	}

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(ParticleGPU) * maxParticles_;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = sizeof(ParticleGPU);

	D3D11_SUBRESOURCE_DATA srd = {};
	srd.pSysMem = initData.data();

	if (FAILED(device->CreateBuffer(&bd, &srd, &particleBuffer_)))
	{
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = DXGI_FORMAT_UNKNOWN;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvd.Buffer.FirstElement = 0;
	srvd.Buffer.NumElements = maxParticles_;

	if (FAILED(device->CreateShaderResourceView(particleBuffer_, &srvd, &particleSRV_)))
	{
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = {};
	uavd.Format = DXGI_FORMAT_UNKNOWN;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavd.Buffer.FirstElement = 0;
	uavd.Buffer.NumElements = maxParticles_;
	uavd.Buffer.Flags = 0;

	if (FAILED(device->CreateUnorderedAccessView(particleBuffer_, &uavd, &particleUAV_)))
	{
		return false;
	}

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* gsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* csBlob = nullptr;

	if (FAILED(LoadCSO(L"..\\x64\\Debug\\VertexShaderGPU.cso", &vsBlob)))
	{
		return false;
	}
	if (FAILED(LoadCSO(L"..\\x64\\Debug\\GeometryShader.cso", &gsBlob)))
	{
		vsBlob->Release();
		return false;
	}
	if (FAILED(LoadCSO(L"..\\x64\\Debug\\PixelShader.cso", &psBlob)))
	{
		vsBlob->Release();
		gsBlob->Release();
		return false;
	}
	if (FAILED(LoadCSO(L"..\\x64\\Debug\\ComputeShader.cso", &csBlob)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		return false;
	}
	if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		csBlob->Release();
		return false;
	}
	if (FAILED(device->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &geometryShader_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		csBlob->Release();
		return false;
	}
	if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		csBlob->Release();
		return false;
	}
	if (FAILED(device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &computeShader_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		csBlob->Release();
		return false;
	}
	vsBlob->Release();
	gsBlob->Release();
	psBlob->Release();
	csBlob->Release();

	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(device->CreateBuffer(&cbd, nullptr, &constantBuffer_)))
	{
		return false;
	}

	D3D11_BUFFER_DESC csbd = {};
	csbd.Usage = D3D11_USAGE_DEFAULT;
	csbd.ByteWidth = sizeof(ComputeConstantBuffer);
	csbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(device->CreateBuffer(&csbd, nullptr, &computeConstantBuffer_)))
	{
		return false;
	}

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC& rt0 = blendDesc.RenderTarget[0];
	rt0.BlendEnable = TRUE;
	rt0.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt0.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt0.BlendOp = D3D11_BLEND_OP_ADD;
	rt0.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt0.DestBlendAlpha = D3D11_BLEND_ZERO;
	rt0.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rt0.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(device->CreateBlendState(&blendDesc, &blendState_)))
	{
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(device->CreateSamplerState(&samplerDesc, &sampler_)))
	{
		return false;
	}

	return true;
}

void ParticleSystemGPU::Update(ID3D11DeviceContext* ctx, float delta)
{
	ComputeConstantBuffer cs = {};
	cs.delta_ = delta;
	cs.maxParticles_ = (UINT)maxParticles_;
	cs.time_ = gTimeAcc_;
	cs.spawnMode_ = gGpuPatternMode_; // 0 : Æø¹ß, 1 : ºÐ¼ö

	ctx->UpdateSubresource(computeConstantBuffer_, 0, nullptr, &cs, 0, 0);

	ctx->CSSetShader(computeShader_, nullptr, 0);
	ctx->CSSetConstantBuffers(1, 1, &computeConstantBuffer_);
	ctx->CSSetUnorderedAccessViews(0, 1, &particleUAV_, nullptr);

	UINT groupCount = (maxParticles_ + 255) / 256;
	ctx->Dispatch(groupCount, 1, 1);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ctx->CSSetShader(nullptr, nullptr, 0);
}

void ParticleSystemGPU::Draw(ID3D11DeviceContext* ctx, const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraRight, const DirectX::XMFLOAT3& cameraUp)
{
	ConstantBuffer cb = {};
	DirectX::XMStoreFloat4x4(&cb.viewProj_, DirectX::XMMatrixTranspose(viewProj));
	cb.cameraRight_ = cameraRight;
	cb.startSize_ = 0.5f;
	cb.cameraUp_ = cameraUp;
	cb.endSize_ = 0.1f;

	cb.startColor_ = DirectX::XMFLOAT4(0.1f, 0.6f, 1.0f, 1.0f);
	cb.endColor_ = DirectX::XMFLOAT4(0.1f, 0.1f, 1.0f, 0.0f);

	ctx->UpdateSubresource(constantBuffer_, 0, nullptr, &cb, 0, 0);

	ctx->IASetInputLayout(nullptr);
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullVB = nullptr;
	ctx->IASetVertexBuffers(0, 1, &nullVB, &stride, &offset);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	ctx->VSSetShader(vertexShader_, nullptr, 0);
	ctx->GSSetShader(geometryShader_, nullptr, 0);
	ctx->PSSetShader(pixelShader_, nullptr, 0);

	ctx->PSSetConstantBuffers(0, 1, &constantBuffer_);
	ctx->GSSetConstantBuffers(0, 1, &constantBuffer_);

	ctx->VSSetShaderResources(1, 1, &particleSRV_);

	ctx->PSSetShaderResources(0, 1, &particleTexSRV_);
	ctx->PSSetSamplers(0, 1, &sampler_);

	float blendFactor[4] = { 0, 0, 0, 0 };
	UINT samplerMask = 0xffffffff;
	ctx->OMSetBlendState(blendState_, blendFactor, samplerMask);

	ctx->Draw(maxParticles_, 0);

	ctx->GSSetShader(nullptr, nullptr, 0);

	ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
	ctx->VSSetShaderResources(1, 1, nullSRV);
}
