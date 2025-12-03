#include "stdafx.h"
#include "ParticleSystemCPU.h"
#include "ParticleSystemGPU.h"

ParticleSystemCPU::ParticleSystemCPU()
	: maxParticleCnt_(0)
	, aliveParticleCnt_(0)
	, pVertexBuffer_(nullptr)
	, pInputLayout_(nullptr)
	, pConstantBuffer_(nullptr)
	, pVertexShader_(nullptr)
	, pGeometryShader_(nullptr)
	, pPixelShader_(nullptr)
	, pBlendState_(nullptr)
	, pSamplerState_(nullptr)
	, pParticleTextureSRV_(nullptr)
{
}

ParticleSystemCPU::~ParticleSystemCPU()
{
	CleanUp();
}


bool ParticleSystemCPU::Init(ID3D11Device* pDevice, unsigned int maxParticleCnt, ID3D11ShaderResourceView* pTextureSRV)
{
	if (false == InitVertexBuffer(pDevice, maxParticleCnt))
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

bool ParticleSystemCPU::InitVertexBuffer(ID3D11Device* pDevice, unsigned int maxParticleCnt)
{
	maxParticleCnt_ = maxParticleCnt;
	particles_.resize(maxParticleCnt_);

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = maxParticleCnt_ * sizeof(ParticleVertexCPU);
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	//desc.MiscFlags;
	//desc.StructureByteStride;

	//D3D11_SUBRESOURCE_DATA data = {};
	//data.pSysMem = particles_.data();
	//data.SysMemPitch;
	//data.SysMemSlicePitch;

	if (FAILED(pDevice->CreateBuffer(&desc, nullptr, &pVertexBuffer_)))
	{
		DEBUG_BREAK();
		CleanUp();
		return false;
	}

	return true;
}

bool ParticleSystemCPU::InitShaders(ID3D11Device* pDevice)
{
	const wchar_t* pVSPath = L"VertexShaderCPU.cso";
	const wchar_t* pPSPath = L"PixelShader.cso";
	const wchar_t* pGSPath = L"GeometryShader.cso";

	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;
	ID3DBlob* pGSBlob = nullptr;

	do
	{
		if (FAILED(D3DReadFileToBlob(pVSPath, &pVSBlob))) break;
		if (FAILED(D3DReadFileToBlob(pPSPath, &pPSBlob))) break;
		if (FAILED(D3DReadFileToBlob(pGSPath, &pGSBlob))) break;
		if (FAILED(pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader_))) break;
		if (FAILED(pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShader_))) break;
		if (FAILED(pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &pGeometryShader_))) break;

		D3D11_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"AGE", 0, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		if (FAILED(pDevice->CreateInputLayout(inputDesc, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pInputLayout_))) break;

		return true;
	} while (true);



	if (nullptr != pInputLayout_)
	{
		pInputLayout_->Release();
		pInputLayout_ = nullptr;
	}
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
bool ParticleSystemCPU::InitConstantBuffer(ID3D11Device* pDevice)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(ConstantBuffer);
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	//desc.CPUAccessFlags;
	//desc.MiscFlags;	
	//desc.StructureByteStride;

	HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, &pConstantBuffer_);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return false;
	}

	return true;
}
bool ParticleSystemCPU::InitBlendState(ID3D11Device* pDevice)
{
	D3D11_BLEND_DESC desc = {};
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC& rt0 = desc.RenderTarget[0];
	rt0.BlendEnable = TRUE;
	rt0.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt0.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt0.BlendOp = D3D11_BLEND_OP_ADD;
	rt0.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt0.DestBlendAlpha = D3D11_BLEND_ZERO;
	rt0.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rt0.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(pDevice->CreateBlendState(&desc, &pBlendState_)))
	{
		DEBUG_BREAK();
		return false;
	}

	return true;
}
bool ParticleSystemCPU::InitSamplerState(ID3D11Device* pDevice)
{
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	//desc.MipLODBias;
	//desc.MaxAnisotropy;

	if (FAILED(pDevice->CreateSamplerState(&desc, &pSamplerState_)))
	{
		DEBUG_BREAK();
		return false;
	}

	return true;
}

void ParticleSystemCPU::EmitParticle()
{
	for (ParticleCPU& p : particles_)
	{
		if (1.0f <= p.age_)
		{
			p.pos_ = DirectX::XMFLOAT3(0, 0, 0);
			p.vel_ = DirectX::XMFLOAT3
			(
				(float(rand()) / RAND_MAX - 0.5f) * 2.0f,
				5.0f + (float(rand()) / RAND_MAX) * 2.0f,
				(float(rand()) / RAND_MAX - 0.5f) * 2.0f
			);
			p.lifeTime_ = 1.0f;
			p.age_ = 0.0f;
			break;
		}
	}
}

void ParticleSystemCPU::Tick(ID3D11DeviceContext* pDeviceContext, float deltaTime)
{
	EmitParticle();

	for (ParticleCPU& p : particles_)
	{
		if (p.age_ < 1.0f)
		{
			float life = p.lifeTime_;
			float ageSec = p.age_ * life + deltaTime;
			if (life <= ageSec)
			{
				p.age_ = 1.0f;
			}
			else
			{
				p.age_ = ageSec / life;
				p.pos_.x += p.vel_.x * deltaTime;
				p.pos_.y += p.vel_.y * deltaTime;
				p.pos_.z += p.vel_.z * deltaTime;

				p.vel_.y -= 9.8f * 0.5f * deltaTime;
			}
		}
	}

	std::vector<ParticleVertexCPU> verts;
	verts.reserve(aliveParticleCnt_);

	for (ParticleCPU& p : particles_)
	{
		if (p.age_ < 1.0f)
		{
			ParticleVertexCPU newVert;
			newVert.pos_ = p.pos_;
			newVert.age_ = p.age_;
			verts.push_back(newVert);
		}
	}

	aliveParticleCnt_ = (unsigned int)verts.size();
	if (0 == aliveParticleCnt_)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(pDeviceContext->Map(pVertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		memcpy(mapped.pData, verts.data(), sizeof(ParticleVertexCPU) * aliveParticleCnt_);
		pDeviceContext->Unmap(pVertexBuffer_, 0);
	}
}

void ParticleSystemCPU::Render
(
	ID3D11DeviceContext* pDeviceContext,
	const DirectX::XMMATRIX& viewProj,
	const DirectX::XMFLOAT3& cameraRight,
	const DirectX::XMFLOAT3& cameraUp
)
{
	if (0 == aliveParticleCnt_)
	{
		return;
	}

	ConstantBuffer cb = {};
	DirectX::XMStoreFloat4x4(&cb.viewProj_, DirectX::XMMatrixTranspose(viewProj));
	cb.cameraRight_ = cameraRight;
	cb.startSize_ = 0.5f;
	cb.cameraUp_ = cameraUp;
	cb.endSize_ = 0.1f;
	cb.startColor_ = DirectX::XMFLOAT4(0.5f, 0.5f, 0.0f, 1.0f);
	cb.endColor_ = DirectX::XMFLOAT4(0.1f, 0.5f, 0.0f, 1.0f);
	pDeviceContext->UpdateSubresource(pConstantBuffer_, 0, nullptr, &cb, 0, 0);


	UINT stirde = sizeof(ParticleVertexCPU);
	UINT offset = 0;
	pDeviceContext->IASetInputLayout(pInputLayout_);
	pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer_, &stirde, &offset);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pDeviceContext->VSSetShader(pVertexShader_, nullptr, 0);

	pDeviceContext->GSSetShader(pGeometryShader_, nullptr, 0);
	pDeviceContext->GSSetConstantBuffers(0, 1, &pConstantBuffer_);

	pDeviceContext->PSSetShader(pPixelShader_, nullptr, 0);
	pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer_);
	pDeviceContext->PSSetShaderResources(0, 1, &pParticleTextureSRV_);
	pDeviceContext->PSSetSamplers(0, 1, &pSamplerState_);

	float blendFactor[4] = { 0, 0, 0, 0 };
	UINT samplerMask = 0xffffffff;
	pDeviceContext->OMSetBlendState(pBlendState_, blendFactor, samplerMask);

	pDeviceContext->Draw(aliveParticleCnt_, 0);

	pDeviceContext->GSSetShader(nullptr, nullptr, 0);
}


void ParticleSystemCPU::CleanUp()
{
	if (nullptr != pParticleTextureSRV_)
	{
		pParticleTextureSRV_->Release();
		pParticleTextureSRV_ = nullptr;
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
	if (nullptr != pConstantBuffer_)
	{
		pConstantBuffer_->Release();
		pConstantBuffer_ = nullptr;
	}
	if (nullptr != pInputLayout_)
	{
		pInputLayout_->Release();
		pInputLayout_ = nullptr;
	}
	if (nullptr != pVertexBuffer_)
	{
		pVertexBuffer_->Release();
		pVertexBuffer_ = nullptr;
	}
}




