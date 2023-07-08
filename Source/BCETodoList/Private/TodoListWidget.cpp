// Fill out your copyright notice in the Description page of Project Settings.


#include "TodoListWidget.h"

#include "TodoListAsset.h"
#include "PropertyCustomizationHelpers.h"
#include "TodoListSettings.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Commands/GenericCommands.h"
#include "BCETodoList.h"
#include "Algo/Reverse.h"

void STodoListWidget::Construct(const FArguments& InArgs)
{
	this->TodoList = TWeakObjectPtr<UTodoListAsset>(InArgs._TodoList);

	FCoreUObjectDelegates::OnObjectModified.AddSP(this, &STodoListWidget::OnObjectModified);
	FCoreUObjectDelegates::OnObjectTransacted.AddSP(this, &STodoListWidget::OnObjectTransacted);

	SUserWidget::Construct(SUserWidget::FArguments()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(TaskListView, STreeView<TSharedPtr<FTodoListWidgetItem>>)
			.TreeItemsSource(&TaskList)
			.OnGenerateRow(this, &STodoListWidget::OnGenerateTodoListRow)
			.OnGetChildren(this, &STodoListWidget::GetChildrenForInfo)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &STodoListWidget::AddTask))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Add Task"))
			]
		]
	]);
}

TSharedRef<ITableRow> STodoListWidget::OnGenerateTodoListRow(TSharedPtr<FTodoListWidgetItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	auto ContextMenuLambda = [this, Item](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{
		if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			TSharedRef<SWidget> MenuContents = GenerateTaskContextMenu(Item);
			FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
			FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuContents, MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

			return FReply::Handled();
		}

		return FReply::Unhandled();
	};

	typedef STableRow<TSharedPtr<FTodoListWidgetItem>> RowType;
	TSharedRef<RowType> NewRow = SNew(RowType, OwnerTable).Padding(FMargin(2.f, 2.f, 2.f, 2.f));
		//.Style(&FPaperStyle::Get()->GetWidgetStyle<FTableRowStyle>("TileMapEditor.LayerBrowser.TableViewRow"));

	//FIsSelected IsSelectedDelegate = FIsSelected::CreateSP(NewRow, &RowType::IsSelectedExclusively);
	//NewRow->SetContent(SNew(STileLayerItem, *Item, TileMapPtr.Get(), IsSelectedDelegate));

	const FTodoTask* Task = Item->GetTask();

	if (Task)
	{
		FSlateFontInfo TaskNameFont = FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle");
		TaskNameFont.Size = 16;

		FSlateFontInfo TaskDescriptionFont = IPropertyTypeCustomizationUtils::GetRegularFont();
		TaskDescriptionFont.Size = 14;

		TSharedRef<SVerticalBox> TaskBox =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(SCheckBox)
				.Type(ESlateCheckBoxType::CheckBox)
				.IsChecked_Lambda([Item]() -> ECheckBoxState
				{
					if (Item->GetTask())
					{
						UTodoListSettings* Settings = UTodoListSettings::Get();
						if (Item->GetTask()->TaskStatus == Settings->DoneStatus)
						{
							return ECheckBoxState::Checked;
						}
						if (Item->GetTask()->TaskStatus == Settings->TodoStatus)
						{
							return ECheckBoxState::Unchecked;
						}
						if (Item->GetTask()->TaskStatus == Settings->InProgressStatus)
						{
							return ECheckBoxState::Undetermined;
						}
					}
					return ECheckBoxState::Unchecked;
				})
				.ToolTipText_Lambda([Item]() -> FText
				{
					if (Item->GetTask())
					{
						return FText::FromString(Item->GetTask()->TaskStatus.ToString());
					}
					return FText::GetEmpty();
				})
				.OnCheckStateChanged_Lambda([Item](ECheckBoxState CheckboxState)
				{
					if (Item->GetTask() == nullptr)
						return;

					UTodoListSettings* Settings = UTodoListSettings::Get();

					const FScopedTransaction Transaction(INVTEXT("Change Task Status"));
					Item->ListAsset->Modify();

					FNotificationInfo NotificationInfo(INVTEXT(""));
					NotificationInfo.ExpireDuration = 3.0f;

					switch (CheckboxState)
					{
					case ECheckBoxState::Checked:
						Item->GetTask()->TaskStatus = Settings->DoneStatus;
						break;
					case ECheckBoxState::Unchecked:
						Item->GetTask()->TaskStatus = Settings->TodoStatus;
						break;
					case ECheckBoxState::Undetermined:
						Item->GetTask()->TaskStatus = Settings->InProgressStatus;
						break;
					}
					NotificationInfo.Text = FText::Format(INVTEXT("Set task \"{0}\" to \"{1}\""), Item->GetTask()->TaskName, FText::FromName(Item->GetTask()->TaskStatus));
					FSlateNotificationManager::Get().AddNotification(NotificationInfo);
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(INVTEXT("- "))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(SEditableText)
				.Text(Task->TaskName)
				.Font(TaskNameFont)
				.OnTextCommitted_Lambda([Item](const FText& InNewText, ETextCommit::Type InCommitType)
				{
					//if (InCommitType == ETextCommit::OnEnter)
					{
						if (Item->GetTask())
						{
							const FScopedTransaction Transaction(INVTEXT("Change Task Name"));
							Item->ListAsset->Modify();

							Item->GetTask()->TaskName = InNewText;
						}
					}
				})
			]
		];
		
		// Editable description slot
		TaskBox->AddSlot()
		.AutoHeight()
		[
			SNew(SMultiLineEditableText)
			.Text(Task->TaskDescription)
			.Font(TaskDescriptionFont)
			.AutoWrapText(true)
			.OnTextCommitted_Lambda([Item](const FText& InNewText, ETextCommit::Type InCommitType)
			{
				if (InCommitType == ETextCommit::OnUserMovedFocus)
				{
					if (Item->GetTask())
					{
						const FScopedTransaction Transaction(INVTEXT("Change Task Description"));
						Item->ListAsset->Modify();

						Item->GetTask()->TaskDescription = InNewText;
					}
				}
			})
		];

		const FButtonStyle& ButtonStyle = FAppStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");

		// Add a border
		NewRow->SetContent(SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(&ButtonStyle.Normal)
			]
			+ SOverlay::Slot()
			.Padding(FMargin(8.f, 2.f, 12.f, 2.f))
			[
				TaskBox
			]
		);
	}

	return NewRow;
}

void STodoListWidget::GetChildrenForInfo(TSharedPtr<FTodoListWidgetItem> InInfo, TArray<TSharedPtr<FTodoListWidgetItem>>& OutChildren)
{
}

void STodoListWidget::SetTodoList(UTodoListAsset* InTodoList)
{
	TodoList = InTodoList;
	RefreshTaskList();
}

UTodoListAsset* STodoListWidget::GetTodoList() const
{
	return TodoList.Get();
}

void STodoListWidget::AddTask()
{
	if (TodoList.IsValid())
	{
		const FScopedTransaction Transaction(INVTEXT("Add Task"));
		TodoList->Modify();

		TodoList->Tasks.AddDefaulted();
		RefreshTaskList();
	}
}

void STodoListWidget::RefreshTaskList()
{
	TaskList.Empty();

	if (TodoList.IsValid())
	{
		int32 i = 0;
		for (const FTodoTask& Task : TodoList->Tasks)
		{
			FTodoListWidgetItem NewItem;
			NewItem.ListAsset = TodoList;
			NewItem.ListItem = i;

			TaskList.Emplace(new FTodoListWidgetItem(MoveTemp(NewItem)));
			i++;
		}
	}

	TaskListView->RequestTreeRefresh();
}

TSharedRef<SWidget> STodoListWidget::GenerateTaskContextMenu(TSharedPtr<FTodoListWidgetItem> Item)
{
	TSharedPtr<FUICommandList> CommandList = MakeShared<FUICommandList>();

	CommandList->MapAction(FGenericCommands::Get().Delete, FExecuteAction::CreateLambda([this, Item]()
	{
		DeleteTask(Item);
	}));

	FMenuBuilder MenuBuilder(true, CommandList);
	MenuBuilder.BeginSection("TaskActions", INVTEXT("Task Actions"));

	//MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
	//MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
	//MenuBuilder.AddMenuEntry(FGenericCommands::Get().Paste);
	MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> STodoListWidget::GenerateTaskListContextMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.BeginSection("TaskListActions", INVTEXT("Task List Actions"));

	MenuBuilder.AddMenuEntry(
		INVTEXT("Create Task"), 
		INVTEXT(""),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]() {
			AddTask();
		}))
	);

	// Delete tasks
	TArray<TSharedPtr<FTodoListWidgetItem>> SelectedItems = TaskListView->GetSelectedItems();
	if (SelectedItems.Num() == 1)
	{
		MenuBuilder.AddMenuEntry(
			INVTEXT("Delete Task"),
			INVTEXT(""),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedItems]() {
				DeleteTask(SelectedItems[0]);
			}))
		);
	}
	else if (SelectedItems.Num() > 1)
	{
		MenuBuilder.AddMenuEntry(
			INVTEXT("Delete Tasks"),
			INVTEXT(""),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedItems]() {
				for (TSharedPtr<FTodoListWidgetItem> Item : SelectedItems)
				{
					DeleteTask(Item);
				}
			}))
		);
	}

	// Move tasks
	if (SelectedItems.Num() > 0 && FBCETodoListModule::Get().GetTodoLists().Num() > 1)
	{
		MenuBuilder.AddSubMenu(
			INVTEXT("Move Tasks"),
			INVTEXT("Move tasks to another task list"),
			FNewMenuDelegate::CreateLambda([this, SelectedItems](FMenuBuilder& SubmenuBuilder)
			{
				for (UTodoListAsset* ModuleTodoList : FBCETodoListModule::Get().GetTodoLists())
				{
					if (ModuleTodoList == nullptr || ModuleTodoList == this->GetTodoList())
						continue;

					SubmenuBuilder.AddMenuEntry(
						ModuleTodoList->ListName,
						INVTEXT(""),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateLambda([this, SelectedItems, ModuleTodoList]() {
							TArray<TSharedPtr<FTodoListWidgetItem>> SelectedItemsCpy = SelectedItems;
							SelectedItemsCpy.Sort([](const TSharedPtr<FTodoListWidgetItem>& Lhs, const TSharedPtr<FTodoListWidgetItem>& Rhs) -> bool
							{
								return Lhs->ListItem > Rhs->ListItem;
							});

							for (TSharedPtr<FTodoListWidgetItem> Item : SelectedItemsCpy)
							{
								MoveTask(Item, ModuleTodoList);
							}
						}))
					);
				}
			}),
			false,
			FSlateIcon());
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

FReply STodoListWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		TSharedRef<SWidget> MenuContents = GenerateTaskListContextMenu();
		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuContents, MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

		return FReply::Handled();
	}

	return SUserWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply STodoListWidget::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		AddTask();
		return FReply::Handled();
	}
	return SUserWidget::OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
}

void STodoListWidget::OnObjectModified(UObject* Object)
{
	if (TodoList.IsValid() && Object == TodoList)
	{
		RefreshTaskList();
	}
}

void STodoListWidget::DeleteTask(TSharedPtr<FTodoListWidgetItem> Item)
{
	if (Item->GetTask())
	{
		const FScopedTransaction Transaction(INVTEXT("Remove Task"));
		Item->ListAsset->Modify();

		Item->ListAsset->Tasks.RemoveAt(Item->ListItem);
		RefreshTaskList();
	}
}

void STodoListWidget::MoveTask(TSharedPtr<FTodoListWidgetItem> Item, UTodoListAsset* OtherList)
{
	if (OtherList == nullptr)
		return;

	if (Item->GetTask())
	{
		const FScopedTransaction Transaction(INVTEXT("Move Task"));
		Item->ListAsset->Modify();
		FTodoTask Task = *Item->GetTask();
		Item->ListAsset->Tasks.RemoveAt(Item->ListItem);

		OtherList->Modify();
		OtherList->Tasks.Add(Task);

		RefreshTaskList();
	}
}

TSharedRef<STodoListWidget> STodoListWidget::New()
{
	return MakeShareable(new STodoListWidget());
}

FTodoTask* FTodoListWidgetItem::GetTask()
{
	if (ListAsset.IsValid())
	{
		if (ListAsset->Tasks.IsValidIndex(ListItem))
		{
			return &ListAsset->Tasks[ListItem];
		}
	}
	return nullptr;
}

const FTodoTask* FTodoListWidgetItem::GetTask() const
{
	if (ListAsset.IsValid())
	{
		if (ListAsset->Tasks.IsValidIndex(ListItem))
		{
			return &ListAsset->Tasks[ListItem];
		}
	}
	return nullptr;
}
