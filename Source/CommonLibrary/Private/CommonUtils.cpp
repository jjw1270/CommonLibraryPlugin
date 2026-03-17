#include "CommonUtils.h"

#include "MessageLogModule.h"
#include "Logging/MessageLog.h"
#include "IMessageLogListing.h"
#include "Misc/UObjectToken.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

DEFINE_LOG_CATEGORY(MyLog)


void EditorLog::EditorMessage(EEditorLogVerbosity _verbosity, const FName& _log_name, const FString& _message)
{
#if WITH_EDITOR
	FMessageLog(_log_name).Open();

	switch (_verbosity)
	{
	case EditorLog::EEditorLogVerbosity::Display:
		FMessageLog(_log_name).Info(FText::FromString(_message));
		break;
	case EditorLog::EEditorLogVerbosity::Warning:
		FMessageLog(_log_name).Warning(FText::FromString(_message));
		break;
	case EditorLog::EEditorLogVerbosity::Error:
		FMessageLog(_log_name).Error(FText::FromString(_message));
		break;
	default:
		break;
	}
#endif
}

void EditorLog::EditorClearMessage(const FName& _log_name)
{
#if WITH_EDITOR
	FMessageLogModule& message_log_module = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	message_log_module.GetLogListing(_log_name)->ClearMessages();
#endif
}

void EditorLog::EditorNotify(EEditorLogVerbosity _verbosity, const FString& _message)
{
#if WITH_EDITOR
	FNotificationInfo info(FText::FromString(_message));
	info.ExpireDuration = 5.0f;
	info.bUseLargeFont = false;
	info.bFireAndForget = true;

	SNotificationItem::ECompletionState state = SNotificationItem::CS_None;

	switch (_verbosity)
	{
	case EditorLog::EEditorLogVerbosity::Display:
		state = SNotificationItem::CS_Success;
		break;
	case EditorLog::EEditorLogVerbosity::Warning:
		state = SNotificationItem::CS_Fail;
		break;
	case EditorLog::EEditorLogVerbosity::Error:
		info.bUseLargeFont = true;
		state = SNotificationItem::CS_Fail;
		break;
	default:
		break;
	}

	TSharedPtr<SNotificationItem> notification = FSlateNotificationManager::Get().AddNotification(info);
	if (IsValid(notification))
	{
		notification->SetCompletionState(state);
	}
#endif
}
