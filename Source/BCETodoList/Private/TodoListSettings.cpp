// Fill out your copyright notice in the Description page of Project Settings.


#include "TodoListSettings.h"

UTodoListSettings::UTodoListSettings(const FObjectInitializer& ObjectInitializer)
{
	TaskStatusTypes = {
		FName("Todo"),
		FName("In Progress"),
		FName("Done")
	};

	TodoStatus = FName("Todo");
	InProgressStatus = FName("In Progress");
	DoneStatus = FName("Done");
}
