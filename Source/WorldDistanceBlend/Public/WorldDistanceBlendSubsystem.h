// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DistanceBlendComponent.h"
#include "DistanceBlendTypes.h"
#include "WorldDistanceBlendSubsystem.generated.h"

/**
 * Base subsystem for tracking and testing against DistanceBlendComponents
 */
UCLASS(Abstract)
class WORLDDISTANCEBLEND_API UWorldDistanceBlendSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	// Use this in derived classes for convenient C++ access
	/** static UWorldDistanceBlendSubsystem* Get(const UWorld* const InWorld) { return InWorld ? InWorld->GetSubsystem<UWorldDistanceBlendSubsystem>() : nullptr; } */

protected:
	UPROPERTY(BlueprintReadOnly, Category = DistanceBlend)
	TArray<UDistanceBlendComponent*> BlendComponents;

	UPROPERTY()
	uint64 LastUpdateFrame = -1;

	bool ShouldUpdateDistance() const
	{
		return GFrameCounter != LastUpdateFrame;
	}

	/**
	 * The actor that the distance calculations are based on
	 * Passing a PlayerCameraManager will use the camera location instead
	 */
	TWeakObjectPtr<AActor> BlendTarget;
	
	UPROPERTY()
	TArray<FDistanceBlendWeight> BlendWeights;

	/**
	 * Last valid blend weights before BlendWeights were cleared
	 * Still invalid if GetBlendWeights() has never been called
	 */
	UPROPERTY()
	TArray<FDistanceBlendWeight> LastValidBlendWeights;
	
public:
	/**
	 * Assign the actor that the distance calculations are based on
	 * Passing a PlayerCameraManager will use the camera location instead
	 */
	UFUNCTION(BlueprintCallable, Category = DistanceBlend)
	void AssignBlendTarget(AActor* NewBlendTarget)
	{
		if (NewBlendTarget != BlendTarget)
		{
			BlendWeights.Empty();
			LastUpdateFrame = -1;
		}
		BlendTarget = NewBlendTarget;
	}
	
	/** Register a DistanceBlendComponent */
	UFUNCTION(BlueprintCallable, Category = DistanceBlend)
	void RegisterBlendComponent(UDistanceBlendComponent* BlendComponent)
	{
		BlendComponents.AddUnique(BlendComponent);
	}

	/** Deregister a DistanceBlendComponent */
	UFUNCTION(BlueprintCallable, Category = DistanceBlend)
	void UnregisterBlendComponent(UDistanceBlendComponent* BlendComponent)
	{
		BlendComponents.Remove(BlendComponent);
	}

	/**
	 * Get the last valid blend weights before BlendWeights were cleared
	 * Still invalid if GetBlendWeights() has never been called (check against bValid)
	 * This will not attempt to update; should only ever be used when GetBlendWeights() bValid is false
	 */
	UFUNCTION(BlueprintCallable, Category = DistanceBlend)
	const TArray<FDistanceBlendWeight>& GetLastValidBlendWeights(bool& bValid) const
	{
		bValid = LastValidBlendWeights.Num() > 0;
		return LastValidBlendWeights;
	}

	/**
	 * @param WorldLocation Location to get the distance to each DistanceBlendComponent
	 * @param bValid True if the blend weights have valid data
	 * @param bDistanceXY If true only get the distance in 2D Space (ignoring Z axis)
	 * @return Blend Weights if already updated this frame, otherwise will update then return
	 */
	UFUNCTION(BlueprintCallable, Category = DistanceBlend)
	const TArray<FDistanceBlendWeight>& GetBlendWeights(bool& bValid, bool bDistanceXY = true) const
	{
		bValid = false;

		if (!BlendTarget.IsValid())
		{
			return BlendWeights;
		}
		
		// Don't compute new blend weights if already updated this frame
		if (ShouldUpdateDistance())
		{
			// Compute new blend weights
			UWorldDistanceBlendSubsystem* MutableThis = const_cast<UWorldDistanceBlendSubsystem*>(this);
			MutableThis->LastUpdateFrame = GFrameCounter;

			TArray<FDistanceBlendWeight>& Weights = MutableThis->BlendWeights;
			Weights.Empty();

			float TotalDistances = 0.f;

			// Cache all relevant information about each component
			for (UDistanceBlendComponent* Comp : BlendComponents)
			{
				// Note: Not null checking, components expected to call RegisterBlendComponent and DeregisterBlendComponent
				// If you crashed here, this is why
				checkSlow(Comp != nullptr && IsValid(Comp->GetOwner()));

				FDistanceBlendWeight BlendWeight { Comp };
				
				const AActor* Owner = Comp->GetOwner();
				BlendWeight.Scalar = Comp->GetBlendScalar();

				FVector TargetLocation = BlendTarget.Get()->GetActorLocation();
				if (const APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(BlendTarget.Get()))
				{
					TargetLocation = CameraManager->GetCameraLocation();
				}

				const FVector Diff = (TargetLocation - Owner->GetActorLocation());
				BlendWeight.Dist = bDistanceXY ? Diff.Size2D() : Diff.Size();

				TotalDistances += BlendWeight.Dist;
				
				Weights.Add(BlendWeight);
			}

			if (Weights.Num() == 0)
			{
				return BlendWeights;
			}

			// Compute biases from gathered information
			const float AverageDistances = TotalDistances / Weights.Num();
			for (FDistanceBlendWeight& W : Weights)
			{
				// Set the BlendWeight based on relativity to average distance and runtime scaling
				// This is the precursor BlendWeight prior to averaging based on BlendWeights achieved by
				// remaining entries within this loop
				W.DistanceBias = AverageDistances / W.Dist;
				W.BlendWeight = W.DistanceBias * W.Scalar;
			}

			// Scale the bias relative to each entry to achieve the final result
			{
				float Lowest = INFINITY;
				float Sum = 0.f;

				for (const FDistanceBlendWeight& W : Weights)
				{
					// Find lowest weight
					if (W.BlendWeight < Lowest)
					{
						Lowest = W.BlendWeight;
					}
				}

				// Divide by lowest and compute sum
				for (FDistanceBlendWeight& W : Weights)
				{
					W.BlendWeight /= Lowest;
					Sum += W.BlendWeight;
				}

				// Scale array to become 1.0
				for (FDistanceBlendWeight& W : Weights)
				{
					W.BlendWeight /= Sum;
					W.Component->BlendWeight = W;
				}
			}

			if (BlendWeights.Num() > 0)
			{
				MutableThis->LastValidBlendWeights = BlendWeights;
			}
		}

		bValid = BlendWeights.Num() > 0;
		return BlendWeights;
	}
};
