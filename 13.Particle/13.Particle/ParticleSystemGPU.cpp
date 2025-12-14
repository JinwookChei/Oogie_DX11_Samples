#include "stdafx.h"
#include "ParticleSystemGPU.h"


//float ParticleSystemGPU::gTimeAcc_ = 0.0f;
//int ParticleSystemGPU::gGpuPatternMode_ = 0;

ParticleSystemGPU::ParticleSystemGPU()
	: maxParticleCnt_(0)
	, pParticleBuffer_(nullptr)
	, pVertexShader_(nullptr)
	, pGeometryShader_(nullptr)
	, pPixelShader_(nullptr)
	, pComputeShader_(nullptr)
	, pConstantBuffer_(nullptr)
	, pComputeConstantBuffer_(nullptr)
	, pBlendState_(nullptr)
	, pSamplerState_(nullptr)
	, pParticleSRV_(nullptr)
	, pParticleUAV_(nullptr)
	, pParticleTextureSRV_(nullptr)
	, gTimeAcc_(0)
{
}

ParticleSystemGPU::~ParticleSystemGPU()
{
	CleanUp();
}

bool ParticleSystemGPU::Init(ID3D11Device* pDevice, unsigned int maxParticleCnt, ID3D11ShaderResourceView* pTextureSRV)
{
	if (false == InitParticleBuffer(pDevice, maxParticleCnt))
	{
		DEBUG_BREAK();
		return false;
	}

	if (false == InitShaders(pDevice))
	{
		DEBUG_BREAK();
		return false;
	}

	if (false == InitConstantBuffer(pDevice))
	{
		DEBUG_BREAK();
		return false;
	}

	if (false == InitBlendState(pDevice))
	{
		DEBUG_BREAK();
		return false;
	}

	if (false == InitSamplerState(pDevice))
	{
		DEBUG_BREAK();
		return false;
	}

	if (nullptr == pTextureSRV)
	{
		DEBUG_BREAK();
		return false;
	}

	pParticleTextureSRV_ = pTextureSRV;
	pParticleTextureSRV_->AddRef();

	return true;
}

void ParticleSystemGPU::Tick(ID3D11DeviceContext* pDeviceContext, float deltaTime)
{
	gTimeAcc_ += deltaTime;

	ComputeConstantBuffer ccBuffer = {};
	ccBuffer.deltaTime_ = deltaTime;
	ccBuffer.maxParticles_ = (UINT)maxParticleCnt_;
	ccBuffer.time_ = gTimeAcc_;
	ccBuffer.spawnMode_ = gGpuPatternMode_; // 0 : Æø¹ß, 1 : ºÐ¼ö
	pDeviceContext->UpdateSubresource(pComputeConstantBuffer_, 0, nullptr, &ccBuffer, 0, 0);	//

	pDeviceContext->CSSetShader(pComputeShader_, nullptr, 0);	//
	pDeviceContext->CSSetConstantBuffers(1, 1, &pComputeConstantBuffer_);	//
	pDeviceContext->CSSetUnorderedAccessViews(0, 1, &pParticleUAV_, nullptr);	//

	UINT groupCount = (maxParticleCnt_ + 255) / 256;
	pDeviceContext->Dispatch(groupCount, 1, 1);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	pDeviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	pDeviceContext->CSSetShader(nullptr, nullptr, 0);
}

void ParticleSystemGPU::Render(ID3D11DeviceContext* pDeviceContext, const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraRight, const DirectX::XMFLOAT3& cameraUp)
{
	ConstantBuffer cb = {};
	DirectX::XMStoreFloat4x4(&cb.viewProj_, DirectX::XMMatrixTranspose(viewProj));
	cb.cameraRight_ = cameraRight;
	cb.startSize_ = 0.5f;
	cb.cameraUp_ = cameraUp;
	cb.endSize_ = 0.1f;
	cb.startColor_ = DirectX::XMFLOAT4(0.1f, 0.6f, 1.0f, 1.0f);
	cb.endColor_ = DirectX::XMFLOAT4(0.1f, 0.1f, 1.0f, 0.0f);
	pDeviceContext->UpdateSubresource(pConstantBuffer_, 0, nullptr, &cb, 0, 0);

	pDeviceContext->IASetInputLayout(nullptr);
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullVB = nullptr;
	pDeviceContext->IASetVertexBuffers(0, 1, &nullVB, &stride, &offset);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pDeviceContext->VSSetShader(pVertexShader_, nullptr, 0);		
	pDeviceContext->GSSetShader(pGeometryShader_, nullptr, 0);		
	pDeviceContext->PSSetShader(pPixelShader_, nullptr, 0);		

	pDeviceContext->GSSetConstantBuffers(0, 1, &pConstantBuffer_);
	pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer_);

	pDeviceContext->VSSetShaderResources(1, 1, &pParticleSRV_);

	pDeviceContext->PSSetShaderResources(0, 1, &pParticleTextureSRV_);

	pDeviceContext->PSSetSamplers(0, 1, &pSamplerState_);

	float blendFactor[4] = { 0, 0, 0, 0 };
	UINT samplerMask = 0xffffffff;
	pDeviceContext->OMSetBlendState(pBlendState_, blendFactor, samplerMask);

	pDeviceContext->Draw(maxParticleCnt_, 0);

	pDeviceContext->GSSetShader(nullptr, nullptr, 0);
	ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
	pDeviceContext->VSSetShaderResources(1, 1, nullSRV);
}


bool ParticleSystemGPU::InitParticleBuffer(ID3D11Device* pDevice, unsigned int maxParticleCnt)
{
	maxParticleCnt_ = maxParticleCnt;

	std::vector<ParticleGPU> initData(maxParticleCnt_);
	for (ParticleGPU& particle : initData)
	{
		particle.position_ = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		particle.velocity_ = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		particle.lifeTime_ = 1.0f;
		particle.age_ = 1.0f;
	}

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(ParticleGPU) * maxParticleCnt_;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(ParticleGPU);

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = initData.data();
	if (FAILED(pDevice->CreateBuffer(&desc, &data, &pParticleBuffer_)))
	{
		DEBUG_BREAK();
		return false;
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = DXGI_FORMAT_UNKNOWN;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvd.Buffer.FirstElement = 0;
	srvd.Buffer.NumElements = maxParticleCnt_;

	if (FAILED(pDevice->CreateShaderResourceView(pParticleBuffer_, &srvd, &pParticleSRV_)))
	{
		DEBUG_BREAK();
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = {};
	uavd.Format = DXGI_FORMAT_UNKNOWN;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavd.Buffer.FirstElement = 0;
	uavd.Buffer.NumElements = maxParticleCnt_;
	uavd.Buffer.Flags = 0;

	if (FAILED(pDevice->CreateUnorderedAccessView(pParticleBuffer_, &uavd, &pParticleUAV_)))
	{
		DEBUG_BREAK();
		return false;
	}


	return true;
}

bool ParticleSystemGPU::InitShaders(ID3D11Device* pDevice)
{
	const wchar_t* pCSPath = L"ComputeShader.cso";
	const wchar_t* pVSPath = L"VertexShaderGPU.cso";
	const wchar_t* pPSPath = L"PixelShader.cso";
	const wchar_t* pGSPath = L"GeometryShader.cso";
	
	ID3DBlob* pCSBlob = nullptr;
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;
	ID3DBlob* pGSBlob = nullptr;

	do
	{
		if (FAILED(D3DReadFileToBlob(pCSPath, &pCSBlob))) break;
		if (FAILED(D3DReadFileToBlob(pVSPath, &pVSBlob))) break;
		if (FAILED(D3DReadFileToBlob(pPSPath, &pPSBlob))) break;
		if (FAILED(D3DReadFileToBlob(pGSPath, &pGSBlob))) break;
		if (FAILED(pDevice->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &pComputeShader_))) break;
		if (FAILED(pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader_))) break;
		if (FAILED(pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShader_))) break;
		if (FAILED(pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &pGeometryShader_))) break;
		return true;
	} while (true);


	DEBUG_BREAK();

	if (nullptr != pGeometryShader_)
	{
		pGeometryShader_->Release();
		pGeometryShader_ = nullptr;
	}
	if (nullptr != pPixelShader_)
	{
		pPixelShader_->Release();
		pPixelShader_ = nullptr;
	}
	if (nullptr != pVertexShader_)
	{
		pVertexShader_->Release();
		pVertexShader_ = nullptr;
	}
	if (nullptr != pCSBlob)
	{
		pCSBlob->Release();
		pCSBlob = nullptr;
	}
	if (nullptr != pVSBlob)
	{
		pVSBlob->Release();
		pVSBlob = nullptr;
	}
	if (nullptr != pPSBlob)
	{
		pPSBlob->Release();
		pPSBlob = nullptr;
	}
	if (nullptr != pGSBlob)
	{
		pGSBlob->Release();
		pGSBlob = nullptr;
	}

	return false;
}

bool ParticleSystemGPU::InitConstantBuffer(ID3D11Device* pDevice)
{
	D3D11_BUFFER_DESC cDesc = {};
	cDesc.Usage = D3D11_USAGE_DEFAULT;
	cDesc.ByteWidth = sizeof(ConstantBuffer);
	cDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(pDevice->CreateBuffer(&cDesc, nullptr, &pConstantBuffer_)))
	{
		DEBUG_BREAK();
		return false;
	}

	D3D11_BUFFER_DESC ccDesc = {};
	ccDesc.Usage = D3D11_USAGE_DEFAULT;
	ccDesc.ByteWidth = sizeof(ComputeConstantBuffer);
	ccDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(pDevice->CreateBuffer(&ccDesc, nullptr, &pComputeConstantBuffer_)))
	{
		DEBUG_BREAK();
		return false;
	}

	return true;
}

bool ParticleSystemGPU::InitBlendState(ID3D11Device* pDevice)
{
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

	if (FAILED(pDevice->CreateBlendState(&blendDesc, &pBlendState_)))
	{
		DEBUG_BREAK();
		return false;
	}

	return true;
}

bool ParticleSystemGPU::InitSamplerState(ID3D11Device* pDevice)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(pDevice->CreateSamplerState(&samplerDesc, &pSamplerState_)))
	{
		DEBUG_BREAK();
		return false;
	}

	return true;
}

void ParticleSystemGPU::CleanUp()
{
	if (nullptr != pParticleTextureSRV_)
	{
		pParticleTextureSRV_->Release();
		pParticleTextureSRV_ = nullptr;
	}
	if (nullptr != pParticleUAV_)
	{
		pParticleUAV_->Release();
		pParticleUAV_ = nullptr;
	}
	if (nullptr != pParticleSRV_)
	{
		pParticleSRV_->Release();
		pParticleSRV_ = nullptr;
	}
	if (nullptr != pSamplerState_)
	{
		pSamplerState_->Release();
		pSamplerState_ = nullptr;
	}
	if (nullptr != pBlendState_)
	{
		pBlendState_->Release();
		pBlendState_ = nullptr;
	}
	if (nullptr != pComputeConstantBuffer_)
	{
		pComputeConstantBuffer_->Release();
		pComputeConstantBuffer_ = nullptr;
	}
	if (nullptr != pConstantBuffer_)
	{
		pConstantBuffer_->Release();
		pConstantBuffer_ = nullptr;
	}
	if (nullptr != pComputeShader_)
	{
		pComputeShader_->Release();
		pComputeShader_ = nullptr;
	}
	if (nullptr != pPixelShader_)
	{
		pPixelShader_->Release();
		pPixelShader_ = nullptr;
	}
	if (nullptr != pGeometryShader_)
	{
		pGeometryShader_->Release();
		pGeometryShader_ = nullptr;
	}

	if (nullptr != pVertexShader_)
	{
		pVertexShader_->Release();
		pVertexShader_ = nullptr;
	}

	if (nullptr != pParticleBuffer_)
	{
		pParticleBuffer_->Release();
		pParticleBuffer_ = nullptr;
	}
}
