// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Characters/Interfaces/Targetable.h"
#include "Dummy.generated.h"

UCLASS()
class MAXENCE_SANDBOX_API ADummy : public AActor, public ITargetable
{
	GENERATED_BODY()


public:	
	// Sets default values for this actor's properties
	ADummy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
