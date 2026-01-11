// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ScannerData.h"
// GAS
#include "AbilitySystemInterface.h"
#include "CustomAbilitySystemComponent.h"
#include "PlayerBasicAttributeSet.h"

#include "GameplayAbilityBase.h"

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

	// Scanning
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* ScannerConeComp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	UMaterialInstanceDynamic* DynMaterial;

	UFUNCTION(BlueprintCallable)
	void SetupScannerBeamParams(FScannerType ScannerType);

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FScannerType CurrentScanerType;

	// Enhanced Input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// GAS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UCustomAbilitySystemComponent> ASC;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UPlayerBasicAttributeSet> BasicDataAttributeSet;

	// Gameplay Abilities
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS | GameplayAbilities ")
	TArray<TSubclassOf<UGameplayAbilityBase>> GrantedAbilities;

	UFUNCTION()
	virtual void OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);

	// AttributeSet Value Chaned Event for desinger in Blueprints
	UFUNCTION(BlueprintImplementableEvent)
	void AttributeSet_ScoreChaned(float OldValue, float NewValue);

	UFUNCTION(BlueprintImplementableEvent)
	void ScanAbility_Activated(bool didAnythingGetScanned);

	UFUNCTION(BlueprintImplementableEvent)
	void ScanAbility_ActivationFailed();
};
