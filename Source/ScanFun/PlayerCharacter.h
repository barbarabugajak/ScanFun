// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

// GAS
#include "AbilitySystemInterface.h"
#include "CustomAbilitySystemComponent.h"
#include "PlayerBasicAttributeSet.h"
// EI
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter.generated.h"


UCLASS()
class SCANFUN_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

	// Enhanced Input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Input | Mappings")
	TSoftObjectPtr<UInputMappingContext> IMC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Movement")
	UInputAction* IA_Move;

	// Movement
	UFUNCTION(BlueprintCallable)
	void Move(const FInputActionValue& Value);

	// GAS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UCustomAbilitySystemComponent> ASC;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY()
	TObjectPtr<UPlayerBasicAttributeSet> BasicDataAttributeSet;

	// Gameplay Effects

	// === SCORE === 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS | Score | GameplayTags")
	FGameplayTag ScoreSetByCallerTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS | Score | GameplayEffects ")
	TSubclassOf<UGameplayEffect> AddScoreEffect_Class;

	UFUNCTION()
	void ApplyGameplayEffect_Score(float value);

	// Temporary for debug
	float i = 0;

	UFUNCTION()
	virtual void OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);
};
