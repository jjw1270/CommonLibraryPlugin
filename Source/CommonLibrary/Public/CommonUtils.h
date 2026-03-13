#pragma once 
 
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonUtils.generated.h"

#pragma region Log
COMMONLIBRARY_API DECLARE_LOG_CATEGORY_EXTERN(MyLog, Display, All);

#define CALLED_FROM (TEXT(__FUNCTION__) + FString::Printf(TEXT("(%d)"), __LINE__))
#define CUSTOM_LOG(Verbosity, Format, ...) UE_LOG(MyLog, Verbosity, TEXT("%s %s"), *CALLED_FROM, *FString::Printf(Format, ##__VA_ARGS__))
#define CUSTOM_LOG_SCREEN(LifeTime, Color, Format, ...) GEngine->AddOnScreenDebugMessage(-1, LifeTime, Color, FString::Printf(Format, ##__VA_ARGS__))

#define TRACE() \
	UE_LOG(MyLog, Display, TEXT("%s"), *CALLED_FROM);

#define TRACE_LOG(Format, ...) \
	CUSTOM_LOG(Display, Format, ##__VA_ARGS__); \
	CUSTOM_LOG_SCREEN(4.0f, FColor::Green, Format, ##__VA_ARGS__);

#define TRACE_WARNING(Format, ...) \
	CUSTOM_LOG(Warning, Format, ##__VA_ARGS__); \
	CUSTOM_LOG_SCREEN(4.0f, FColor::Yellow, Format, ##__VA_ARGS__);

#define TRACE_ERROR(Format, ...) \
	CUSTOM_LOG(Error, Format, ##__VA_ARGS__); \
	ensure(false);

#pragma endregion

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
FORCEINLINE bool IsValidEnumValue(const UEnum* _enum, int64 _value, bool _include_hidden = false)
{
	if (_enum == nullptr)
		return false;

	const int32 num = _enum->NumEnums();

	for (int32 i = 0; i < num; ++i)
	{
		if (!_include_hidden && _enum->HasMetaData(TEXT("Hidden"), i))
			continue;

		if (_enum->GetValueByIndex(i) == _value)
			return true;
	}

	return false;
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
	static FORCEINLINE T* GetGameInstanceSubsystem(const UObject* _obj)
	{
		if (IsValid(_obj))
		{
			const auto world = _obj->GetWorld();
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
	static FORCEINLINE T* GetLocalPlayerSubsystem(const UObject* _obj)
	{
		if (IsValid(_obj))
		{
			const auto world = _obj->GetWorld();
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
	static FORCEINLINE T* GetLocalPlayerController(const UObject* _obj)
	{
		if (IsValid(_obj))
		{
			const auto world = _obj->GetWorld();
			if (IsValid(world))
			{
				return world->GetFirstPlayerController<T>();
			}
		}

		return nullptr;
	}
};
