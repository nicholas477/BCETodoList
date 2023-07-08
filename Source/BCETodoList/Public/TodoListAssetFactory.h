// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "TodoListAssetFactory.generated.h"

/**
 * 
 */
UCLASS()
class BCETODOLIST_API UTodoListAssetFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UTodoListAssetFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual bool ShouldShowInNewMenu() const override;
};
