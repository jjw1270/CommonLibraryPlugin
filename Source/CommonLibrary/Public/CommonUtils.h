#pragma once 

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonUtils.generated.h"

#pragma region Log
COMMONLIBRARY_API DECLARE_LOG_CATEGORY_EXTERN(MyLog, Display, All);

#define CALLED_FROM (TEXT(__FUNCTION__) + FString::Printf(TEXT("(%d)"), __LINE__))
#define CUSTOM_LOG(Verbosity, Format, ...) UE_LOG(MyLog, Verbosity, TEXT("%s %s"), *CALLED_FROM, *FString::Printf(Format, ##__VA_ARGS__))
#define CUSTOM_LOG_SCREEN(LifeTime, Color, Format, ...) do { if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, LifeTime, Color, FString::Printf(Format, ##__VA_ARGS__)); } } while (false)

#define TRACE() \
	do { \
		UE_LOG(MyLog, Display, TEXT("%s"), *CALLED_FROM); \
	} while (false)

#define TRACE_LOG(Format, ...) \
	do { \
		CUSTOM_LOG(Display, Format, ##__VA_ARGS__); \
		CUSTOM_LOG_SCREEN(4.0f, FColor::Green, Format, ##__VA_ARGS__); \
	} while (false)

#define TRACE_WARNING(Format, ...) \
	do { \
		CUSTOM_LOG(Warning, Format, ##__VA_ARGS__); \
		CUSTOM_LOG_SCREEN(4.0f, FColor::Yellow, Format, ##__VA_ARGS__); \
	} while (false)

#if WITH_EDITOR && UE_BUILD_DEBUG
#define TRACE_ERROR(Format, ...) \
	do { \
		CUSTOM_LOG(Error, Format, ##__VA_ARGS__); \
		check(false); \
	} while (false)
#else
#define TRACE_ERROR(Format, ...) \
	do { \
		CUSTOM_LOG(Error, Format, ##__VA_ARGS__); \
	} while (false)
#endif

#pragma endregion

#pragma region Editor Log
namespace EditorLog
{
	enum class EEditorLogVerbosity
	{
		Display,
		Warning,
		Error
	};

	COMMONLIBRARY_API void EditorMessage(EEditorLogVerbosity _verbosity, const FName& _log_name, const FString& _message);
	COMMONLIBRARY_API void EditorMessage(EEditorLogVerbosity _verbosity, const FName& _log_name, const FString& _message, const UObject* _target_object);
	COMMONLIBRARY_API void EditorClearMessage(const FName& _log_name);

	COMMONLIBRARY_API void EditorPopup(const FString& _message);

	COMMONLIBRARY_API void EditorNotify(EEditorLogVerbosity _verbosity, const FString& _message);
}

#define EDITOR_MESSAGE_LOG(_log_name, _format, ...) \
	EditorLog::EditorMessage(EditorLog::EEditorLogVerbosity::Display, _log_name, FString::Printf(_format, ##__VA_ARGS__))

#define EDITOR_MESSAGE_WARNING(_log_name, _format, ...) \
	EditorLog::EditorMessage(EditorLog::EEditorLogVerbosity::Warning, _log_name, FString::Printf(_format, ##__VA_ARGS__))

#define EDITOR_MESSAGE_ERROR(_log_name, _format, ...) \
	EditorLog::EditorMessage(EditorLog::EEditorLogVerbosity::Error, _log_name, FString::Printf(_format, ##__VA_ARGS__))

#define EDITOR_MESSAGE_ERROR_OBJECT(_log_name, _object, _format, ...) \
	EditorLog::EditorMessage(EditorLog::EEditorLogVerbosity::Error, _log_name, FString::Printf(_format, ##__VA_ARGS__), _object)

#define EDITOR_MESSAGE_CLEAR(_log_name) \
	EditorLog::EditorClearMessage(_log_name)

#define EDITOR_POPUP( _format, ...) \
	EditorLog::EditorPopup(FString::Printf(_format, ##__VA_ARGS__))

#define EDITOR_NOTIFY_LOG(_format, ...) \
	EditorLog::EditorNotify(EditorLog::EEditorLogVerbosity::Display,  FString::Printf(_format, ##__VA_ARGS__))

#define EDITOR_NOTIFY_WARNING(_format, ...) \
	EditorLog::EditorNotify(EditorLog::EEditorLogVerbosity::Warning,  FString::Printf(_format, ##__VA_ARGS__))

#define EDITOR_NOTIFY_ERROR(_format, ...) \
	EditorLog::EditorNotify(EditorLog::EEditorLogVerbosity::Error,  FString::Printf(_format, ##__VA_ARGS__))

#pragma endregion Editor Log

FORCEINLINE constexpr bool IsAny()
{
	return false;
}

template <typename... Args>
FORCEINLINE constexpr bool IsAny(Args&&... _rest)
{
	return (... || static_cast<bool>(_rest));
}

FORCEINLINE constexpr bool IsAll()
{
	return true;
}

template <typename... Args>
FORCEINLINE constexpr bool IsAll(Args&&... _rest)
{
	return (... && static_cast<bool>(_rest));
}

#pragma region ValidCheck

template <typename T>
FORCEINLINE bool IsValid(const TWeakObjectPtr<T>& _weak_obj_ptr)
{
	return _weak_obj_ptr.IsValid();
}

template <typename T>
FORCEINLINE typename TEnableIf<!TIsDerivedFrom<T, UObject>::Value, bool>::Type
IsValid(const T* _ptr)
{
	return _ptr != nullptr;
}

template <typename T>
FORCEINLINE bool IsValid(const TSharedPtr<T>& _shared_ptr)
{
	return _shared_ptr.IsValid();
}

FORCEINLINE bool IsAllValid()
{
	return true;
}

template <typename... Args>
FORCEINLINE bool IsAllValid(Args&&... _args)
{
	return (... && IsValid(Forward<Args>(_args)));
}

template <typename T>
FORCEINLINE bool IsInvalid(T&& _arg)
{
	return !IsValid(Forward<T>(_arg));
}

FORCEINLINE bool IsAnyInvalid()
{
	return false;
}

template <typename... Args>
FORCEINLINE bool IsAnyInvalid(Args&&... _args)
{
	return (... || !IsValid(Forward<Args>(_args)));
}

#pragma endregion
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Enum
FORCEINLINE bool IsValidEnumValue(const UEnum* _enum, int64 _value)
{
	if (_enum == nullptr)
		return false;

	return _enum->GetIndexByValue(_value) != INDEX_NONE;
}

template<typename TEnum>
FORCEINLINE bool IsValidEnumValue(TEnum _enum_value)
{
	static_assert(TIsEnum<TEnum>::Value, "TEnum must be an enum type.");

	const UEnum* enum_ptr = StaticEnum<TEnum>();
	if (enum_ptr == nullptr)
		return false;

	return IsValidEnumValue(enum_ptr, static_cast<int64>(_enum_value));
}

template <typename TEnum>
FORCEINLINE FString TEnumToString(TEnum _enum_value)
{
	static_assert(TIsEnum<TEnum>::Value, "TEnumToString requires an enum type.");

	const UEnum* enum_ptr = StaticEnum<TEnum>();
	if (enum_ptr == nullptr)
		return TEXT("InvalidEnumType");

	const int64 value = static_cast<int64>(_enum_value);

	if (!IsValidEnumValue(enum_ptr, value))
		return TEXT("InvalidEnumValue");

	return enum_ptr->GetNameStringByValue(value);
}
#pragma endregion Enum
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept CONCEPT_GameInstanceSubsystem = TIsDerivedFrom<T, UGameInstanceSubsystem>::Value;

template<typename T>
concept CONCEPT_LocalPlayerSubsystem = TIsDerivedFrom<T, ULocalPlayerSubsystem>::Value;

template<typename T>
concept CONCEPT_PlayerController = TIsDerivedFrom<T, APlayerController>::Value;

UCLASS()
class COMMONLIBRARY_API UCommonUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template<CONCEPT_GameInstanceSubsystem T>
	static FORCEINLINE T* GetGameInstanceSubsystem(const UObject* _world_ctx)
	{
		if (IsValid(_world_ctx))
		{
			const auto world = _world_ctx->GetWorld();
			if (IsValid(world))
			{
				auto game_inst = world->GetGameInstance();
				if (IsValid(game_inst))
				{
					return game_inst->GetSubsystem<T>();
				}
			}
		}

		return nullptr;
	}

	template<CONCEPT_LocalPlayerSubsystem T>
	static FORCEINLINE T* GetLocalPlayerSubsystem(const UObject* _world_ctx)
	{
		if (IsValid(_world_ctx))
		{
			const auto world = _world_ctx->GetWorld();
			if (IsValid(world))
			{
				auto local_player = world->GetFirstLocalPlayerFromController();
				if (IsValid(local_player))
				{
					return local_player->GetSubsystem<T>();
				}
			}
		}

		return nullptr;
	}

	template<CONCEPT_PlayerController T = APlayerController>
	static FORCEINLINE T* GetLocalPlayerController(const UObject* _world_ctx)
	{
		if (IsValid(_world_ctx))
		{
			const auto world = _world_ctx->GetWorld();
			if (IsValid(world))
			{
				return world->GetFirstPlayerController<T>();
			}
		}

		return nullptr;
	}

	UFUNCTION(BlueprintPure, meta = (WorldContext = "_world_ctx"))
	static APlayerController* GetLocalPlayerController(const UObject* _world_ctx)
	{
		return GetLocalPlayerController<APlayerController>(_world_ctx);
	}
};
