// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "VRGameModeBase.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"

// Sets default values
AVRPlayer::AVRPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
}

// Called every frame
void AVRPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVRPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto InputSystem = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if(InputSystem)
	{
		// Binding
		InputSystem->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVRPlayer::Move);
	}
}

void AVRPlayer::Move(const FInputActionValue& Values)
{
	UE_LOG(LogTemp, Warning, TEXT("Move !!!!"));
	// ������� �Է¿����� �յ��¿�� �̵��ϰ� �ʹ�.
	// 1. ������� �Է¿� ����
	FVector2D Axis = Values.Get<FVector2D>();
	// 2. �յ��¿��� ������ �ʿ�.
	FVector Dir(Axis.X, Axis.Y, 0);
	// 3. �̵��ϰ�ʹ�.
	// P = P0 + vt
	FVector P0 = GetActorLocation();
	FVector vt = Dir * moveSpeed * GetWorld()->DeltaTimeSeconds;
	FVector P = P0 + vt;
	SetActorLocation(P);
}

