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
#include <NiagaraComponent.h>
#include <../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h>
#include <Haptics/HapticFeedbackEffect_Curve.h>
#include <UMG/Public/Components/WidgetInteractionComponent.h>

#define PRINTTOScreen(msg) GEngine->AddOnScreenDebugMessage(0, 1, FColor::Blue, msg)
// Sets default values
AVRPlayer::AVRPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(RootComponent);
	VRCamera->bUsePawnControlRotation = false;

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
	TeleportCircle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TeleportCircle"));
	TeleportCircle->SetupAttachment(RootComponent);
	TeleportCircle->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TeleportCurveComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TeleportCurveComp"));
	TeleportCurveComp->SetupAttachment(RootComponent);

	// ���Լհ���
	RightAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightAim"));
	RightAim->SetupAttachment(RootComponent);
	RightAim->SetTrackingMotionSource(FName("RightAim"));

	WidgetInteractionComp = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionComp"));
	WidgetInteractionComp->SetupAttachment(RightAim);
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

	// ũ�ν���� ��ü �����
	if (CrosshairFactory)
	{
		Crosshair = GetWorld()->SpawnActor<AActor>(CrosshairFactory);
	}

	// HMD �� ����Ǿ� ���� �ʴٸ� 
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		// Hand �� �׽�Ʈ �� �� �ִ� ��ġ�� �̵���Ű��.
		RightHand->SetRelativeLocation(FVector(20, 20, 0));
		RightAim->SetRelativeLocation(FVector(20, 20, 0));
		// ī�޶��� Use Pawn Control Rotation �� Ȱ��ȭ ��Ű��
		VRCamera->bUsePawnControlRotation = true;
	}
	// ���� HMD �� ����Ǿ� �ִٸ�
	else
	{
		// -> �⺻ Ʈ��ŷ offset ����
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	}
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
		RightAim->SetRelativeRotation(VRCamera->GetRelativeRotation());
	}

	// �ڷ���Ʈ Ȯ�� ó��
	if (bTeleporting)
	{
		// ���� ������ �׸��ٸ�
		if (bTeleportCurve == false)
		{
			DrawTeleportStraight();
		}
		// �׷��� ������
		else
		{
			// � �׸���
			DrawTeleportCurve();
		}

		// ���̾ư��� �̿��� ���׸���
		if (TeleportCurveComp)
		{
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(TeleportCurveComp, FName(TEXT("User.PointArray")), Lines);
		}
	}

	// Crosshair
	DrawCrosshair();

	Grabbing();
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

		InputSystem->BindAction(IA_Fire, ETriggerEvent::Started, this, &AVRPlayer::FireInput);
		InputSystem->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AVRPlayer::ReleaseUIInput);
		
		// ���
		InputSystem->BindAction(IA_Grab, ETriggerEvent::Started, this, &AVRPlayer::TryGrab);
		InputSystem->BindAction(IA_Grab, ETriggerEvent::Completed, this, &AVRPlayer::UnTryGrab);
	}
}

void AVRPlayer::Move(const FInputActionValue& Values)
{
	// ������� �Է¿����� �յ��¿�� �̵��ϰ� �ʹ�.
	// 1. ������� �Է¿� ����
	FVector2D Axis = Values.Get<FVector2D>();
	PRINTTOScreen(FString::Printf(TEXT("x : %.2f, y : %.2f"), Axis.X, Axis.Y));
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
	// ������ ���̵��� Ȱ��ȭ
	TeleportCurveComp->SetVisibility(true);
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

	// ���� ���� ����ó��
	if (IsWarp)
	{
		DoWarp();
		return;
	}
	// �׷��� ������� �ڷ���Ʈ
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
	TeleportCurveComp->SetVisibility(false);

	return bCanTeleport;
}

void AVRPlayer::DrawTeleportStraight()
{
	Lines.RemoveAt(0, Lines.Num());
	// ������ �׸��� �ʹ�.
	// �ʿ����� : ������, ������
	FVector StartPos = RightAim->GetComponentLocation();
	FVector EndPos = StartPos + RightAim->GetForwardVector() * 1000;

	// �� �� ���̿� �浹ü�� �ִ��� üũ����
	CheckHitTeleport(StartPos, EndPos);
	Lines.Add(StartPos);
	Lines.Add(EndPos);
	//DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, false, -1, 0, 1);
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

// �־��� �ӵ��� ����ü�� ���������� ����ü�� ������ ���� �������
void AVRPlayer::DrawTeleportCurve()
{
	// Lines �ʱ�ȭ
	Lines.RemoveAt(0, Lines.Num());
	// �־��� �ӵ��� ����ü�� ���������� ����ü�� ������ ���� �������
	// 1. ������, ����, ���� ����ü�� ������.
	FVector Pos = RightAim->GetComponentLocation();
	FVector Dir = RightAim->GetForwardVector() * CurvedPower;
	// �������� ���� ���� �������
	Lines.Add(Pos);
	for (int i = 0; i < LineSmooth; i++)
	{
		// ���� �� ���
		FVector LastPos = Pos;
		// 2. ����ü�� �̵������ϱ� �ݺ������� 
		// v = v0 + at
		Dir += FVector::UpVector * Gravity * SimulatedTime;
		// P = P0 + vt
		Pos += Dir * SimulatedTime;
		// 3. ����ü�� ��ġ����
		//  -> ���� �� ���̿� ��ü�� ���θ��� �ִٸ�
		if (CheckHitTeleport(LastPos, Pos))
		{
			//		-> ������ ������ ������ ����
			Lines.Add(Pos);
			break;
		}
		// 4. ���� �������
		Lines.Add(Pos);
	}
	//// � �׸���
	//for(int i=0;i<Lines.Num()-1;i++)
	//{
	//	DrawDebugLine(GetWorld(), Lines[i], Lines[i+1], FColor::Red, false, -1, 0, 1);
	//}
}

void AVRPlayer::DoWarp()
{
	// ��������� Ȱ��ȭ �Ǿ� ���� ��
	if (IsWarp == false)
	{
		return;
	}
	// ����ó�� �ϰ� �ʹ�.
	// -> �����ð����� ������ �̵��ϴ°ž�
	// ����ð� �ʱ�ȭ
	CurTime = 0;
	// �浹ü ��Ȱ��ȭ 
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 3. �ð��� �귯���Ѵ�.
	// 2. �����ð�����
	// [ĸó]()->{ body }
	GetWorld()->GetTimerManager().SetTimer(WarpHandle, FTimerDelegate::CreateLambda(
		[this]()->void
		{
			// body
			// �����ð��ȿ� �������� �����ϰ� �ʹ�.
			// 1. �ð��� �귯���Ѵ�.
			CurTime += GetWorld()->DeltaTimeSeconds;
			// ����
			FVector CurPos = GetActorLocation();
			// ����
			FVector EndPos = TeleportLocation + FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			// 2. �̵��ؾ��Ѵ�.
			CurPos = FMath::Lerp<FVector>(CurPos, EndPos, CurTime / WarpTime);
			// 3. �������� ����
			SetActorLocation(CurPos);
			// �ð��� �� �귶�ٸ� 
			if (CurTime >= WarpTime)
			{
				// -> �� ��ġ�� �Ҵ��ϰ�
				SetActorLocation(EndPos);
				// -> Ÿ�̸� �������ֱ�
				GetWorld()->GetTimerManager().ClearTimer(WarpHandle);
				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
			// �Ÿ��� ���� ��������ٸ� �� ��ġ�� �Ҵ����ֱ�.
			// -> Ÿ�̸� �����ϱ�(��������)
			/*float Distance = FVector::Dist(CurPos, EndPos);
			if (Distance < 0.1f)
			{
				CurPos = EndPos;
			}*/
		}
	), 0.02f, true);
}

void AVRPlayer::ReleaseUIInput()
{
	if (WidgetInteractionComp)
	{
		WidgetInteractionComp->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

void AVRPlayer::FireInput(const FInputActionValue& Values)
{
	// UI �� �̺�Ʈ �����ϰ� �ʹ�.
	if (WidgetInteractionComp)
	{
		WidgetInteractionComp->PressPointerKey(EKeys::LeftMouseButton);
	}

	// ����ó�� �ϰ� �ʹ�.
	auto PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->PlayHapticEffect(HF_Fire, EControllerHand::Right);
	}
	// LineTrace �̿��ؼ� ���� ��� �ʹ�.
	// ������
	FVector StartPos = RightAim->GetComponentLocation();
	// ������
	FVector EndPos = StartPos + RightAim->GetForwardVector() * 10000;
	// �ѽ��(LineTrace ����)
	FHitResult HitInfo;
	bool bHit = HitTest(StartPos, EndPos, HitInfo);
	// ���� �ε����� �༮�� ������ ����������
	if (bHit)
	{
		auto HitComp = HitInfo.GetComponent();
		if (HitComp && HitComp->IsSimulatingPhysics())
		{
			// ����������
			// F = ma
			HitComp->AddForceAtLocation((EndPos - StartPos).GetSafeNormal() * 1000000 * HitComp->GetMass(), HitInfo.Location);
		}
	}
}

// �Ÿ��� ���� ũ�ν���� ũ�Ⱑ ���� ���̵�������
void AVRPlayer::DrawCrosshair()
{
	// ������
	FVector StartPos = RightAim->GetComponentLocation();
	// ����
	FVector EndPos = StartPos + RightAim->GetForwardVector() * 10000;
	// �浹������ ����
	FHitResult HitInfo;
	// �浹üũ
	bool bHit = HitTest(StartPos, EndPos, HitInfo);

	float Distance = 0;
	// -> �浹�� �߻��ϸ� 
	if (bHit)
	{
		//		-> �浹�� ������ ũ�ν���� ǥ��
		Crosshair->SetActorLocation(HitInfo.Location);
		Distance = HitInfo.Distance;
	}
	// -> �׷��� ������
	else
	{
		//		-> �׳� ������ ũ�ν���� ǥ��
		Crosshair->SetActorLocation(EndPos);
		Distance = (EndPos - StartPos).Size();
	}

	Crosshair->SetActorScale3D(FVector(FMath::Max<float>(1, Distance)));

	// ������
	// -> ũ�ν��� ī�޶� �ٶ󺸵��� ó��
	FVector Direction = Crosshair->GetActorLocation() - VRCamera->GetComponentLocation();
	Crosshair->SetActorRotation(Direction.Rotation());
}

// ��ü�� ��� �ʹ�.
void AVRPlayer::TryGrab()
{
	// �߽���
	FVector Center = RightHand->GetComponentLocation();
	// �浹üũ(���浹)
	// �浹�� ��ü�� ����� �迭
	// �浹 ���� �ۼ�
	FCollisionQueryParams Param;
	Param.AddIgnoredActor(this);
	Param.AddIgnoredComponent(RightHand);
	TArray<FOverlapResult> HitObjs;
	bool bHit = GetWorld()->OverlapMultiByChannel(HitObjs, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(GrabRange), Param);

	// �浹���� �ʾҴٸ� �ƹ�ó�� ���� �ʴ´�.
	if (bHit == false)
	{
		return;
	}
	// -> ���� ����� ��ü �⵵�� ���� (�������)
	
	// ���� ����� ��ü �ε���
	int Closest = 0;
	for (int i = 0; i<HitObjs.Num(); i++)
	{
		// 1. ���� ����� Ȱ��ȭ �Ǿ� �ִ� �༮�� �Ǵ�
		// -> ���� �ε��� ������Ʈ�� ��������� ��Ȱ��ȭ �Ǿ� �ִٸ�
		if (HitObjs[i].GetComponent()->IsSimulatingPhysics() == false)
		{
			// �����ϰ� ���� �ʴ�.
			continue;
		}
		// ��Ҵ�!
		IsGrabbed = true;

		// 2. ���� �հ� ���� ����� �༮�� �̹��� ������ �༮�� �� ����� �༮�� �ִٸ�
		// -> �ʿ�Ӽ� : ���� ���尡��� �༮�� �հ��� �Ÿ�
		float ClosestDist = FVector::Dist(HitObjs[Closest].GetActor()->GetActorLocation(), Center);
		// -> �ʿ�Ӽ� : �̹��� ������ �༮�� �հ��� �Ÿ�
		float NextDist = FVector::Dist(HitObjs[i].GetActor()->GetActorLocation(), Center);
		// 3. ���� �̹����� ���粨 ���� �� �����ٸ�
		if (NextDist < ClosestDist)
		{
			//  -> ���� ����� �༮���� �����ϱ�
			Closest = i;
		}
	}

	// ���� ��Ҵٸ�
	if (IsGrabbed)
	{
		GrabbedObject = HitObjs[Closest].GetComponent();
		// -> ��ü ������� ��Ȱ��ȭ
		GrabbedObject->SetSimulatePhysics(false);
		GrabbedObject->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// -> �տ� �ٿ�����
		GrabbedObject->AttachToComponent(RightHand, FAttachmentTransformRules::KeepWorldTransform);

		PrevPos = RightHand->GetComponentLocation();
	}
}

// ���� �༮�� ������ ����ʹ�.
void AVRPlayer::UnTryGrab()
{
	if (IsGrabbed == false)
	{
		return;
	}

	// 1. �������� ���·� ��ȯ
	IsGrabbed = false;
	// 2. �տ��� �����
	GrabbedObject->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	// 3. ������� Ȱ��ȭ
	GrabbedObject->SetSimulatePhysics(true);
	// 4. �浹��� Ȱ��ȭ
	GrabbedObject->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// ������
	GrabbedObject->AddForce(ThrowDirection * ThrowPower * GrabbedObject->GetMass());

	// ȸ�� ��Ű��
	// ���ӵ� = (1 / dt) * dTheta(Ư�� �� ���� ���� ���� Axis, angle)
	float Angle;
	FVector Axis;
	DeltaRotation.ToAxisAndAngle(Axis, Angle);
	float dt = GetWorld()->DeltaTimeSeconds;
	FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
	GrabbedObject->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);

	GrabbedObject = nullptr;
}

// ���� ������ ������Ʈ�ϱ����� ���
void AVRPlayer::Grabbing()
{
	if (IsGrabbed == false)
	{
		return;
	}

	// �������� ������Ʈ
	ThrowDirection = RightHand->GetComponentLocation() - PrevPos;
	// ȸ������ ������Ʈ
	// ���ʹϿ� ����
	// Angle1 = Q1, Angle2 = Q2
	// Angle1 + Angle2 = Q1 * Q2
	// -Angle1 = Q1.Inverse()
	// Angle2 - Angle1 = Q2 * Q1.Inverse()
	DeltaRotation = RightHand->GetComponentQuat() * PrevRot.Inverse();

	// ������ġ ������Ʈ
	PrevPos = RightHand->GetComponentLocation();
	// ����ȸ���� ������Ʈ
	PrevRot = RightHand->GetComponentQuat();
}



