// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

// GAS
#include "AbilitySystemInterface.h"
#include "CustomAbilitySystemComponent.h"
#include "PlayerBasicAttributeSet.h"
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

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// GAS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UCustomAbilitySystemComponent> ASC;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPlayerBasicAttributeSet> BasicDataAttributeSet;

	// Gameplay Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS | GameplayEffects ")
	TSubclassOf<UGameplayEffect> UGE_Score_Class;

	UFUNCTION()
	void ApplyGameplayEffect_Score(float value);

	// Temporary for debug
	float i = 0;

};
