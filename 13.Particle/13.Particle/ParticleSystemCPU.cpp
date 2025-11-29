#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>

#include "Defines.h"
#include "ParticleSystemCPU.h"

static HRESULT LoadCSO(const wchar_t* filePath, ID3DBlob** ppBlob)
{
	return D3DReadFileToBlob(filePath, ppBlob);
}

ParticleSystemCPU::ParticleSystemCPU()
	:maxParticles_(0),
	aliveCount_(0),
	vertexBuffer_(nullptr),
	vertexShader_(nullptr),
	geometryShader_(nullptr),
	pixelShader_(nullptr),
	inputLayout_(nullptr),
	constantBuffer_(nullptr),
	sampler_(nullptr),
	blendState_(nullptr),
	particleTexSRV_(nullptr)
{
}

ParticleSystemCPU::~ParticleSystemCPU()
{
	if (vertexBuffer_)
	{
		vertexBuffer_->Release();
		vertexBuffer_ = nullptr;
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
	if (inputLayout_)
	{
		inputLayout_->Release();
		inputLayout_ = nullptr;
	}
	if (constantBuffer_)
	{
		constantBuffer_->Release();
		constantBuffer_ = nullptr;
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

bool ParticleSystemCPU::Initialize(ID3D11Device* device, int maxPrticles, ID3D11ShaderResourceView* particleTex)
{
	maxParticles_ = maxPrticles;
	particles_.resize(maxParticles_);

	for (ParticleCPU& p : particles_)
	{
		p.position_ = DirectX::XMFLOAT3(0, 0, 0);
		p.velocity_ = DirectX::XMFLOAT3(0, 0, 0);
		p.lifeTime_ = 1.0f;
		p.age_ = 1.0f;
	}

	particleTexSRV_ = particleTex;
	particleTexSRV_->AddRef();

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(ParticleVertexCPU) * maxParticles_;
	vbd.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;

	if (FAILED(device->CreateBuffer(&vbd, nullptr, &vertexBuffer_)))
	{
		return false;
	}

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* gsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

	if (FAILED(LoadCSO(L"..\\x64\\Debug\\VertexShaderCPU.cso", &vsBlob)))
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

	if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		return false;
	}

	if (FAILED(device->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &geometryShader_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		return false;
	}

	if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layouyDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	if (FAILED(device->CreateInputLayout(layouyDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout_)))
	{
		vsBlob->Release();
		gsBlob->Release();
		psBlob->Release();
		return false;
	}

	vsBlob->Release();
	gsBlob->Release();
	psBlob->Release();

	D3D11_BUFFER_DESC constantDec = {};
	constantDec.Usage = D3D11_USAGE_DEFAULT;
	constantDec.ByteWidth = sizeof(ConstantBuffer);
	constantDec.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (FAILED(device->CreateBuffer(&constantDec, nullptr, &constantBuffer_)))
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

void ParticleSystemCPU::EmitParticle()
{
	for (ParticleCPU& p : particles_)
	{
		if (1.0f <= p.age_)
		{
			p.position_ = DirectX::XMFLOAT3(0, 0, 0);
			p.velocity_ = DirectX::XMFLOAT3(
				(float(rand()) / RAND_MAX - 0.5f) * 2.0f,
				5.0f + (float(rand()) / RAND_MAX) * 2.0f,
				(float(rand()) / RAND_MAX - 0.5f) * 2.0f);

			p.lifeTime_ = 2.0f;
			p.age_ = 0.0f;
			break;
		}
	}
}

void ParticleSystemCPU::Update(ID3D11DeviceContext* ctx, float delta)
{
	EmitParticle();

	for (ParticleCPU& p : particles_)
	{
		if (p.age_ < 1.0f)
		{
			float life = p.lifeTime_;
			float ageSec = p.age_ * life + delta;
			if (life <= ageSec)
			{
				p.age_ = 1.0f;
			}
			else
			{
				p.age_ = ageSec / life;
				p.position_.x += p.velocity_.x * delta;
				p.position_.y += p.velocity_.y * delta;
				p.position_.z += p.velocity_.z * delta;

				p.velocity_.y -= 9.8f * 0.5f * delta;
			}
		}
	}

	std::vector<ParticleVertexCPU> verts;
	verts.reserve(maxParticles_);

	for (ParticleCPU& p : particles_)
	{
		if (p.age_ < 1.0f)
		{
			ParticleVertexCPU newVert;
			newVert.position_ = p.position_;
			newVert.age_ = p.age_;
			verts.push_back(newVert);
		}
	}

	aliveCount_ = (unsigned int)verts.size();

	if (0 == aliveCount_)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(ctx->Map(vertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		memcpy(mapped.pData, verts.data(), sizeof(ParticleVertexCPU) * aliveCount_);
		ctx->Unmap(vertexBuffer_, 0);
	}
}

void ParticleSystemCPU::Draw(ID3D11DeviceContext* ctx, const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraRight, const DirectX::XMFLOAT3& cameraUp)
{
	if (0 == aliveCount_)
	{
		return;
	}

	ConstantBuffer cb;
	DirectX::XMStoreFloat4x4(&cb.viewProj_, DirectX::XMMatrixTranspose(viewProj));
	cb.cameraRight_ = cameraRight;
	cb.startSize_ = 0.5f;
	cb.cameraUp_ = cameraUp;
	cb.endSize_ = 0.1f;

	cb.startColor_ = DirectX::XMFLOAT4(0.5f, 0.5f, 0.0f, 1.0f);
	cb.endColor_ = DirectX::XMFLOAT4(0.1f, 0.5f, 0.0f, 1.0f);

	ctx->UpdateSubresource(constantBuffer_, 0, nullptr, &cb, 0, 0);

	UINT stirde = sizeof(ParticleVertexCPU);
	UINT offset = 0;
	ctx->IASetInputLayout(inputLayout_);
	ctx->IASetVertexBuffers(0, 1, &vertexBuffer_, &stirde, &offset);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	ctx->VSSetShader(vertexShader_, nullptr, 0);
	ctx->GSSetShader(geometryShader_, nullptr, 0);
	ctx->PSSetShader(pixelShader_, nullptr, 0);

	ctx->PSSetConstantBuffers(0, 1, &constantBuffer_);
	ctx->GSSetConstantBuffers(0, 1, &constantBuffer_);

	ctx->PSSetShaderResources(0, 1, &particleTexSRV_);
	ctx->PSSetSamplers(0, 1, &sampler_);

	float blendFactor[4] = { 0, 0, 0, 0 };
	UINT samplerMask = 0xffffffff;
	ctx->OMSetBlendState(blendState_, blendFactor, samplerMask);

	ctx->Draw(aliveCount_, 0);

	ctx->GSSetShader(nullptr, nullptr, 0);
}
