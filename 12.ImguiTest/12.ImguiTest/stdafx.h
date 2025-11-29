#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // 꼭 추가 (min/max 매크로 충돌 방지)

#include <Windows.h>
#include <string>
#include <math.h>

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <DirectXTex.h>

#pragma comment(lib, "DirectXTex")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxgi")

// ------------------------------
// ImGui / ImGuizmo 관련
// ------------------------------
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"   // ImGuizmo 내부에서 일부 심볼 사용
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"
// ------------------------------

#ifdef _DEBUG
//#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <vector>

#ifdef _DEBUG
#define DEBUG_BREAK __debugbreak
#else
#define DEBUG_BREAK
#endif

#include "VectorType.h"
#include "Transform.h"
