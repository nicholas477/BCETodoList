// Fill out your copyright notice in the Description page of Project Settings.


#include "TodoListAssetFactory.h"

#include "TodoListAsset.h"

UTodoListAssetFactory::UTodoListAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UTodoListAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}

UObject* UTodoListAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UTodoListAsset>(InParent, InClass, InName, Flags);
}

bool UTodoListAssetFactory::ShouldShowInNewMenu() const
{
	return true;
}
