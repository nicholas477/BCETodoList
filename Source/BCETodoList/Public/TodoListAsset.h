// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TodoListAsset.generated.h"

USTRUCT(BlueprintType)
struct FTodoTask
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Todo Task")
		FText TaskName = FText::FromString("Task Name");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Todo Task")
		FText TaskDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Todo Task")
		FName TaskStatus;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class BCETODOLIST_API UTodoListAsset : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsEditorOnly() const override { return true; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Todo List")
		FText ListName = FText::FromString("Todo List");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Todo List")
		TArray<FTodoTask> Tasks;
};
