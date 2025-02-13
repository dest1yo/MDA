// Copyright 2023 dest1yo. All Rights Reserved.

#include "AnimNode_MDA.h"
#include "AnimationRuntime.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNode_MDA)
#endif

struct FMDAData : public TThreadSingleton<FMDAData>
{
	TArray<FCompactPose, TInlineAllocator<8>> SourcePoses;
	TArray<float, TInlineAllocator<8>> SourceWeights;
	TArray<EMDABlendMode, TInlineAllocator<8>> SourceBlendModes;
	TArray<FBlendedCurve, TInlineAllocator<8>> SourceCurves;
	TArray<UE::Anim::FStackAttributeContainer, TInlineAllocator<8>> SourceAttributes;
};

/////////////////////////////////////////////////////
// FAnimNode_MDA

void FAnimNode_MDA::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	FAnimNode_Base::Initialize_AnyThread(Context);

	// this should be consistent all the time by editor node
	if (!ensure(Poses.Num() == BlendWeights.Num()))
	{
		BlendWeights.Init(0.f, Poses.Num());
	}

	if (!ensure(Poses.Num() == BlendModes.Num()))
	{
		BlendModes.Init(EMDABlendMode::Add, Poses.Num());
	}
	// ActualAlphas = BlendWeights;
	ActualAlphas.Init(0.f, Poses.Num());

	AlphaScaleBiasClamp.Reinitialize();

	BasePose.Initialize(Context);

	for (FPoseLink& Pose : Poses)
	{
		Pose.Initialize(Context);
	}
}

void FAnimNode_MDA::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)

	BasePose.CacheBones(Context);
	
	for (FPoseLink& Pose : Poses)
	{
		Pose.CacheBones(Context);
	}
}



void FAnimNode_MDA::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FAnimationNode_MDA_Update);
	GetEvaluateGraphExposedInputs().Execute(Context);

	BasePose.Update(Context);

	for (int32 PoseIndex = 0; PoseIndex < Poses.Num(); ++PoseIndex)
	{
		ActualAlphas[PoseIndex] = AlphaScaleBiasClamp.ApplyTo(BlendWeights[PoseIndex], Context.GetDeltaTime());
		if (ActualAlphas[PoseIndex] > ZERO_ANIMWEIGHT_THRESH)
		{
			Poses[PoseIndex].Update(Context);
		}
	}
}

void FAnimNode_MDA::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)

	// this function may be reentrant when multiple multiblend nodes are chained together
	// these scratch arrays are treated as stacks below
	FMDAData& BlendData = FMDAData::Get();
	TArray<FCompactPose, TInlineAllocator<8>>& SourcePoses = BlendData.SourcePoses;
	TArray<FBlendedCurve, TInlineAllocator<8>>& SourceCurves = BlendData.SourceCurves;
	TArray<UE::Anim::FStackAttributeContainer, TInlineAllocator<8>>& SourceAttributes = BlendData.SourceAttributes;
	TArray<float, TInlineAllocator<8>>& SourceWeights = BlendData.SourceWeights;
	TArray<EMDABlendMode, TInlineAllocator<8>>& SourceBlendModes = BlendData.SourceBlendModes;

	const int32 SourcePosesInitialNum = SourcePoses.Num();
	int32 SourcePosesAdded = 0;

	if (ensure(Poses.Num() == ActualAlphas.Num()))
	{
		for (int32 PoseIndex = 0; PoseIndex < Poses.Num(); ++PoseIndex)
		{
			const float CurrentAlpha = ActualAlphas[PoseIndex];
			const EMDABlendMode CurrentBlendModes = BlendModes[PoseIndex];
			if (CurrentAlpha > ZERO_ANIMWEIGHT_THRESH)
			{
				// evaluate input pose, potentially reentering this function and pushing/popping more poses
				FPoseContext PoseContext(Output);
				Poses[PoseIndex].Evaluate(PoseContext);

				// push source pose data
				FCompactPose& SourcePose = SourcePoses.AddDefaulted_GetRef();
				SourcePose.MoveBonesFrom(PoseContext.Pose);

				FBlendedCurve& SourceCurve = SourceCurves.AddDefaulted_GetRef();
				SourceCurve.MoveFrom(PoseContext.Curve);

				UE::Anim::FStackAttributeContainer& SourceAttribute = SourceAttributes.AddDefaulted_GetRef();
				SourceAttribute.MoveFrom(PoseContext.CustomAttributes);

				SourceWeights.Add(CurrentAlpha);

				SourceBlendModes.Add(CurrentBlendModes);

				++SourcePosesAdded;
			}
		}
	}

	BasePose.Evaluate(Output);

	if (SourcePosesAdded > 0)
	{
		// obtain views onto the ends of our stacks
		TArrayView<FCompactPose> SourcePosesView = MakeArrayView(&SourcePoses[SourcePosesInitialNum], SourcePosesAdded);
		TArrayView<FBlendedCurve> SourceCurvesView = MakeArrayView(&SourceCurves[SourcePosesInitialNum], SourcePosesAdded);
		TArrayView<UE::Anim::FStackAttributeContainer> SourceAttributesView = MakeArrayView(&SourceAttributes[SourcePosesInitialNum], SourcePosesAdded);
		TArrayView<float> SourceWeightsView = MakeArrayView(&SourceWeights[SourcePosesInitialNum], SourcePosesAdded);
		TArrayView<EMDABlendMode> SourceBlendModesView = MakeArrayView(&SourceBlendModes[SourcePosesInitialNum], SourcePosesAdded);

		// Accumulate Additive Poses
		FAnimationPoseData OutputAnimationPoseData(Output);
		AccumulateAdditivePose(SourcePosesView, SourceCurvesView, SourceAttributesView, SourceWeightsView, SourceBlendModesView, OutputAnimationPoseData);

		// pop the poses we added
		SourcePoses.SetNum(SourcePosesInitialNum, false);
		SourceCurves.SetNum(SourcePosesInitialNum, false);
		SourceWeights.SetNum(SourcePosesInitialNum, false);
		SourceAttributes.SetNum(SourcePosesInitialNum, false);
		SourceBlendModes.SetNum(SourcePosesInitialNum, false);
	}
}

void FAnimNode_MDA::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)
	const int NumPoses = Poses.Num();
	
	FString DebugLine = DebugData.GetNodeName(this);
	DebugLine += FString::Printf(TEXT("(Num Poses: %i)"), NumPoses);
	DebugData.AddDebugItem(DebugLine);

	BasePose.GatherDebugData(DebugData.BranchFlow(1.f));
	
	for (int32 ChildIndex = 0; ChildIndex < NumPoses; ++ChildIndex)
	{
		Poses[ChildIndex].GatherDebugData(DebugData.BranchFlow(BlendWeights[ChildIndex]));
	}
	// TODO: More info for debugging
}

void FAnimNode_MDA::AccumulateAdditivePose(TArrayView<const FCompactPose> SourcePoses, TArrayView<const FBlendedCurve> SourceCurves, TArrayView<const UE::Anim::FStackAttributeContainer> SourceAttributes, TArrayView<const float> SourceWeights, TArrayView<const EMDABlendMode> SourceBlendModes, FAnimationPoseData& OutAnimationPoseData)
{
	check(SourcePoses.Num() > 0);

	// Get out anim data
	FCompactPose& OutPose = OutAnimationPoseData.GetPose();
	FBlendedCurve& OutCurve = OutAnimationPoseData.GetCurve();
	UE::Anim::FStackAttributeContainer& OutAttributes = OutAnimationPoseData.GetAttributes();

	for (int32 PoseIndex = 0; PoseIndex < SourcePoses.Num(); ++PoseIndex)
	{
		switch (SourceBlendModes[PoseIndex])
		{
			case EMDABlendMode::Add:
			{
				AccumulateAdditivePoseInternal<EMDABlendMode::Add>(OutPose, SourcePoses[PoseIndex], SourceWeights[PoseIndex]);
				break;
			}
			case EMDABlendMode::Subtract:
			{
				AccumulateAdditivePoseInternal<EMDABlendMode::Subtract>(OutPose, SourcePoses[PoseIndex], SourceWeights[PoseIndex]);
				break;
			}
			case EMDABlendMode::CoDAdd:
			{
				AccumulateAdditivePoseInternal<EMDABlendMode::CoDAdd>(OutPose, SourcePoses[PoseIndex], SourceWeights[PoseIndex]);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	// Ensure that all of the resulting rotations are normalized
	if (SourcePoses.Num() > 0)
	{
		OutPose.NormalizeRotations();
	}

	// If curve exists, blend with the weight
	// TODO: To be optimized
	if (SourceCurves.Num() > 0)
	{
		TArray<FBlendedCurve> FullCurvesArray(SourceCurves);
		FBlendedCurve Curve = OutCurve;
		FullCurvesArray.EmplaceAt(0, Curve);

		TArray<float> FullWeightsArray(SourceWeights);
		FullWeightsArray.EmplaceAt(0, 1);

		BlendCurves1(FullCurvesArray, FullWeightsArray, OutCurve, CurveBlendOption);
	}

	if (SourceAttributes.Num() > 0)
	{
		UE::Anim::Attributes::BlendAttributes(SourceAttributes, SourceWeights, OutAttributes);
	}
}

void FAnimNode_MDA::BlendCurves1(const TArrayView<const FBlendedCurve> SourceCurves, const TArrayView<const float> SourceWeights, FBlendedCurve& OutCurve, ECurveBlendOption::Type BlendOption)
{
	if (SourceCurves.IsEmpty())
		return;


	TArray<const FBlendedCurve*> SourceCurvesPtr;
	for (const FBlendedCurve& Curve : SourceCurves)
	{
		SourceCurvesPtr.Add(&Curve);
	}
	
	if (BlendOption == ECurveBlendOption::Type::BlendByWeight)
	{
		BlendCurves(SourceCurvesPtr, SourceWeights, OutCurve);
	}
	else if (BlendOption == ECurveBlendOption::Type::NormalizeByWeight)
	{
		float SumOfWeight = 0.f;
		for (const auto& Weight : SourceWeights)
		{
			SumOfWeight += Weight;
		}

		if (FAnimWeight::IsRelevant(SumOfWeight))
		{
			TArray<float> NormalizeSourceWeights;
			NormalizeSourceWeights.AddUninitialized(SourceWeights.Num());
			for(int32 Idx=0; Idx<SourceWeights.Num(); ++Idx)
			{
				NormalizeSourceWeights[Idx] = SourceWeights[Idx] / SumOfWeight;
			}

			BlendCurves(SourceCurvesPtr, NormalizeSourceWeights, OutCurve);
		}
		else
		{
			BlendCurves(SourceCurvesPtr, SourceWeights, OutCurve);
		}
	}
	else if (BlendOption == ECurveBlendOption::Type::UseMaxValue)
	{
		OutCurve.Override(SourceCurves[0], SourceWeights[0]);

		for (int32 CurveIndex = 1; CurveIndex < SourceCurves.Num(); ++CurveIndex)
		{
			OutCurve.UseMaxValue(SourceCurves[CurveIndex]);
		}
	}
	else if (BlendOption == ECurveBlendOption::Type::UseMinValue)
	{
		OutCurve.Override(SourceCurves[0], SourceWeights[0]);

		for (int32 CurveIndex = 1; CurveIndex < SourceCurves.Num(); ++CurveIndex)
		{
			OutCurve.UseMinValue(SourceCurves[CurveIndex]);
		}
	}
	else if (BlendOption == ECurveBlendOption::Type::UseBasePose)
	{
		OutCurve.Override(SourceCurves[0], SourceWeights[0]);
	}
	else if (BlendOption == ECurveBlendOption::Type::DoNotOverride)
	{
		OutCurve.Override(SourceCurves[0], SourceWeights[0]);

		for (int32 CurveIndex = 1; CurveIndex < SourceCurves.Num(); ++CurveIndex)
		{
			OutCurve.CombinePreserved(SourceCurves[CurveIndex]);
		}
	}
	else
	{
		OutCurve.Override(SourceCurves[0], SourceWeights[0]);

		for(int32 CurveIndex = 1; CurveIndex < SourceCurves.Num(); ++CurveIndex)
		{
			OutCurve.Combine(SourceCurves[CurveIndex]);
		}
	}
}
