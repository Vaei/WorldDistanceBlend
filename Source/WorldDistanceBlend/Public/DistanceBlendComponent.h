// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DistanceBlendTypes.h"
#include "Components/ActorComponent.h"
#include "DistanceBlendComponent.generated.h"


UCLASS(Abstract, ClassGroup=(Custom))
class WORLDDISTANCEBLEND_API UDistanceBlendComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = DistanceBlend)
	FDistanceBlendWeight BlendWeight;
	
public:
	/** Override to change how much blend weight this component has */
	UFUNCTION(BlueprintNativeEvent, Category = DistanceBlend)
	float GetBlendScalar() const;

protected:
	virtual void OnRegister() override
	{
		// Call UActorComponent::OnRegister instead of Super::OnRegister on override
		ensureMsgf(false, TEXT("UDistanceBlendComponent::OnRegister not implemented"));
	}
	virtual void OnUnregister() override
	{
		// Call UActorComponent::OnUnregister instead of Super::OnUnregister on override
		ensureMsgf(false, TEXT("UDistanceBlendComponent::OnUnregister not implemented"));
	}
};
