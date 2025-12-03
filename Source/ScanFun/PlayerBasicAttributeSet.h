// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

// This'll be standardized in UE 5.6. Refactor once available
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "CustomAbilitySystemComponent.h"
#include "PlayerBasicAttributeSet.generated.h"


UCLASS()
class SCANFUN_API UPlayerBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	public:
		
		UPlayerBasicAttributeSet();

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FGameplayAttributeData Score;

		ATTRIBUTE_ACCESSORS(UPlayerBasicAttributeSet, Score);
	
	
};
