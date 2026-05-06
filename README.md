# Common Library Plugin

`CommonLibrary`는 Unreal Engine 5 프로젝트의 여러 플러그인에서 반복 사용하는 **로그, 에디터 알림, 유효성 검사, Enum 문자열화, Subsystem/PlayerController 접근 헬퍼**를 모아 둔 런타임 공용 라이브러리 플러그인이다.

이 문서는 `Plugins/CommonLibraryPlugin`의 **현재 코드 기준**으로 유지되는 기술 README다. 실제 사용 절차 중심 문서는 [UserGuide.md](./UserGuide.md)를 참고한다.

---

## 핵심 요약

- `CommonLibrary` 런타임 모듈 하나로 구성된 코드 전용 플러그인이다.
- 핵심 공개 헤더는 `CommonUtils.h`다.
- 전역 로그 카테고리 `MyLog`와 `TRACE_*`, `CUSTOM_LOG*` 매크로를 제공한다.
- 에디터 전용 Message Log, Popup, Notification 헬퍼를 제공한다.
- `IsAllValid`, `IsAnyInvalid` 같은 공통 유효성 검사 헬퍼를 제공한다.
- `TEnumToString`, `IsValidEnumValue`로 UEnum 기반 Enum 값을 안전하게 다룬다.
- `UCommonUtils`는 Subsystem과 Local PlayerController 조회 헬퍼를 제공한다.

---

## 플러그인 정보

| 항목 | 값 |
| --- | --- |
| 경로 | `Plugins/CommonLibraryPlugin` |
| 플러그인 이름 | `CommonLibrary` |
| 모듈 | `CommonLibrary` |
| 모듈 타입 | Runtime |
| LoadingPhase | Default |
| Content 포함 | 없음 (`CanContainContent: false`) |
| Engine version | 5.7 |

---

## 모듈 구성

| 파일 | 역할 |
| --- | --- |
| `CommonLibrary.uplugin` | 플러그인/모듈 정의 |
| `Source/CommonLibrary/CommonLibrary.Build.cs` | 모듈 의존성 정의 |
| `Source/CommonLibrary/Public/CommonLibrary.h` | 모듈 인터페이스 선언, `CommonUtils.h` 포함 |
| `Source/CommonLibrary/Private/CommonLibrary.cpp` | `FCommonLibraryModule` 구현 |
| `Source/CommonLibrary/Public/CommonUtils.h` | 공용 로그/에디터 로그/검사/Enum/Subsystem 헬퍼 선언 |
| `Source/CommonLibrary/Private/CommonUtils.cpp` | 로그 카테고리 및 에디터 헬퍼 구현 |

### Build.cs 의존성

항상 사용하는 Public 의존성:

- `Core`
- `CoreUObject`
- `Engine`

에디터 빌드에서만 사용하는 Private 의존성:

- `MessageLog`
- `Slate`
- `SlateCore`

에디터 기능 구현부는 `#if WITH_EDITOR`로 감싸져 있어 패키지/런타임 빌드에서는 Message Log, Notification, Dialog 코드가 실행되지 않는다.

---

## 주요 공개 기능

### 로그 매크로

파일: `Source/CommonLibrary/Public/CommonUtils.h`

로그 카테고리:

```cpp
DECLARE_LOG_CATEGORY_EXTERN(MyLog, Display, All);
```

구현 파일에서 다음처럼 정의된다.

```cpp
DEFINE_LOG_CATEGORY(MyLog)
```

| 매크로 | 의미 |
| --- | --- |
| `CALLED_FROM` | 현재 함수명과 라인 번호 문자열 |
| `CUSTOM_LOG(Verbosity, Format, ...)` | `MyLog` 카테고리로 함수/라인 정보와 함께 로그 출력 |
| `CUSTOM_LOG_SCREEN(LifeTime, Color, Format, ...)` | `GEngine->AddOnScreenDebugMessage`로 화면 로그 출력 |
| `TRACE()` | 현재 함수/라인만 `Display` 로그로 출력 |
| `TRACE_LOG(Format, ...)` | `Display` 로그 + 초록색 화면 로그 |
| `TRACE_WARNING(Format, ...)` | `Warning` 로그 + 노란색 화면 로그 |
| `TRACE_ERROR(Format, ...)` | `Error` 로그. 에디터 Debug 빌드에서는 `check(false)` 포함 |

`TRACE_ERROR` 정책:

- `WITH_EDITOR && UE_BUILD_DEBUG`일 때는 로그 출력 후 `check(false)`로 즉시 실패시킨다.
- 그 외 빌드에서는 에러 로그만 출력한다.

사용 예:

```cpp
#include "CommonUtils.h"

void UMyObject::DoSomething()
{
	TRACE();
	TRACE_LOG(TEXT("Start task: %s"), *GetName());
	TRACE_WARNING(TEXT("Optional asset is missing."));
}
```

---

### 에디터 Message Log / Popup / Notification

파일:

- `Source/CommonLibrary/Public/CommonUtils.h`
- `Source/CommonLibrary/Private/CommonUtils.cpp`

Namespace:

```cpp
namespace EditorLog
```

| 함수 | 의미 |
| --- | --- |
| `EditorMessage(EEditorLogVerbosity, LogName, Message)` | 에디터 Message Log에 메시지 출력 후 Log 창 열기 |
| `EditorMessage(EEditorLogVerbosity, LogName, Message, TargetObject)` | UObject 토큰이 포함된 Message Log 출력 |
| `EditorClearMessage(LogName)` | 지정 Message Log 목록 비우기 |
| `EditorPopup(Message)` | OK 버튼만 있는 Message Dialog 표시 |
| `EditorNotify(EEditorLogVerbosity, Message)` | 에디터 우하단 Notification 표시 |

편의 매크로:

| 매크로 | 의미 |
| --- | --- |
| `EDITOR_MESSAGE_LOG(LogName, Format, ...)` | Info Message Log |
| `EDITOR_MESSAGE_WARNING(LogName, Format, ...)` | Warning Message Log |
| `EDITOR_MESSAGE_ERROR(LogName, Format, ...)` | Error Message Log |
| `EDITOR_MESSAGE_ERROR_OBJECT(LogName, Object, Format, ...)` | UObject 링크가 포함된 Error Message Log |
| `EDITOR_MESSAGE_CLEAR(LogName)` | Message Log Clear |
| `EDITOR_POPUP(Format, ...)` | Modal Popup |
| `EDITOR_NOTIFY_LOG(Format, ...)` | Success Notification |
| `EDITOR_NOTIFY_WARNING(Format, ...)` | Warning/Fail Notification |
| `EDITOR_NOTIFY_ERROR(Format, ...)` | Error Notification |

비에디터 빌드에서는 구현부가 `#if WITH_EDITOR`로 막혀 있으므로 호출해도 아무 에디터 UI가 표시되지 않는다.

사용 예:

```cpp
static const FName CompileLogName(TEXT("MyCompile"));

EDITOR_MESSAGE_CLEAR(CompileLogName);
EDITOR_MESSAGE_LOG(CompileLogName, TEXT("Compile started."));
EDITOR_MESSAGE_ERROR_OBJECT(CompileLogName, Asset, TEXT("Invalid asset setting."));
EDITOR_NOTIFY_ERROR(TEXT("Compile failed."));
```

---

### Boolean Fold 헬퍼

파일: `Source/CommonLibrary/Public/CommonUtils.h`

| 함수 | 반환 정책 |
| --- | --- |
| `IsAny()` | 인자가 없으면 `false` |
| `IsAny(Args&&...)` | 하나라도 truthy면 `true` |
| `IsAll()` | 인자가 없으면 `true` |
| `IsAll(Args&&...)` | 모두 truthy면 `true` |

---

### 유효성 검사 헬퍼

파일: `Source/CommonLibrary/Public/CommonUtils.h`

제공 overload:

| 대상 | 처리 |
| --- | --- |
| `TWeakObjectPtr<T>` | `_weak_obj_ptr.IsValid()` |
| `TSharedPtr<T>` | `_shared_ptr.IsValid()` |
| UObject가 아닌 Raw Pointer | `ptr != nullptr` |
| UObject Pointer | Unreal Engine 전역 `IsValid(UObject*)` 사용 |

편의 함수:

| 함수 | 반환 정책 |
| --- | --- |
| `IsAllValid()` | 인자가 없으면 `true` |
| `IsAllValid(Args&&...)` | 모든 인자가 유효하면 `true` |
| `IsInvalid(Arg)` | 단일 인자의 유효성 반전 |
| `IsAnyInvalid()` | 인자가 없으면 `false` |
| `IsAnyInvalid(Args&&...)` | 하나라도 유효하지 않으면 `true` |

사용 예:

```cpp
if (IsAnyInvalid(World, PlayerController, WidgetClass))
{
	TRACE_WARNING(TEXT("Required object is invalid."));
	return;
}
```

주의:

- UObject 포인터의 경우 Unreal Engine의 전역 `IsValid`가 사용되므로 pending kill 상태까지 고려한다.
- UObject가 아닌 일반 포인터는 nullptr 여부만 확인한다.

---

### Enum 헬퍼

파일: `Source/CommonLibrary/Public/CommonUtils.h`

| 함수 | 의미 |
| --- | --- |
| `IsValidEnumValue(const UEnum*, int64)` | UEnum에 해당 값이 존재하는지 확인 |
| `IsValidEnumValue<TEnum>(TEnum)` | `StaticEnum<TEnum>()` 기반으로 값 유효성 확인 |
| `TEnumToString<TEnum>(TEnum)` | 유효한 Enum 값이면 `GetNameStringByValue` 반환 |

`TEnumToString` 반환 정책:

| 상황 | 반환 |
| --- | --- |
| `StaticEnum<TEnum>() == nullptr` | `InvalidEnumType` |
| Enum 값이 UEnum에 없음 | `InvalidEnumValue` |
| 정상 값 | Enum 이름 문자열 |

사용 예:

```cpp
const FString StateName = TEnumToString(CurrentState);
TRACE_LOG(TEXT("CurrentState=%s"), *StateName);
```

---

### `UCommonUtils` Subsystem / Controller 접근

파일: `Source/CommonLibrary/Public/CommonUtils.h`

`UCommonUtils`는 `UBlueprintFunctionLibrary` 기반 클래스다.

C++ 템플릿 함수:

| 함수 | 제약 | 의미 |
| --- | --- | --- |
| `GetGameInstanceSubsystem<T>(WorldContext)` | `T : UGameInstanceSubsystem` | WorldContext에서 GameInstance를 찾고 해당 Subsystem 반환 |
| `GetLocalPlayerSubsystem<T>(WorldContext)` | `T : ULocalPlayerSubsystem` | 첫 LocalPlayer에서 LocalPlayerSubsystem 반환 |
| `GetLocalPlayerController<T>(WorldContext)` | `T : APlayerController` | 첫 PlayerController를 지정 타입으로 반환 |

Blueprint 함수:

| 함수 | 의미 |
| --- | --- |
| `GetLocalPlayerController(WorldContext)` | 첫 Local PlayerController 반환 |

C++ 예:

```cpp
#include "CommonUtils.h"

UMyGameInstanceSubsystem* MySubsystem =
	UCommonUtils::GetGameInstanceSubsystem<UMyGameInstanceSubsystem>(this);

UMyLocalPlayerSubsystem* MyLocalSubsystem =
	UCommonUtils::GetLocalPlayerSubsystem<UMyLocalPlayerSubsystem>(this);

AMyPlayerController* PC =
	UCommonUtils::GetLocalPlayerController<AMyPlayerController>(this);
```

---

## 다른 플러그인에서의 사용 방식

CommonLibrary를 사용하는 플러그인은 일반적으로 두 군데에 의존성을 추가한다.

### `.uplugin`

```json
"Plugins": [
	{
		"Name": "CommonLibrary",
		"Enabled": true
	}
]
```

### `*.Build.cs`

```csharp
PublicDependencyModuleNames.AddRange(
	new string[]
	{
		"Core",
		"CoreUObject",
		"Engine",
		"CommonLibrary"
	}
);
```

사용 코드에서는 다음 헤더를 포함한다.

```cpp
#include "CommonUtils.h"
```

---

## 현재 한계 / 주의점

- 로그 카테고리 이름이 범용 `MyLog`라서 플러그인별 필터링이 필요하면 별도 카테고리를 추가해야 한다.
- `TRACE_LOG`, `TRACE_WARNING`은 화면 로그도 함께 출력하므로 반복 Tick 경로에서 과도하게 호출하지 않는다.
- `TRACE_ERROR`는 에디터 Debug 빌드에서 `check(false)`를 발생시키므로 복구 가능한 실패에는 `TRACE_WARNING` 또는 일반 에러 로그를 사용한다.
- 에디터 로그/알림 함수는 비에디터 빌드에서 아무 UI 효과가 없다. 런타임 게임플레이 로직을 이 동작에 의존시키지 않는다.
- Blueprint에 직접 노출된 함수는 현재 `GetLocalPlayerController`뿐이다. 나머지 템플릿/매크로 헬퍼는 C++ 전용이다.
- `GetLocalPlayerSubsystem`과 `GetLocalPlayerController`는 첫 LocalPlayer/첫 PlayerController 기준이다.

---

## 추천 코드 읽기 순서

1. `CommonLibrary.uplugin`
2. `Source/CommonLibrary/CommonLibrary.Build.cs`
3. `Source/CommonLibrary/Public/CommonLibrary.h`
4. `Source/CommonLibrary/Private/CommonLibrary.cpp`
5. `Source/CommonLibrary/Public/CommonUtils.h`
6. `Source/CommonLibrary/Private/CommonUtils.cpp`

---

## 관련 문서

- [User Guide](./UserGuide.md) — 플러그인을 사용하는 개발자용 작업 가이드
