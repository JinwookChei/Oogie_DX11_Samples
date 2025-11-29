#include "stdafx.h"
#include "Transform.h"

#include "stdafx.h"
#include "Transform.h"

Transform::Transform()
	: scale_(1, 1, 1, 0),
	rotation_(0, 0, 0, 0),
	position_(0, 0, 0, 1),
	quaternion_(0, 0, 0, 1)
{
	MatrixIdentity(scaleMatrix_);
	MatrixIdentity(rotationMatrix_);
	MatrixIdentity(positionMatrix_);
	MatrixIdentity(worldMatrix_);
}

Transform::~Transform()
{
}

void Transform::SetScale(const Float4& scale)
{
	scale_ = scale;

	TransformUpdate();
}

void Transform::SetRotation(const Float4& rotation)
{
	rotation_ = rotation;

	TransformUpdate();
}

void Transform::AddRotaion(const Float4& offset)
{
	VectorAdd(rotation_, rotation_, offset);

	TransformUpdate();
}

void Transform::AddRotaionX(float offset)
{
	rotation_.X += offset;

	TransformUpdate();
}

void Transform::AddRotaionY(float offset)
{
	rotation_.Y += offset;

	TransformUpdate();
}

void Transform::AddRotaionZ(float offset)
{
	rotation_.Z += offset;

	TransformUpdate();
}

void Transform::SetPosition(const Float4& position)
{
	position_ = position;

	TransformUpdate();
}

void Transform::AddPosition(const Float4& offset)
{
	VectorAdd(position_, position_, offset);

	TransformUpdate();
}

void Transform::AddPositionX(float offset)
{
}

void Transform::AddPositionY(float offset)
{
}

void Transform::AddPositionZ(float offset)
{
}

const Float4x4& Transform::GetWorldMatrix() const
{
	return worldMatrix_;
}

const Float4x4 Transform::GetWorldMatrixTranspose() const
{
	Float4x4 ret;
	MatrixTranspose(ret, worldMatrix_);
	return ret;
}

const Float4& Transform::GetScale() const
{
	return scale_;
}

const Float4& Transform::GetRotation() const
{
	return rotation_;
}

const Float4& Transform::GetPosition() const
{
	return position_;
}

DirectX::XMMATRIX Transform::GetScaleMatrix() const
{	
	return DirectX::XMMatrixScaling(scale_.X, scale_.Y, scale_.Z);
}

DirectX::XMMATRIX Transform::GetRotationMatrix() const
{
	return DirectX::XMMatrixRotationRollPitchYaw(rotation_.X, rotation_.Y, rotation_.Z);
}

DirectX::XMMATRIX Transform::GetPositionMatrix() const
{
	return DirectX::XMMatrixTranslation(position_.X, position_.Y, position_.Z);
}

Float4 Transform::ForwardVector() const
{
	Float4 normal;
	VectorNormalize(normal, worldMatrix_.r[0]);
	return normal;
}

Float4 Transform::UpVector() const
{
	Float4 normal;
	VectorNormalize(normal, worldMatrix_.r[2]);
	return normal;
}

Float4 Transform::RightVector() const
{
	Float4 normal;
	VectorNormalize(normal, worldMatrix_.r[1]);
	return normal;
}

void Transform::TransformUpdate()
{
	MatrixCompose(worldMatrix_, scale_, rotation_, position_);

	/*Float4 roQ;
	MatrixDecomposeFromRotQ(worldMatrix_, roQ);

	Float4 roDeg;
	QuaternionToEulerDeg(roDeg, roQ);

	Float4x4 tmpMatrix;
	MatrixCompose(tmpMatrix, scale_, roDeg, position_);*/
}
