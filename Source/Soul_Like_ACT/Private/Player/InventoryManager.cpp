// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManager.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "SoulSaveGame.h"
#include "SoulGameInstanceBase.h"

UInventoryManager::UInventoryManager()
{
	PrimaryComponentTick.bCanEverTick = 0;
}

void UInventoryManager::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryManager::EquipGear(AWeaponActor *const Inp)
{
	if (CurrentWeapon)
	{
		FDetachmentTransformRules LocalDetRules{ EDetachmentRule::KeepWorld, 1 };
		CurrentWeapon->DetachFromActor(LocalDetRules);
		CurrentWeapon->Destroy();
	}

	CurrentWeapon = Inp;

	FAttachmentTransformRules LocalTransRules{ EAttachmentRule::SnapToTarget, 1 };
	if(Inp->GearInfo->WeaponType == EWeaponType::VE_1HSword)
		Inp->AttachToComponent(Cast<ACharacter>(GetOwner())->GetMesh(), LocalTransRules, TEXT("2H_Mace"));
	else if (Inp->GearInfo->WeaponType == EWeaponType::VE_2HMace)
		Inp->AttachToComponent(Cast<ACharacter>(GetOwner())->GetMesh(), LocalTransRules, TEXT("2H_Mace"));
	else if (Inp->GearInfo->WeaponType == EWeaponType::VE_Fist)
		Inp->AttachToComponent(Cast<ACharacter>(GetOwner())->GetMesh(), LocalTransRules, TEXT("2H_Mace"));	
}

bool UInventoryManager::SaveInventory()
{
	return false;
}

bool UInventoryManager::LoadInventoryData(TArray<FSoulItemData> InInventoryItems
	, TMap<FSoulEquipmentSlot, FSoulItemData> InEquipedItems)
{
	/*InventoryItems = InInventoryItems;*/
	int i = 0;

	for (FSoulItemData TempItemData : InInventoryItems)
	{
		if (TempItemData.IsValid())
		{
			InventoryItems.Add(FSoulItemSlot(i), TempItemData);
		}
		else
		{
			InventoryItems.Add(FSoulItemSlot(i), FSoulItemData());
		}
		++i;
	}

	EquipedItems = EquipedItems;

	//TODO: update GAs

	Notify_OnInventoryLoadingFinished(true);
	
	return true;
}

void UInventoryManager::Notify_OnInventoryLoadingFinished(bool bFirstTimeInit)
{
	if (OnInventoryLoadingFinished.IsBound())
		OnInventoryLoadingFinished.Broadcast(this, bFirstTimeInit);
}

bool UInventoryManager::FillEmptySlotWithItem(FSoulItemData NewItemData)
{
	// Look for an empty item slot to fill with this item
	FPrimaryAssetType NewItemType = NewItemData.ItemBase->GetPrimaryAssetId().PrimaryAssetType;
	FSoulItemSlot EmptySlot;
	for (TPair<FSoulItemSlot, FSoulItemData>& Pair : InventoryItems)
	{
		if (Pair.Value.ItemBase == NewItemData.ItemBase)
		{
			// Item is already slotted
			return false;
		}
		else if (Pair.Value.ItemBase == nullptr && (!EmptySlot.IsValid() || EmptySlot.SlotNumber > Pair.Key.SlotNumber))
		{
			// We found an empty slot worth filling
			EmptySlot = Pair.Key;
		}
	}

	if (EmptySlot.IsValid())
	{
		InventoryItems[EmptySlot] = NewItemData;
		NotifySlottedItemChanged(EmptySlot, NewItemData);
		return true;
	}

	return false;
}

void UInventoryManager::NotifySlottedItemChanged(FSoulItemSlot ItemSlot, FSoulItemData ItemData)
{
	OnSlottedItemChanged.Broadcast(ItemSlot, ItemData);
}

void UInventoryManager::NotifyEquipmentChanged(FSoulEquipmentSlot EquipmentSlot, FSoulItemData ItemData)
{
	OnEquipmentChangedChanged.Broadcast(EquipmentSlot, ItemData);
}
