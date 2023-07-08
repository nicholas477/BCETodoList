// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TodoListSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Editor)
class BCETODOLIST_API UTodoListSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UTodoListSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Todo List Settings")
		TArray<FName> TaskStatusTypes;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Todo List Settings", meta=(GetOptions = "GetTaskStatusTypes"))
		FName TodoStatus;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Todo List Settings", meta=(GetOptions = "GetTaskStatusTypes"))
		FName InProgressStatus;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Todo List Settings", meta=(GetOptions = "GetTaskStatusTypes"))
		FName DoneStatus;

	UFUNCTION()
		static TArray<FName> GetTaskStatusTypes() { return Get()->TaskStatusTypes; }

	static UTodoListSettings* Get() { return GetMutableDefault<UTodoListSettings>(); }
};
