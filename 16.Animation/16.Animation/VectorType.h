#pragma once

#pragma once

struct Float4 {
	union {
		struct {
			float X;
			float Y;
			float Z;
			float W;
		};
		struct {
			float R;
			float G;
			float B;
			float A;
		};
		float Arr1D[4];
	};

	Float4()
		: X(0.0f), Y(0.0f), Z(0.0f), W(1.0f) {
	}

	Float4(float x, float y, float z)
		: X(x), Y(y), Z(z), W(1.0f) {
	}

	Float4(float x, float y, float z, float w)
		: X(x), Y(y), Z(z), W(w) {
	}

	Float4 operator+(const Float4& rhs)
	{
		return { this->X + rhs.X, this->Y + rhs.Y, this->Z + rhs.Z, this->W + rhs.W };
	}
	Float4& operator+=(const Float4& rhs)
	{
		this->X += rhs.X;
		this->Y += rhs.Y;
		this->Z += rhs.Z;
		this->W += rhs.W;

		return *this;
	}

	Float4 operator-(const Float4& rhs)
	{
		return { this->X - rhs.X, this->Y - rhs.Y, this->Z - rhs.Z,  this->W - rhs.W };
	}

	Float4& operator-=(const Float4& rhs)
	{
		this->X -= rhs.X;
		this->Y -= rhs.Y;
		this->Z -= rhs.Z;
		this->W -= rhs.W;
		return *this;
	}

	Float4 operator*(const Float4& rhs)
	{
		return { this->X * rhs.X, this->Y * rhs.Y, this->Z * rhs.Z,this->W * rhs.W };
	}

	Float4& operator*=(const Float4& rhs)
	{
		this->X *= rhs.X;
		this->Y *= rhs.Y;
		this->Z *= rhs.Z;
		this->W *= rhs.W;

		return *this;
	}
};

struct Float3 {
	union {
		struct {
			float X;
			float Y;
			float Z;
		};
		float Arr1D[3];
	};

	Float3()
		: X(0.0f), Y(0.0f), Z(0.0f) {
	}

	Float3(float x, float y, float z)
		: X(x), Y(y), Z(z) {
	}

	Float3 operator+(const Float3& rhs)
	{
		return { this->X + rhs.X, this->Y + rhs.Y, this->Z + rhs.Z };
	}

	Float3& operator+=(const Float3& rhs)
	{
		this->X += rhs.X;
		this->Y += rhs.Y;
		this->Z += rhs.Z;

		return *this;
	}

	Float3 operator-(const Float3& rhs)
	{
		return { this->X - rhs.X, this->Y - rhs.Y, this->Z - rhs.Z };
	}

	Float3& operator-=(const Float3& rhs)
	{
		this->X -= rhs.X;
		this->Y -= rhs.Y;
		this->Z -= rhs.Z;
		return *this;
	}

	Float3 operator*(const Float3& rhs)
	{
		return { this->X * rhs.X, this->Y * rhs.Y, this->Z * rhs.Z };
	}

	Float3& operator*=(const Float3& rhs)
	{
		this->X *= rhs.X;
		this->Y *= rhs.Y;
		this->Z *= rhs.Z;

		return *this;
	}
};

struct Float2 {
	union {
		struct {
			float X;
			float Y;
		};
		float Arr1D[2];
	};

	Float2()
		: X(0.0f), Y(0.0f) {
	}

	Float2(float x, float y)
		: X(x), Y(y) {
	}

	Float2 operator+(const Float2& rhs)
	{
		return { this->X + rhs.X, this->Y + rhs.Y };
	}
	Float2& operator+=(const Float2& rhs)
	{
		this->X += rhs.X;
		this->Y += rhs.Y;

		return *this;
	}

	Float2 operator-(const Float2& rhs)
	{
		return { this->X - rhs.X, this->Y - rhs.Y };
	}

	Float2& operator-=(const Float2& rhs)
	{
		this->X -= rhs.X;
		this->Y -= rhs.Y;

		return *this;
	}

	Float2 operator*(const Float2& rhs)
	{
		return { this->X * rhs.X, this->Y * rhs.Y };
	}

	Float2& operator*=(const Float2& rhs)
	{
		this->X *= rhs.X;
		this->Y *= rhs.Y;

		return *this;
	}
};

typedef Float4 Vector;
typedef Float4 Color;

namespace MATH
{
	const static float PI = 3.141592654f;
	const static float DegToRadFloat = PI / 180.0f;
	const static float RadToDegFloat = 180.0f / PI;

	static const Float4 Left = { 0.0f, -1.0f,  0.0f, 0.0f };
	static const Float4 Right = { 0.0f,  1.0f,  0.0f, 0.0f };
	static const Float4 Up = { 0.0f,  0.0f,  1.0f, 0.0f };
	static const Float4 Down = { 0.0f,  0.0f, -1.0f, 0.0f };
	static const Float4 Front = { 1.0f,  0.0f,  0.0f, 0.0f };
	static const Float4 Back = { -1.0f,  0.0f,  0.0f, 0.0f };

	static const Float4 DegToRad(DegToRadFloat, DegToRadFloat, DegToRadFloat, 1.0f);
}

struct __declspec(align(16)) Float4x4
{
	Float4 r[4];
};

void VectorDot(float& out, const Float4& lhs, const Float4& rhs);
void VectorCross(Float4& out, const Float4& lhs, const Float4& rhs);
void VectorAdd(Float4& out, const Float4& lhs, const Float4& rhs);
void VectorSub(Float4& out, const Float4& lhs, const Float4& rhs);
void VectorMultiply(Float4& out, const Float4& lhs, const Float4& rhs);
void VectorScale(Float4& out, const Float4& lhs, float scale);
void VectorNormalize(Float4& out, const Float4& lhs);
float VectorLength(const Float4& lhs);

void VectorDot(float& out, const Float3& lhs, const Float3& rhs);
void VectorCross(Float3& out, const Float3& lhs, const Float3& rhs);
void VectorAdd(Float3& out, const Float3& lhs, const Float3& rhs);
void VectorSub(Float3& out, const Float3& lhs, const Float3& rhs);
void VectorMultiply(Float3& out, const Float3& lhs, const Float3& rhs);
void VectorScale(Float3& out, const Float3& lhs, float scale);
void VectorNormalize(Float3& out, const Float3& lhs);
float VectorLength(const Float3& lhs);

void QuaternionToEulerDeg(Float4& out, const Float4& Q);
void QuaternionToEulerRad(Float4& out, const Float4& Q);

void MatrixIdentity(Float4x4& out);
void MatrixTranspose(Float4x4& out, const Float4x4& src);
void MatrixCompose(Float4x4& out, const Float4& scale, const Float4& rotAngle, const Float4& pos);
void MatrixDecompose(const Float4x4& matrx, Float4& scale, Float4& rotQ, Float4& pos);
void MatrixDecomposeFromRotQ(const Float4x4& matrx, Float4& rotQ);
void MatrixLookToLH(Float4x4& out, const Float4& eyePos, const Float4& eyeDir, const Float4& eyeUp);
void MatrixPerspectiveFovLH(Float4x4& out, float fovDegAngle, float aspectRatio, float nearZ, float farZ);

float ConvertDegToRad(float deg);
float ConvertRadToDeg(float rad);