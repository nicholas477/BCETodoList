// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SUserWidget.h"

class UTodoListAsset;

struct FTodoListWidgetItem : public TSharedFromThis<FTodoListWidgetItem>
{
	TWeakObjectPtr<UTodoListAsset> ListAsset;
	int32 ListItem;

	struct FTodoTask* GetTask();
	const struct FTodoTask* GetTask() const;
};

class BCETODOLIST_API STodoListWidget : public SUserWidget
{
public:
	SLATE_USER_ARGS(STodoListWidget)
		: _TodoList(nullptr)
	{}
		SLATE_ARGUMENT(UTodoListAsset*, TodoList)

	SLATE_END_ARGS()

	virtual void Construct(const FArguments& InArgs);
	
public:
	void SetTodoList(UTodoListAsset* TodoList);
	UTodoListAsset* GetTodoList() const;

	void RefreshTaskList();
	TSharedRef<SWidget> GenerateTaskContextMenu(TSharedPtr<FTodoListWidgetItem> Item);
	TSharedRef<SWidget> GenerateTaskListContextMenu();

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

protected:
	TWeakObjectPtr<UTodoListAsset> TodoList;

	TSharedPtr<STreeView<TSharedPtr<FTodoListWidgetItem>>> TaskListView;
	TArray<TSharedPtr<FTodoListWidgetItem>> TaskList;
	TSharedRef<ITableRow> OnGenerateTodoListRow(TSharedPtr<FTodoListWidgetItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void GetChildrenForInfo(TSharedPtr<FTodoListWidgetItem> InInfo, TArray<TSharedPtr<FTodoListWidgetItem>>& OutChildren);

	void AddTask();
	void DeleteTask(TSharedPtr<FTodoListWidgetItem> Item);
	void MoveTask(TSharedPtr<FTodoListWidgetItem> Item, UTodoListAsset* OtherList);

	void OnObjectModified(UObject* Object);
	void OnObjectTransacted(UObject* InObject, const FTransactionObjectEvent& InTransactionEvent) { OnObjectModified(InObject); }
};
