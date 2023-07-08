// Fill out your copyright notice in the Description page of Project Settings.


#include "TodoListAssetTypeActions.h"

#include "TodoListAsset.h"

UClass* FTodoListAssetTypeActions::GetSupportedClass() const
{
    return UTodoListAsset::StaticClass();
}

FText FTodoListAssetTypeActions::GetName() const
{
    return INVTEXT("Todo List");
}

FColor FTodoListAssetTypeActions::GetTypeColor() const
{
    return FColor::Cyan;
}

uint32 FTodoListAssetTypeActions::GetCategories()
{
    return EAssetTypeCategories::Misc;
}