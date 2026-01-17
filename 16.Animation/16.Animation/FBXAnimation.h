#pragma once


struct BoneKeyframe
{
	FbxTime time;
	FbxAMatrix localTransform;
	FbxAMatrix globalTransform;
};

struct BoneAnimation
{
	std::string boneName;
	std::vector<BoneKeyframe> keyframes;
};

struct AnimationClip
{
	std::string name;
	double duration;     // seconds
	double frameRate;

	std::vector<BoneAnimation> boneAnimations; // bone index ±‚¡ÿ
};

class FBXAnimation
{
public:
	FBXAnimation();
	~FBXAnimation();


	AnimationClip animationClip_;
};