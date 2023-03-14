// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "VRPlayer.generated.h"

// ������� �Է¿����� �յ��¿�� �̵��ϰ� �ʹ�.
// �ʿ�Ӽ� : �̵��ӵ�, �Է¾׼�, �Է¸������ؽ�Ʈ
// ����ڰ� �ڷ���Ʈ ��ư�� ������ ���� �ڷ���Ʈ �ǵ��� �ϰ� �ʹ�.
// 1. �ڷ���Ʈ ��ư�� ������ �����ϱ�
// 2. ����ڰ� �� ������ ���������ϱ�
// 3. �ڷ���Ʈ �������� �ʿ��ϴ�.
// 4. �ڷ���Ʈ�̵� �ϰ� �ʹ�.
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

	// �ʿ�Ӽ� : �̵��ӵ�, �Է¾׼�, �Է¸������ؽ�Ʈ
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float moveSpeed = 500;
	// Input Mapping Context
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMC_VRInput;
	// Input Action for Move
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Move;

	// �̵�ó�� �Լ�
	void Move(const FInputActionValue& Values);

public: // ���콺 �Է�ó��
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Mouse;
	// ȸ��ó�� �Լ�
	void Turn(const FInputActionValue& Values);

protected:
	UPROPERTY(VisibleAnywhere, Category="VRCamera")
	class UCameraComponent* VRCamera;

	// ��Ʈ�ѷ�
	UPROPERTY(VisibleAnywhere, Category="MotionController")
	class UMotionControllerComponent* LeftHand;
	UPROPERTY(VisibleAnywhere, Category="MotionController")
	class UMotionControllerComponent* RightHand;
	// ����� �� ��
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MotionController")
	class USkeletalMeshComponent* LeftHandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MotionController")
	class USkeletalMeshComponent* RightHandMesh;

public:	// Teleport Straight
	UPROPERTY(VisibleAnywhere, Category="Teleport")
	class UNiagaraComponent* TeleportCircle;

	// �ڷ���Ʈ ��� Ȱ��ȭ ����
	bool bTeleporting = false;
	// ��ư ������ �� ó���� �Լ�
	void TeleportStart(const FInputActionValue& Values);
	// ��ư ���� �� ó���� �Լ�
	void TeleportEnd(const FInputActionValue& Values);
	// �ڷ���Ʈ ��� ����
	bool ResetTeleport();
	// ���� �ڷ���Ʈ ó���ϱ�
	void DrawTeleportStraight();
	// �ڷ���Ʈ ���� �浹üũ �Լ�
	bool CheckHitTeleport(FVector LastPos, FVector& CurPos);
	// �浹ó�� �Լ�
	bool HitTest(FVector LastPos, FVector CurPos, FHitResult& HitInfo);

	// �ڷ���Ʈ �Է¾׼�
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* IA_Teleport;

	// �ڷ���Ʈ�� ��ġ
	FVector TeleportLocation;

private: // � �ڷ���Ʈ
	// � �ڷ���Ʈ ��뿩��
	UPROPERTY(EditAnywhere, Category="Teleport", meta=(AllowPrivateAccess = true))
	bool bTeleportCurve = true;
	// ������ ��
	UPROPERTY(EditAnywhere, Category="Teleport", meta=(AllowPrivateAccess = true))
	float CurvedPower = 1500;
	// �߷�
	UPROPERTY(EditAnywhere, Category="Teleport", meta=(AllowPrivateAccess = true))
	float Gravity = -5000;
	// �ùķ��̼ǽð�
	UPROPERTY(EditAnywhere, Category="Teleport", meta=(AllowPrivateAccess = true))
	float SimulatedTime = 0.02f;
	// ��� �̷�� �� ����
	UPROPERTY(EditAnywhere, Category="Teleport", meta=(AllowPrivateAccess = true))
	int32 LineSmooth = 40;
	// ��������� �迭
	UPROPERTY()
	TArray<FVector> Lines;

	void DrawTeleportCurve();

	// ����� ���̾ư��� ������Ʈ(LineTrace)
	UPROPERTY(VisibleAnywhere, Category="Teleport")
	class UNiagaraComponent* TeleportCurveComp;

private:
	// ���� ��뿩��
	UPROPERTY(EditAnywhere, Category="Teleport", meta=(AllowPrivateAccess = true))
	bool IsWarp = true;

	// Ÿ�̸�
	UPROPERTY()
	FTimerHandle WarpHandle;
	// ����ð�
	UPROPERTY()
	float CurTime = 0;
	// ������ �� �ɸ� �ð�
	UPROPERTY(EditAnywhere, Category="Teleport", meta=(AllowPrivateAccess = true))
	float WarpTime = 0.2f;

	// ���� ������ �Լ�
	void DoWarp();

private: // �ѽ��
	UPROPERTY(EditDefaultsOnly, Category="Input", meta=(AllowPrivateAccess=true))
	class UInputAction* IA_Fire;

	// �ѽ�� ó���� �Լ�
	void FireInput(const FInputActionValue& Values);

	// ���Լհ��� ǥ���� �����Ʈ�ѷ�
	UPROPERTY(VisibleAnywhere, Category="HandComp", meta = (AllowPrivateAccess = true))
	class UMotionControllerComponent* RightAim;

	// Crosshair
	UPROPERTY(EditAnywhere, Category="Crosshair", meta = (AllowPrivateAccess = true))
	TSubclassOf<AActor> CrosshairFactory;

	// �ν��Ͻ� ũ�ν����
	UPROPERTY()
	AActor* Crosshair;
	// ũ�ν���� �׸���
	void DrawCrosshair();
	
public: // ��� ��ư�� ������ ��ü�� ��� �ʹ�.
	// �ʿ�Ӽ� : �Է¾׼�, ���� ���� 
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* IA_Grab;
	// ���� ����
	UPROPERTY(EditDefaultsOnly, Category="Grab")
	float GrabRange = 100;

	// ���� ��ü ���
	UPROPERTY()
	class UPrimitiveComponent* GrabbedObject;

	// ���� �༮�� �ִ��� ���� ����� ����
	bool IsGrabbed = false;

	// ������ ���ϴ� �������� ���ư����� �ϰ�ʹ�.
	// ���� ����
	FVector ThrowDirection;
	// ���� ��
	UPROPERTY(EditAnywhere, Category="Grab")
	float ThrowPower = 1000;
	// ���� ��ġ
	FVector PrevPos;
	// ���� ȸ����
	FQuat PrevRot;
	// ȸ������
	FQuat DeltaRotation;
	// ȸ��������
	UPROPERTY(EditAnywhere, Category="Grab")
	float ToquePower = 1000;

	// ���õ����
	void TryGrab();
	// ����
	void UnTryGrab();
	// ����ִ���
	void Grabbing();

	UPROPERTY(EditDefaultsOnly, Category="Haptic")
	class UHapticFeedbackEffect_Curve* HF_Fire;

protected: // Widget ���� �Ӽ�
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Widget")
	class UWidgetInteractionComponent* WidgetInteractionComp;
	
	// Release ���·� �ǵ�������
	void ReleaseUIInput();

public:
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputMappingContext* IMC_Hand;
};
