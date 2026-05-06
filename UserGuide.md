# Common Library User Guide

이 문서는 CommonLibrary 플러그인을 실제 프로젝트/플러그인 코드에서 사용하는 개발자를 위한 작업 가이드다. 코드 구조 설명보다 **어디에 의존성을 추가하고, 어떤 헬퍼를 언제 쓰는지**에 초점을 둔다.

---

## 1. 기본 개념

CommonLibrary는 게임 기능을 직접 제공하는 플러그인이 아니라, 다른 C++ 코드에서 반복해서 필요한 공통 기능을 제공한다.

| 기능 | 사용할 때 |
| --- | --- |
| 로그 매크로 | 함수/라인 정보가 포함된 빠른 디버그 로그가 필요할 때 |
| 에디터 메시지 | Compile/검증 결과를 Message Log, Popup, Notification으로 보여줄 때 |
| 유효성 검사 | UObject, SharedPtr, WeakPtr, Raw Pointer를 함께 검사할 때 |
| Enum 헬퍼 | Enum 값을 안전하게 문자열로 표시할 때 |
| Subsystem 접근 | WorldContext에서 GameInstanceSubsystem/LocalPlayerSubsystem을 빠르게 얻을 때 |
| PlayerController 접근 | WorldContext에서 첫 PlayerController를 얻을 때 |

---

## 2. 처음 설정하기

### 2.1 플러그인 의존성 추가

CommonLibrary를 사용하는 다른 플러그인의 `.uplugin`에 다음을 추가한다.

```json
"Plugins": [
	{
		"Name": "CommonLibrary",
		"Enabled": true
	}
]
```

이미 `Plugins` 배열이 있다면 그 안에 항목만 추가한다.

### 2.2 Build.cs 의존성 추가

CommonLibrary의 헤더나 함수를 사용하는 모듈의 `*.Build.cs`에 `CommonLibrary`를 추가한다.

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

해당 모듈의 public 헤더에서 `CommonUtils.h` 타입/함수를 노출하지 않고 `.cpp` 내부에서만 쓴다면 `PrivateDependencyModuleNames`에 넣을 수도 있다. 이 프로젝트의 플러그인들은 대체로 Public 의존성으로 추가해 사용한다.

### 2.3 코드에서 include

```cpp
#include "CommonUtils.h"
```

---

## 3. 로그 사용하기

### 3.1 단순 호출 추적

현재 함수와 라인만 보고 싶을 때 사용한다.

```cpp
void UMySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TRACE();
}
```

### 3.2 일반 디버그 로그

```cpp
TRACE_LOG(TEXT("Loaded widgets: %d"), LoadedWidgetCount);
```

동작:

- Output Log에 `Display` 로그 출력
- 화면에 초록색 로그를 4초 표시

### 3.3 경고 로그

```cpp
if (IsInvalid(WidgetClass))
{
	TRACE_WARNING(TEXT("WidgetClass is invalid."));
	return;
}
```

동작:

- Output Log에 `Warning` 로그 출력
- 화면에 노란색 로그를 4초 표시

### 3.4 에러 로그

```cpp
if (IsInvalid(RequiredAsset))
{
	TRACE_ERROR(TEXT("RequiredAsset is invalid."));
	return;
}
```

주의:

- 에디터 Debug 빌드에서는 `check(false)`가 실행되어 즉시 중단된다.
- 사용자가 복구할 수 있는 설정 누락, 선택지 없음, 반복 가능한 경고에는 `TRACE_WARNING`을 우선 사용한다.
- 패키지/Development 등 다른 빌드에서는 에러 로그만 남긴다.

### 3.5 화면 로그 없이 Output Log만 쓰기

```cpp
CUSTOM_LOG(Display, TEXT("Only output log: %s"), *Name.ToString());
```

`TRACE_LOG`/`TRACE_WARNING`은 화면 로그도 함께 찍히므로 Tick처럼 자주 호출되는 곳에서는 `CUSTOM_LOG`를 고려한다.

---

## 4. 에디터 Message Log 사용하기

에셋 검증, 그래프 Compile, Registry 갱신처럼 에디터 작업 결과를 사용자에게 보여줄 때 사용한다.

### 4.1 로그 이름 정하기

```cpp
static const FName MyCompileLogName(TEXT("MyCompile"));
```

LogName별로 에디터 Message Log 목록이 분리된다.

### 4.2 이전 메시지 지우기

```cpp
EDITOR_MESSAGE_CLEAR(MyCompileLogName);
```

### 4.3 일반 메시지 출력

```cpp
EDITOR_MESSAGE_LOG(MyCompileLogName, TEXT("Compile started."));
EDITOR_MESSAGE_WARNING(MyCompileLogName, TEXT("Optional field is empty."));
EDITOR_MESSAGE_ERROR(MyCompileLogName, TEXT("Required field is missing."));
```

호출 시 Message Log 창이 열린다.

### 4.4 에셋 링크가 포함된 에러 출력

```cpp
EDITOR_MESSAGE_ERROR_OBJECT(
	MyCompileLogName,
	TargetAsset,
	TEXT("Target asset has invalid settings.")
);
```

Message Log에 UObject 토큰이 추가되어 사용자가 해당 에셋으로 이동하기 쉽다.

### 4.5 Notification 사용

```cpp
EDITOR_NOTIFY_LOG(TEXT("Refresh success."));
EDITOR_NOTIFY_WARNING(TEXT("Refresh completed with warnings."));
EDITOR_NOTIFY_ERROR(TEXT("Refresh failed."));
```

사용 기준:

- `EDITOR_MESSAGE_*`: 상세한 오류 목록, 에셋 링크, 누적 로그가 필요할 때
- `EDITOR_NOTIFY_*`: 작업 성공/실패를 짧게 알려줄 때
- `EDITOR_POPUP`: 사용자가 즉시 확인해야 하는 모달 경고가 필요할 때

---

## 5. 유효성 검사 사용하기

### 5.1 단일 값 검사

```cpp
if (IsInvalid(TargetObject))
{
	return;
}
```

`IsInvalid`는 `!IsValid(...)`의 편의 함수다.

### 5.2 여러 값 중 하나라도 invalid인지 확인

```cpp
if (IsAnyInvalid(World, PlayerController, WidgetClass))
{
	TRACE_WARNING(TEXT("Required references are invalid."));
	return;
}
```

### 5.3 모든 값이 valid인지 확인

```cpp
if (IsAllValid(UserWidget, UserWidget->WidgetTree, UserWidget->WidgetTree->RootWidget))
{
	// 모두 유효할 때만 실행
}
```

주의:

- 위 예시처럼 체인 접근을 인자로 넘길 때는 앞 인자가 null이면 뒤 인자를 평가하기 전에 이미 접근이 일어날 수 있다.
- 안전하지 않은 체인 접근은 단계별로 나누는 것이 좋다.

안전한 예:

```cpp
if (IsInvalid(UserWidget))
{
	return;
}

if (IsAllValid(UserWidget->WidgetTree, UserWidget->WidgetTree->RootWidget))
{
	// 안전하게 사용
}
```

### 5.4 빈 인자 정책

| 호출 | 반환 |
| --- | --- |
| `IsAllValid()` | `true` |
| `IsAnyInvalid()` | `false` |
| `IsAll()` | `true` |
| `IsAny()` | `false` |

---

## 6. Enum 문자열 사용하기

Enum 값을 로그나 UI 텍스트에 넣을 때 사용한다.

```cpp
const FString TypeText = TEnumToString(ItemType);
TRACE_LOG(TEXT("ItemType=%s"), *TypeText);
```

반환값:

| 상황 | 반환 문자열 |
| --- | --- |
| 정상 Enum 값 | Enum 이름 |
| `StaticEnum<TEnum>()`을 찾지 못함 | `InvalidEnumType` |
| UEnum에 없는 값 | `InvalidEnumValue` |

값 유효성만 확인하려면 다음을 사용한다.

```cpp
if (!IsValidEnumValue(ItemType))
{
	TRACE_WARNING(TEXT("Invalid enum value."));
}
```

---

## 7. Subsystem 얻기

### 7.1 GameInstanceSubsystem

```cpp
USaveGameSubsystem* SaveSubsystem =
	UCommonUtils::GetGameInstanceSubsystem<USaveGameSubsystem>(this);

if (IsInvalid(SaveSubsystem))
{
	TRACE_WARNING(TEXT("SaveSubsystem is invalid."));
	return;
}
```

내부 절차:

1. WorldContext 유효성 확인
2. `WorldContext->GetWorld()` 확인
3. `World->GetGameInstance()` 확인
4. `GameInstance->GetSubsystem<T>()` 반환

### 7.2 LocalPlayerSubsystem

```cpp
UWidgetSubsystem* WidgetSubsystem =
	UCommonUtils::GetLocalPlayerSubsystem<UWidgetSubsystem>(this);
```

내부 절차:

1. WorldContext 유효성 확인
2. World 확인
3. `World->GetFirstLocalPlayerFromController()` 확인
4. `LocalPlayer->GetSubsystem<T>()` 반환

주의:

- 첫 LocalPlayer 기준이다.
- Split-screen이나 다중 LocalPlayer가 필요하면 별도 정책을 가진 함수를 만들어야 한다.

### 7.3 PlayerController

C++에서 타입을 지정할 수 있다.

```cpp
AMyPlayerController* PC =
	UCommonUtils::GetLocalPlayerController<AMyPlayerController>(this);
```

Blueprint에서는 타입 없는 `APlayerController*` 반환 노드가 노출된다.

```cpp
APlayerController* PC = UCommonUtils::GetLocalPlayerController(this);
```

---

## 8. Blueprint에서 사용하기

현재 Blueprint에 직접 노출된 CommonLibrary 함수는 다음 하나다.

| Blueprint 노드 | C++ 함수 | 설명 |
| --- | --- | --- |
| `Get Local Player Controller` | `UCommonUtils::GetLocalPlayerController` | WorldContext에서 첫 PlayerController 반환 |

로그 매크로, 에디터 메시지 매크로, 템플릿 Subsystem 헬퍼, Enum 헬퍼는 C++ 전용이다.

---

## 9. 추천 사용 패턴

### 9.1 설정 누락은 경고 또는 에디터 에러로 보고하기

런타임에서 복구 가능한 설정 누락:

```cpp
TRACE_WARNING(TEXT("Widget registry is not configured."));
```

에디터 검증에서 사용자가 고쳐야 하는 오류:

```cpp
EDITOR_MESSAGE_ERROR_OBJECT(LogName, Asset, TEXT("Registry is missing."));
EDITOR_NOTIFY_ERROR(TEXT("Validation failed."));
```

### 9.2 치명적 개발 오류만 `TRACE_ERROR` 사용

`TRACE_ERROR`는 에디터 Debug 빌드에서 assert가 발생한다.

적합한 예:

- 코드상 절대 들어오면 안 되는 상태
- 잘못된 위젯 타입이 패널에 들어온 경우
- 개발 중 즉시 잡아야 하는 invariant 위반

부적합한 예:

- 유저 입력 오류
- 선택 가능한 데이터가 일시적으로 비어 있음
- 에셋 설정이 아직 완료되지 않은 상태

### 9.3 Message Log와 Notification 함께 쓰기

상세 내용은 Message Log에 남기고, 최종 결과만 Notification으로 알려준다.

```cpp
EDITOR_MESSAGE_CLEAR(LogName);
EDITOR_MESSAGE_LOG(LogName, TEXT("Refresh started."));

if (bSuccess)
{
	EDITOR_MESSAGE_LOG(LogName, TEXT("Refresh success."));
	EDITOR_NOTIFY_LOG(TEXT("Refresh success."));
}
else
{
	EDITOR_MESSAGE_ERROR(LogName, TEXT("Refresh failed."));
	EDITOR_NOTIFY_ERROR(TEXT("Refresh failed."));
}
```

---

## 10. 문제 해결

### `CommonUtils.h`를 찾을 수 없음

확인할 것:

1. 사용하는 플러그인의 `.uplugin`에 `CommonLibrary` 플러그인 의존성이 있는지 확인한다.
2. 사용하는 모듈의 `*.Build.cs`에 `CommonLibrary` 모듈 의존성이 있는지 확인한다.
3. Unreal Editor/IDE 프로젝트 파일을 재생성하거나 다시 빌드한다.

### `TRACE_ERROR` 때문에 에디터가 멈춤

에디터 Debug 빌드에서는 의도된 동작이다. 복구 가능한 상황이면 `TRACE_WARNING`으로 낮춘다.

### `GetLocalPlayerSubsystem`이 nullptr을 반환함

확인할 것:

1. WorldContext가 유효한 객체인지 확인한다.
2. 아직 LocalPlayer가 생성되기 전 시점인지 확인한다.
3. 대상 타입이 실제 `ULocalPlayerSubsystem`에서 파생되었는지 확인한다.
4. 다중 로컬 플레이어 환경이라면 첫 LocalPlayer 기준이 맞는지 확인한다.

### `TEnumToString`이 `InvalidEnumType`을 반환함

확인할 것:

- Enum이 Unreal reflection에 등록되어 있는지 확인한다.
- 일반 C++ enum이 아니라 `UENUM()` 기반 Enum인지 확인한다.

---

## 11. 새 코드에 적용하는 순서

1. `.uplugin`에 `CommonLibrary` 플러그인 의존성을 추가한다.
2. `*.Build.cs`에 `CommonLibrary` 모듈 의존성을 추가한다.
3. 사용할 `.cpp` 또는 `.h`에 `#include "CommonUtils.h"`를 추가한다.
4. 반복되는 null 검사 코드를 `IsAnyInvalid`/`IsAllValid`로 정리한다.
5. 반복되는 Subsystem 접근 코드를 `UCommonUtils` 템플릿 함수로 정리한다.
6. 에디터 검증 결과는 `EDITOR_MESSAGE_*`와 `EDITOR_NOTIFY_*`로 분리해 표시한다.
7. 반복 Tick 경로에서는 화면 로그가 남는 `TRACE_LOG`/`TRACE_WARNING`을 남발하지 않는다.
8. 치명적 invariant가 아닌 경우 `TRACE_ERROR` 대신 `TRACE_WARNING`을 사용한다.
