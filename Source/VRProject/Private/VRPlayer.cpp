// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "VRGameModeBase.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include <Camera/CameraComponent.h>
#include <MotionControllerComponent.h>
#include <DrawDebugHelpers.h>
#include <HeadMountedDisplayFunctionLibrary.h>
#include <Components/CapsuleComponent.h>

// Sets default values
AVRPlayer::AVRPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(RootComponent);
	VRCamera->bUsePawnControlRotation = true;

	// 손추가
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(RootComponent);
	LeftHand->SetTrackingMotionSource(FName("Left"));
	
	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(RootComponent);
	RightHand->SetTrackingMotionSource(FName("Right"));

	// 스켈레탈메시컴포넌트 만들기
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHand);

	// 스켈레탈메시 로드해서 할당
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));
	if (TempMesh.Succeeded())
	{
		LeftHandMesh->SetSkeletalMesh(TempMesh.Object);
		LeftHandMesh->SetRelativeLocation(FVector(-2.9f, -3.5f, 4.5f));
		LeftHandMesh->SetRelativeRotation(FRotator(-25, -180, 90));
	}

	// 오른손 메시
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHand);

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh2(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_right.SKM_MannyXR_right'"));
	if (TempMesh2.Succeeded())
	{
		RightHandMesh->SetSkeletalMesh(TempMesh2.Object);
		RightHandMesh->SetRelativeLocation(FVector(-2.9f, 3.5f, 4.5f));
		RightHandMesh->SetRelativeRotation(FRotator(25, 0, 90));
	}

	// Teleport
	TeleportCircle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportCircle"));
	TeleportCircle->SetupAttachment(RootComponent);
	TeleportCircle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AVRPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input 사용처리
	auto PC = Cast<APlayerController>( GetWorld()->GetFirstPlayerController() );

	if(PC)
	{
		// LocalPlayer
		auto localPlayer = PC->GetLocalPlayer();
		auto subSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(localPlayer);
		if(subSystem)
		{
			subSystem->AddMappingContext(IMC_VRInput, 0);
		}
	}

	ResetTeleport();
}

// Called every frame
void AVRPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// HMD 가 연결돼 있지 않으면
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		// -> 손이 카메라 방향과 일치하도록 하자
		RightHand->SetRelativeRotation(VRCamera->GetRelativeRotation());
	}

	// 텔레포트 확인 처리
	if (bTeleporting)
	{
		DrawTeleportStraight();
	}
}

// Called to bind functionality to input
void AVRPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto InputSystem = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if(InputSystem)
	{
		// Binding for Moving
		InputSystem->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVRPlayer::Move);
		InputSystem->BindAction(IA_Mouse, ETriggerEvent::Triggered, this, &AVRPlayer::Turn);

		InputSystem->BindAction(IA_Teleport, ETriggerEvent::Started, this, &AVRPlayer::TeleportStart);

		InputSystem->BindAction(IA_Teleport, ETriggerEvent::Completed, this, &AVRPlayer::TeleportEnd);
	}
}

void AVRPlayer::Move(const FInputActionValue& Values)
{
	// 사용자의 입력에따라 앞뒤좌우로 이동하고 싶다.
	// 1. 사용자의 입력에 따라
	FVector2D Axis = Values.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.X);
	AddMovementInput(GetActorRightVector(), Axis.Y);
	//// 2. 앞뒤좌우라는 방향이 필요.
	//FVector Dir(Axis.X, Axis.Y, 0);
	//// 3. 이동하고싶다.
	//// P = P0 + vt
	//FVector P0 = GetActorLocation();
	//FVector vt = Dir * moveSpeed * GetWorld()->DeltaTimeSeconds;
	//FVector P = P0 + vt;
	//SetActorLocation(P);
}

void AVRPlayer::Turn(const FInputActionValue& Values)
{
	FVector2D Axis = Values.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

// 텔레포트 기능 활성화처리
void AVRPlayer::TeleportStart(const FInputActionValue& Values)
{
	// 누르고 있는 중에는 사용자가 어디를 가리키는지 주시하고 싶다.
	bTeleporting = true;
}

void AVRPlayer::TeleportEnd(const FInputActionValue& Values)
{
	// 텔레포트 기능 리셋
	// 만약 텔레포트가 불가능하다면
	if (ResetTeleport() == false)
	{
		// 다음 처리를 하지 않는다.
		return;
	}
	// 텔레포트 위치로 이동하고 싶다.
	SetActorLocation(TeleportLocation + FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

bool AVRPlayer::ResetTeleport()
{
	// 텔레포트써클이 활성화 되어 있을 때만 텔레포트 가능하다.
	bool bCanTeleport = TeleportCircle->GetVisibleFlag();
	// 써클 안보이게 처리
	TeleportCircle->SetVisibility(false);
	bTeleporting = false;

	return bCanTeleport;
}

void AVRPlayer::DrawTeleportStraight()
{
	// 직선을 그리고 싶다.
	// 필요정보 : 시작점, 종료점
	FVector StartPos = RightHand->GetComponentLocation();
	FVector EndPos = StartPos + RightHand->GetForwardVector() * 1000;

	// 두 점 사이에 충돌체가 있는지 체크하자
	CheckHitTeleport(StartPos, EndPos);

	DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, false, -1, 0, 1);
}

bool AVRPlayer::CheckHitTeleport(FVector LastPos, FVector& CurPos)
{
	FHitResult HitInfo;
	bool bHit = HitTest(LastPos, CurPos, HitInfo);
	// 만약 부딪힌 대상이 바닥이라면
	if (bHit && HitInfo.GetActor()->GetName().Contains(TEXT("Floor")))
	{
		// 마지막 점을(EndPos) 최종 점으로 수정하고 싶다.
		CurPos = HitInfo.Location;
		// 써클 활성화
		TeleportCircle->SetVisibility(true);
		// 텔레포트써클을 위치
		TeleportCircle->SetWorldLocation(CurPos);
		TeleportLocation = CurPos;
	}
	else
	{
		TeleportCircle->SetVisibility(false);
	}
	return bHit;
}

bool AVRPlayer::HitTest(FVector LastPos, FVector CurPos, FHitResult& HitInfo)
{
	FCollisionQueryParams Params;
	// 자기자신은 무시해라
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, LastPos, CurPos, ECC_Visibility, Params);

	return bHit;
}

