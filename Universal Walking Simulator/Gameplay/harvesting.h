#pragma once

#include <Gameplay/helper.h>
#include <Gameplay/inventory.h>

inline bool OnDamageServerHook(UObject* BuildingActor, UFunction* Function, void* Parameters)
{
	static auto BuildingSMActorClass = FindObject(("Class /Script/FortniteGame.BuildingSMActor"));
	if (BuildingActor->IsA(BuildingSMActorClass)) // || BuildingActor->GetFullName().contains(("Car_")))
	{
		auto InstigatedByOffset = FindOffsetStruct(("Function /Script/FortniteGame.BuildingActor.OnDamageServer"), ("InstigatedBy"));
		auto InstigatedBy = *(UObject**)(__int64(Parameters) + InstigatedByOffset);

		auto DamageCauserOffset = FindOffsetStruct(("Function /Script/FortniteGame.BuildingActor.OnDamageServer"), ("DamageCauser"));
		auto DamageCauser = *(UObject**)(__int64(Parameters) + DamageCauserOffset);

		auto DamageTagsOffset = FindOffsetStruct(("Function /Script/FortniteGame.BuildingActor.OnDamageServer"), ("DamageTags"));
		auto DamageTags = (FGameplayTagContainer*)(__int64(Parameters) + DamageTagsOffset);

		auto DamageOffset = FindOffsetStruct(("Function /Script/FortniteGame.BuildingActor.OnDamageServer"), ("Damage"));
		auto Damage = (float*)(__int64(Parameters) + DamageOffset);

		struct Bitfield
		{
			unsigned char                                      UnknownData09 : 1;                                        // 0x0544(0x0001)
			unsigned char                                      bWorldReadyCalled : 1;                                    // 0x0544(0x0001) (Transient)
			unsigned char                                      bBeingRotatedOrScaled : 1;                                // 0x0544(0x0001) (Transient)
			unsigned char                                      bBeingTranslated : 1;                                     // 0x0544(0x0001) (Transient)
			unsigned char                                      bRotateInPlaceEditor : 1;                                 // 0x0544(0x0001)
			unsigned char                                      bEditorPlaced : 1;                                        // 0x0544(0x0001) (Net, Transient)
			unsigned char                                      bPlayerPlaced : 1;                                        // 0x0544(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, Net, DisableEditOnTemplate)
			unsigned char                                      bShouldTick : 1;
		};

		auto BitField = BuildingActor->Member<Bitfield>(("bPlayerPlaced"));
		auto bPlayerPlaced = false; // BitField->bPlayerPlaced;

		static auto FortPlayerControllerAthenaClass = FindObject(("Class /Script/FortniteGame.FortPlayerControllerAthena"));

		// if (DamageTags)
			// std::cout << ("DamageTags: ") << DamageTags->ToStringSimple(false) << '\n';

		if (!bPlayerPlaced && InstigatedBy && InstigatedBy->IsA(FortPlayerControllerAthenaClass) &&
			DamageCauser->GetFullName().contains("B_Melee_Impact_Pickaxe_Athena_C")) // cursed
		{
			// TODO: Not hardcode the PickaxeDef, do like slot 0  or something
			static auto PickaxeDef = FindObject(("FortWeaponMeleeItemDefinition /Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01"));
			auto CurrentWeapon = *(*InstigatedBy->Member<UObject*>(("MyFortPawn")))->Member<UObject*>(("CurrentWeapon"));

			if (CurrentWeapon && *CurrentWeapon->Member<UObject*>(("WeaponData")) == PickaxeDef)
			{
				std::random_device rd; // obtain a random number from hardware
				std::mt19937 gen(rd()); // seed the generator
				std::uniform_int_distribution<> distr(4, 6); // define the range

				auto Random = distr(gen);

				auto HitWeakspot = (*Damage) == 100.f;

				auto funne = distr(gen);

				if (HitWeakspot)
				{
					std::random_device rd; // obtain a random number from hardware
					std::mt19937 gen(rd()); // seed the generator
					std::uniform_int_distribution<> distr(4, 5); // define the range

					funne += distr(gen);
				}

				struct
				{
					UObject* BuildingSMActor;                                          // (Parm, ZeroConstructor, IsPlainOldData)
					TEnumAsByte<EFortResourceType>                     PotentialResourceType;                                    // (Parm, ZeroConstructor, IsPlainOldData)
					int                                                PotentialResourceCount;                                   // (Parm, ZeroConstructor, IsPlainOldData)
					bool                                               bDestroyed;                                               // (Parm, ZeroConstructor, IsPlainOldData)
					bool                                               bJustHitWeakspot;                                         // (Parm, ZeroConstructor, IsPlainOldData)
				} AFortPlayerController_ClientReportDamagedResourceBuilding_Params{ BuildingActor, *BuildingActor->Member<TEnumAsByte<EFortResourceType>>(("ResourceType")),
					 funne, false, HitWeakspot }; // ender weakspotrs

				static auto ClientReportDamagedResourceBuilding = InstigatedBy->Function(("ClientReportDamagedResourceBuilding"));

				if (ClientReportDamagedResourceBuilding)
				{
					auto Params = &AFortPlayerController_ClientReportDamagedResourceBuilding_Params;

					InstigatedBy->ProcessEvent(ClientReportDamagedResourceBuilding, &AFortPlayerController_ClientReportDamagedResourceBuilding_Params);

					// idk y hook no work

					auto Pawn = *InstigatedBy->Member<UObject*>(("Pawn"));

					static auto WoodItemData = FindObject(("FortResourceItemDefinition /Game/Items/ResourcePickups/WoodItemData.WoodItemData"));
					static auto StoneItemData = FindObject(("FortResourceItemDefinition /Game/Items/ResourcePickups/StoneItemData.StoneItemData"));
					static auto MetalItemData = FindObject(("FortResourceItemDefinition /Game/Items/ResourcePickups/MetalItemData.MetalItemData"));

					UObject* ItemDef = WoodItemData;

					if (Params->PotentialResourceType.Get() == EFortResourceType::Stone)
						ItemDef = StoneItemData;

					if (Params->PotentialResourceType.Get() == EFortResourceType::Metal)
						ItemDef = MetalItemData;

					auto ItemInstance = Inventory::FindItemInInventory(InstigatedBy, ItemDef);

					int AmountToGive = Params->PotentialResourceCount;

					if (ItemInstance && Pawn)
					{
						auto Entry = ItemInstance->Member<__int64>(("ItemEntry"));

						// BUG: You lose some mats if you have like 998 or idfk
						if (*FFortItemEntry::GetCount(Entry) >= 999)
						{
							Helper::SummonPickup(Pawn, ItemDef, Helper::GetActorLocation(Pawn), EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, AmountToGive);
							return false;
						}
					}

					Inventory::GiveItem(InstigatedBy, ItemDef, EFortQuickBars::Secondary, 1, AmountToGive);
				}
			}
		}
;	}

	return false;
}

void InitializeHarvestingHooks()
{
	AddHook(("Function /Script/FortniteGame.BuildingActor.OnDamageServer"), OnDamageServerHook);
	// AddHook(("Function /Script/FortniteGame.FortPlayerController.ClientReportDamagedResourceBuilding"), ClientReportDamagedResourceBuildingHook);
}