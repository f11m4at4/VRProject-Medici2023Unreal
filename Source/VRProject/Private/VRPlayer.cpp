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

	// ���߰�
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(RootComponent);
	LeftHand->SetTrackingMotionSource(FName("Left"));
	
	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(RootComponent);
	RightHand->SetTrackingMotionSource(FName("Right"));

	// ���̷�Ż�޽�������Ʈ �����
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHand);

	// ���̷�Ż�޽� �ε��ؼ� �Ҵ�
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));
	if (TempMesh.Succeeded())
	{
		LeftHandMesh->SetSkeletalMesh(TempMesh.Object);
		LeftHandMesh->SetRelativeLocation(FVector(-2.9f, -3.5f, 4.5f));
		LeftHandMesh->SetRelativeRotation(FRotator(-25, -180, 90));
	}

	// ������ �޽�
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

	// Enhanced Input ���ó��
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

	// HMD �� ����� ���� ������
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		// -> ���� ī�޶� ����� ��ġ�ϵ��� ����
		RightHand->SetRelativeRotation(VRCamera->GetRelativeRotation());
	}

	// �ڷ���Ʈ Ȯ�� ó��
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
	// ������� �Է¿����� �յ��¿�� �̵��ϰ� �ʹ�.
	// 1. ������� �Է¿� ����
	FVector2D Axis = Values.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.X);
	AddMovementInput(GetActorRightVector(), Axis.Y);
	//// 2. �յ��¿��� ������ �ʿ�.
	//FVector Dir(Axis.X, Axis.Y, 0);
	//// 3. �̵��ϰ�ʹ�.
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

// �ڷ���Ʈ ��� Ȱ��ȭó��
void AVRPlayer::TeleportStart(const FInputActionValue& Values)
{
	// ������ �ִ� �߿��� ����ڰ� ��� ����Ű���� �ֽ��ϰ� �ʹ�.
	bTeleporting = true;
}

void AVRPlayer::TeleportEnd(const FInputActionValue& Values)
{
	// �ڷ���Ʈ ��� ����
	// ���� �ڷ���Ʈ�� �Ұ����ϴٸ�
	if (ResetTeleport() == false)
	{
		// ���� ó���� ���� �ʴ´�.
		return;
	}
	// �ڷ���Ʈ ��ġ�� �̵��ϰ� �ʹ�.
	SetActorLocation(TeleportLocation + FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

bool AVRPlayer::ResetTeleport()
{
	// �ڷ���Ʈ��Ŭ�� Ȱ��ȭ �Ǿ� ���� ���� �ڷ���Ʈ �����ϴ�.
	bool bCanTeleport = TeleportCircle->GetVisibleFlag();
	// ��Ŭ �Ⱥ��̰� ó��
	TeleportCircle->SetVisibility(false);
	bTeleporting = false;

	return bCanTeleport;
}

void AVRPlayer::DrawTeleportStraight()
{
	// ������ �׸��� �ʹ�.
	// �ʿ����� : ������, ������
	FVector StartPos = RightHand->GetComponentLocation();
	FVector EndPos = StartPos + RightHand->GetForwardVector() * 1000;

	// �� �� ���̿� �浹ü�� �ִ��� üũ����
	CheckHitTeleport(StartPos, EndPos);

	DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, false, -1, 0, 1);
}

bool AVRPlayer::CheckHitTeleport(FVector LastPos, FVector& CurPos)
{
	FHitResult HitInfo;
	bool bHit = HitTest(LastPos, CurPos, HitInfo);
	// ���� �ε��� ����� �ٴ��̶��
	if (bHit && HitInfo.GetActor()->GetName().Contains(TEXT("Floor")))
	{
		// ������ ����(EndPos) ���� ������ �����ϰ� �ʹ�.
		CurPos = HitInfo.Location;
		// ��Ŭ Ȱ��ȭ
		TeleportCircle->SetVisibility(true);
		// �ڷ���Ʈ��Ŭ�� ��ġ
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
	// �ڱ��ڽ��� �����ض�
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, LastPos, CurPos, ECC_Visibility, Params);

	return bHit;
}

