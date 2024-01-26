// Copyright 2023 dest1yo. All Rights Reserved.

#pragma once

#include "Animation/AnimNodeBase.h"
#include "Animation/InputScaleBias.h"
#include "AnimNode_MDA.generated.h" 

UENUM()
enum class EMDABlendMode : uint8
{
	Add UMETA(DisplayName="Add"),
	Subtract UMETA(DisplayName="Subtract"),
	CoDAdd UMETA(DisplayName="CoD Add"),
};

// MDA; has dynamic number of blendposes
USTRUCT(BlueprintInternalUseOnly)
struct MDARUNTIME_API FAnimNode_MDA : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

public:
	/** The source pose */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Links)
	FPoseLink BasePose;

	/** Each layer's blended pose */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category=Links, meta=(BlueprintCompilerGeneratedDefaults))
	TArray<FPoseLink> Poses;

	/** The weights of each layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category=Settings, meta=(BlueprintCompilerGeneratedDefaults, PinShownByDefault))
	TArray<float> BlendWeights;

	/** Switch blend modes to blend poses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category=Config, meta=(BlueprintCompilerGeneratedDefaults))
	TArray<EMDABlendMode> BlendModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Alpha)
	FInputScaleBiasClamp AlphaScaleBiasClamp;

	/** How to blend the curve of layers together */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Config)
	TEnumAsByte<ECurveBlendOption::Type> CurveBlendOption;

private:
	TArray<float> ActualAlphas;

public:
	FAnimNode_MDA(): CurveBlendOption(ECurveBlendOption::BlendByWeight)
	{
	}

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	int32 AddPose()
	{
		Poses.AddDefaulted();
		BlendWeights.Add(1.f);
		BlendModes.AddDefaulted();

		return Poses.Num();
	}

	void RemovePose(int32 PoseIndex)
	{
		Poses.RemoveAt(PoseIndex);
		BlendWeights.RemoveAt(PoseIndex);
		BlendModes.RemoveAt(PoseIndex);
	}

	void ResetPoses()
	{
		Poses.Reset();
		BlendWeights.Reset();
		BlendModes.Reset();
	}

private:
	void AccumulateAdditivePose(
	TArrayView<const FCompactPose> SourcePoses,
	TArrayView<const FBlendedCurve> SourceCurves,
	TArrayView<const UE::Anim::FStackAttributeContainer> SourceAttributes,
	TArrayView<const float> SourceWeights,
	TArrayView<const EMDABlendMode> SourceBlendModes,
	FAnimationPoseData& OutAnimationPoseData
	);

	static void BlendCurves1(const TArrayView<const FBlendedCurve> SourceCurves, const TArrayView<const float> SourceWeights, FBlendedCurve& OutCurve, ECurveBlendOption::Type BlendOption);
};

/** Accumulates weighted AdditivePose to BasePose. Rotations are NOT normalized. */
template <EMDABlendMode>
static void AccumulateAdditivePoseInternal(FCompactPose& BasePose, const FCompactPose& AdditivePose, float Weight);

template <>
inline void AccumulateAdditivePoseInternal<EMDABlendMode::Add>(FCompactPose& BasePose, const FCompactPose& AdditivePose, float Weight)
{
	// Check wight value
	if (!FAnimWeight::IsRelevant(Weight))
		return;

	for (const FCompactPoseBoneIndex BoneIndex : BasePose.ForEachBoneIndex())
	{
		FTransform& BaseTransform = BasePose[BoneIndex];
		FTransform AdditiveTransform = AdditivePose[BoneIndex];
		AdditiveTransform.BlendWith(FTransform::Identity, 1.f - Weight);

		BaseTransform.SetLocation(BaseTransform.GetLocation() + AdditiveTransform.GetLocation());
		BaseTransform.SetRotation(BaseTransform.GetRotation() * AdditiveTransform.GetRotation());
		BaseTransform.SetScale3D(UE::Math::TVector<double>::One());
	}
}

template <>
inline void AccumulateAdditivePoseInternal<EMDABlendMode::Subtract>(FCompactPose& BasePose, const FCompactPose& AdditivePose, float Weight)
{
	// Check wight value
	if (!FAnimWeight::IsRelevant(Weight))
		return;

	for (const FCompactPoseBoneIndex BoneIndex : BasePose.ForEachBoneIndex())
	{
		FTransform& BaseTransform = BasePose[BoneIndex];
		FTransform AdditiveTransform = AdditivePose[BoneIndex];
		AdditiveTransform.BlendWith(FTransform::Identity, 1.f - Weight);

		BaseTransform.SetLocation(BaseTransform.GetLocation() - AdditiveTransform.GetLocation());
		BaseTransform.SetRotation(BaseTransform.GetRotation() * AdditiveTransform.GetRotation().Inverse());
		BaseTransform.SetScale3D(UE::Math::TVector<double>::One());
	}
}

template <>
inline void AccumulateAdditivePoseInternal<EMDABlendMode::CoDAdd>(FCompactPose& BasePose, const FCompactPose& AdditivePose, float Weight)
{
	// Check wight value
	if (!FAnimWeight::IsRelevant(Weight))
		return;

	for (const FCompactPoseBoneIndex BoneIndex : BasePose.ForEachBoneIndex())
	{
		FTransform RefTransform = BasePose.GetRefPose(BoneIndex);
		FTransform& BaseTransform = BasePose[BoneIndex];
		FTransform AdditiveTransform = AdditivePose[BoneIndex];
		AdditiveTransform.BlendWith(FTransform::Identity, 1.f - Weight);

		BaseTransform.SetLocation(BaseTransform.GetLocation() + AdditiveTransform.GetLocation() - RefTransform.GetLocation());
		BaseTransform.SetRotation(BaseTransform.GetRotation() * AdditiveTransform.GetRotation());
		BaseTransform.SetScale3D(UE::Math::TVector<double>::One());
	}
}
