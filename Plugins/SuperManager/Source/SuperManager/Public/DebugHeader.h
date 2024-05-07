#pragma once

#include "Misc/MessageDialog.h" // add dialogue message
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"


void Print (const FString& Message, const FColor& Color = FColor::White)
{
	if(!GEngine) {return;}
	
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, Color, Message);
}


void PrintLog (const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}


EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgAsWarning = true)
{

	if(bShowMsgAsWarning)
	{
		const FText MsgTitle = FText::FromString(TEXT("Warning"));

		return FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please enter a VALID number")), &MsgTitle);
	}
	else
	{
		return FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please enter a VALID number")));

	}
}

void ShowNotifyInfo(const FString& Message)
{
	FNotificationInfo NotifyInfo(FText::FromString(Message));
	NotifyInfo.bUseLargeFont = true;
	NotifyInfo.FadeOutDuration = 7.f;

	FSlateNotificationManager::Get().AddNotification(NotifyInfo);
}