// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DistanceBlendTypes.generated.h"

class UDistanceBlendComponent;

USTRUCT(BlueprintType)
struct FDistanceBlendWeight
{
	GENERATED_BODY()

	FDistanceBlendWeight(UDistanceBlendComponent* InComponent = nullptr)
		: Component(InComponent)
		, BlendWeight(0.f)
		, DistanceBias(1.f)
		, Scalar(1.f)
		, Dist(0.f)
	{}

	UPROPERTY(BlueprintReadOnly, Category = DistanceBlend)
	UDistanceBlendComponent* Component;

	/** Final computed result, the total of every FDistanceBlendWeight in the TArray is 1.0 */
	UPROPERTY(BlueprintReadOnly, Category = DistanceBlend)
	float BlendWeight;

	/**
	 * Higher bias means closer to the Target
	 * If Dist == AverageDistance to the Target, this is 1.0
	 */
	UPROPERTY(BlueprintReadOnly, Category = DistanceBlend)
	float DistanceBias;

	/** Runtime scaling to factor circumstances (eg. light intensity) */
	UPROPERTY(BlueprintReadOnly, Category = DistanceBlend)
	float Scalar;

	/** How far from the target */
	UPROPERTY(BlueprintReadOnly, Category = DistanceBlend)
	float Dist;
};