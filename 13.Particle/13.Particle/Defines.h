#pragma once
struct ConstantBuffer
{
	DirectX::XMFLOAT4X4 viewProj_;
	DirectX::XMFLOAT3 cameraRight_;
	float startSize_;
	DirectX::XMFLOAT3 cameraUp_;
	float endSize_;
	DirectX::XMFLOAT4 startColor_;
	DirectX::XMFLOAT4 endColor_;
};