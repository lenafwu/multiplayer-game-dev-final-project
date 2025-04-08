#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "US_CharacterStats.generated.h"

USTRUCT(BlueprintType)
struct US_LTOL_LW_1_API FUS_CharacterStats : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float WalkSpeed = 200.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float SprintSpeed = 400.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float DamageMultiplier = 1.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 NextLevelXp = 10.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float StealthMultiplier = 1.0f;
};