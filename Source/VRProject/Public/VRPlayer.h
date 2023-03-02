// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "VRPlayer.generated.h"

// 사용자의 입력에따라 앞뒤좌우로 이동하고 싶다.
// 필요속성 : 이동속도, 입력액션, 입력매핑컨텍스트
// 사용자가 텔레포트 버튼을 눌렀다 떼면 텔레포트 되도록 하고 싶다.
// 1. 텔레포트 버튼을 눌렀다 뗐으니까
// 2. 사용자가 그 지점을 가리켰으니까
// 3. 텔레포트 목적지가 필요하다.
// 4. 텔레포트이동 하고 싶다.
UCLASS()
class VRPROJECT_API AVRPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 필요속성 : 이동속도, 입력액션, 입력매핑컨텍스트
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float moveSpeed = 500;
	// Input Mapping Context
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMC_VRInput;
	// Input Action for Move
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Move;

	// 이동처리 함수
	void Move(const FInputActionValue& Values);

public: // 마우스 입력처리
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Mouse;
	// 회전처리 함수
	void Turn(const FInputActionValue& Values);

protected:
	UPROPERTY(VisibleAnywhere, Category="VRCamera")
	class UCameraComponent* VRCamera;

	// 컨트롤러
	UPROPERTY(VisibleAnywhere, Category="MotionController")
	class UMotionControllerComponent* LeftHand;
	UPROPERTY(VisibleAnywhere, Category="MotionController")
	class UMotionControllerComponent* RightHand;
	// 사용할 손 모델
	UPROPERTY(VisibleAnywhere, Category="MotionController")
	class USkeletalMeshComponent* LeftHandMesh;
	UPROPERTY(VisibleAnywhere, Category="MotionController")
	class USkeletalMeshComponent* RightHandMesh;

public:	// Teleport Straight
	UPROPERTY(VisibleAnywhere, Category="Teleport")
	class UStaticMeshComponent* TeleportCircle;

	// 텔레포트 기능 활성화 여부
	bool bTeleporting = false;
	// 버튼 눌렀을 때 처리할 함수
	void TeleportStart(const FInputActionValue& Values);
	// 버튼 뗐을 때 처리할 함수
	void TeleportEnd(const FInputActionValue& Values);
	// 텔레포트 기능 리셋
	bool ResetTeleport();
	// 직선 텔레포트 처리하기
	void DrawTeleportStraight();
	// 텔레포트 선과 충돌체크 함수
	bool CheckHitTeleport(FVector LastPos, FVector& CurPos);
	// 충돌처리 함수
	bool HitTest(FVector LastPos, FVector CurPos, FHitResult& HitInfo);

	// 텔레포트 입력액션
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* IA_Teleport;

	// 텔레포트할 위치
	FVector TeleportLocation;
};
