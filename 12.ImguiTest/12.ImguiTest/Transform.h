#pragma once

class Transform final {
public:
	Transform();

	~Transform();

	void SetScale(const Float4& scale);

	void SetRotation(const Float4& rotation);

	void AddRotaion(const Float4& offset);

	void AddRotaionX(float offset);

	void AddRotaionY(float offset);

	void AddRotaionZ(float offset);

	void SetPosition(const Float4& position);

	void AddPosition(const Float4& offset);

	void AddPositionX(float offset);

	void AddPositionY(float offset);

	void AddPositionZ(float offset);

	const Float4x4& GetWorldMatrix() const;

	const Float4x4 GetWorldMatrixTranspose() const;

	const Float4& GetScale() const;

	const Float4& GetRotation() const;

	const Float4& GetPosition() const;

	DirectX::XMMATRIX GetScaleMatrix() const;

	DirectX::XMMATRIX GetRotationMatrix() const;

	DirectX::XMMATRIX GetPositionMatrix() const;

	Float4 ForwardVector() const;
	Float4 UpVector() const;
	Float4 RightVector() const;

private:
	void TransformUpdate();

	Float4 scale_;
	Float4 rotation_;
	Float4 quaternion_;
	Float4 position_;

	Float4x4 scaleMatrix_;
	Float4x4 rotationMatrix_;
	Float4x4 positionMatrix_;
	Float4x4 worldMatrix_;
};