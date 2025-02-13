#pragma once

#include <Net/funcs.h>
#include <Gameplay/helper.h>
#include <Gameplay/inventory.h>

#include <unordered_map>

// Includes building and editing..

struct IsDestroyedBitField {
	unsigned char                                      bSurpressHealthBar : 1;                                   // 0x0541(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, DisableEditOnInstance)
	unsigned char                                      bCreateVerboseHealthLogs : 1;                             // 0x0541(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, DisableEditOnInstance)
	unsigned char                                      bIsIndestructibleForTargetSelection : 1;                  // 0x0541(0x0001) (Edit, BlueprintVisible, BlueprintReadOnly, DisableEditOnInstance)
	unsigned char                                      bDestroyed : 1;                                           // 0x0541(0x0001) (BlueprintVisible, BlueprintReadOnly, Net, Transient)
	unsigned char                                      bPersistToWorld : 1;                                      // 0x0541(0x0001) (Edit, DisableEditOnInstance)
	unsigned char                                      bRefreshFullSaveDataBeforeZoneSave : 1;                   // 0x0541(0x0001) (Edit, DisableEditOnInstance)
	unsigned char                                      bBeingDragged : 1;                                        // 0x0541(0x0001) (Transient)
	unsigned char                                      bRotateInPlaceGame : 1;                                   // 0x0541(0x0001)
};

bool CanBuild(UObject* BuildingActor)
{
	if (!bDoubleBuildFix)
		return true;

	if (!BuildingActor)
		return false;

	auto StructuralSupportSystem = Helper::GetStructuralSupportSystem();

	static auto K2_GetBuildingActorsInGridCell = StructuralSupportSystem->Function("K2_GetBuildingActorsInGridCell");

	struct
	{
		struct FVector                                     WorldLocation;                                            // (ConstParm, Parm, OutParm, ReferenceParm, IsPlainOldData)
		FBuildingGridActorFilter                    Filter;                                                   // (ConstParm, Parm, OutParm, ReferenceParm)
		FBuildingNeighboringActorInfo               OutActorsInGridCell;                                      // (Parm, OutParm)
		bool                                               ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
	} UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params{ Helper::GetActorLocation(BuildingActor), FBuildingGridActorFilter{true, true, true, true}};

	if (K2_GetBuildingActorsInGridCell)
		StructuralSupportSystem->ProcessEvent(K2_GetBuildingActorsInGridCell, &UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params);

	std::cout << "Neighboring Wall Size: " << UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params.OutActorsInGridCell.NeighboringWallInfos.Num() << '\n';
	std::cout << "Neighboring Floor Size: " << UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params.OutActorsInGridCell.NeighboringFloorInfos.Num() << '\n';
	std::cout << "Neighboring CenterCell Size: " << UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params.OutActorsInGridCell.NeighboringCenterCellInfos.Num() << '\n';

	static auto BuildingTypeOffset = GetOffset(BuildingActor, "BuildingType");
	auto BuildingType = *(EFortBuildingType*)(__int64(BuildingActor) + BuildingTypeOffset);

	auto MainLocation = Helper::GetActorLocation(BuildingActor);
	auto MainRot = Helper::GetActorRotation(BuildingActor);
	auto MainCellIdx = Helper::GetCellIndexFromLocation(MainLocation);

	auto HandleNeighbor = [](UObject* NeighboringActor) -> bool {

	};

	auto NewBuildingType = *(EFortBuildingType*)(__int64(BuildingActor) + BuildingTypeOffset);

	if (NewBuildingType == EFortBuildingType::Wall)
	{
		auto& WallInfos = UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params.OutActorsInGridCell.NeighboringWallInfos;
		for (int i = 0; i < WallInfos.Num(); i++)
		{
			auto& CurrentWallInfo = WallInfos.At(i);

			auto WallActor = CurrentWallInfo.NeighboringActor.Get();

			if (CurrentWallInfo.NeighboringCellIdx == MainCellIdx)
			{
				// auto bSameRotation = CurrentWallInfo.WallPosition == BuildingActor->WallPosition;
				auto bSameRotation = Helper::GetActorRotation(WallActor) == MainRot;

				if (bSameRotation)
					return false;
			}
		}
	}
	else if (NewBuildingType == EFortBuildingType::Floor) // depending on the like rotation or idfk it osmetiems double builds
	{
		auto& FloorInfos = UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params.OutActorsInGridCell.NeighboringFloorInfos;
		for (int i = 0; i < FloorInfos.Num(); i++)
		{
			auto& CurrentFloorInfo = FloorInfos.At(i);

			auto FloorActor = CurrentFloorInfo.NeighboringActor.Get();

			if (CurrentFloorInfo.NeighboringCellIdx == MainCellIdx)
			{
				// auto bSameRotation = CurrentWallInfo.WallPosition == BuildingActor->WallPosition;
				auto bSameRotation = Helper::GetActorRotation(FloorActor) == MainRot;

				if (bSameRotation)
					return false;
			}
		}
	}
	else // if (NewBuildingType == EFortBuildingType::GenericCenterCellActor)
	{
		bool bCanBuild = true;

		for (int i = 0; i < ExistingBuildings.size(); i++) // (const auto Building : ExistingBuildings)
		{
			auto Building = ExistingBuildings[i];

			if (!Building)
				continue;

			// TODO: Test the code below!

			// if (Building->Member<IsDestroyedBitField>(("bDestroyed"))->bDestroyed)
				// sExistingBuildings.erase(ExistingBuildings.begin() + i);

			if ((Helper::GetActorLocation(Building) == Helper::GetActorLocation(BuildingActor)) &&
				(*(EFortBuildingType*)(__int64(Building) + BuildingTypeOffset) == BuildingType))
			{
				bCanBuild = false;
			}
		}

		if (bCanBuild || ExistingBuildings.size() == 0)
		{
			ExistingBuildings.push_back(BuildingActor);

			return true;
		}

		return false;

		/* auto& CenterCellInfos = UBuildingStructuralSupportSystem_K2_GetBuildingActorsInGridCell_Params.OutActorsInGridCell.NeighboringCenterCellInfos;
		for (int i = 0; i < CenterCellInfos.Num(); i++)
		{
			auto& CurrentCenterInfo = CenterCellInfos.At(i);

			if (CurrentCenterInfo.NeighboringCellIdx == MainCellIdx)
				return false;
		} */
	}

	return true;
}

inline bool ServerCreateBuildingActorHook(UObject* Controller, UFunction* Function, void* Parameters)
{
	if (Controller && Parameters)
	{
		static auto WoodItemData = FindObject(("FortResourceItemDefinition /Game/Items/ResourcePickups/WoodItemData.WoodItemData"));
		static auto StoneItemData = FindObject(("FortResourceItemDefinition /Game/Items/ResourcePickups/StoneItemData.StoneItemData"));
		static auto MetalItemData = FindObject(("FortResourceItemDefinition /Game/Items/ResourcePickups/MetalItemData.MetalItemData"));

		bool bSuccessful = false;
		UObject* MatDefinition = nullptr;

		if (FnVerDouble > 8)
		{
			struct FCreateBuildingActorData
			{
				uint32_t                                           BuildingClassHandle;                                      // 0x0000(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
				struct FVector				                       BuildLoc;                                                 // 0x0004(0x000C) (Transient)
				struct FRotator                                    BuildRot;                                                 // 0x0010(0x000C) (ZeroConstructor, Transient, IsPlainOldData)
				bool                                               bMirrored;                                                // 0x001C(0x0001) (ZeroConstructor, Transient, IsPlainOldData)
				unsigned char                                      UnknownData00[0x3];                                       // 0x001D(0x0003) MISSED OFFSET
				float                                              SyncKey;                                                  // 0x0020(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
				unsigned char                                      UnknownData01[0x4];                                       // 0x0024(0x0004) MISSED OFFSET
				// struct FBuildingClassData                          BuildingClassData;                                        // 0x0028(0x0010) (Transient)
				char pad[0x10];
			};

			struct SCBAParams { FCreateBuildingActorData CreateBuildingData; };
			auto Params = (SCBAParams*)Parameters;
			{
				auto CreateBuildingData = Params->CreateBuildingData;

				static auto BuildLocOffset = FindOffsetStruct(("ScriptStruct /Script/FortniteGame.CreateBuildingActorData"), ("BuildLoc"));
				static auto BuildRotOffset = FindOffsetStruct(("ScriptStruct /Script/FortniteGame.CreateBuildingActorData"), ("BuildRot"));
				static auto BuildingClassDataOffset = FindOffsetStruct(("ScriptStruct /Script/FortniteGame.CreateBuildingActorData"), ("bMirrored"));
				static auto bMirroredOffset = FindOffsetStruct(("ScriptStruct /Script/FortniteGame.CreateBuildingActorData"), ("BuildingClassData"));

				static auto BuildingClassOffset = FindOffsetStruct(("ScriptStruct /Script/FortniteGame.BuildingClassData"), ("BuildingClass"));

				auto Pawn = Helper::GetPawnFromController(Controller);

				// auto BuildingClassData = Get<__int64>(BuildingClassDataOffset, CreateBuildingData);

				// if (BuildingClassData)
				{
					// auto BuildingClass = Get<UObject*>(BuildingClassOffset, *BuildingClassData);
					static auto BroadcastRemoteClientInfoOffset = GetOffset(Controller, "BroadcastRemoteClientInfo");
					auto RemoteClientInfo = (UObject**)(__int64(Controller) + BroadcastRemoteClientInfoOffset);

					if (RemoteClientInfo && *RemoteClientInfo)
					{
						// auto BuildingClass = Controller->Member<UObject*>(("CurrentBuildableClass"));
						static auto RemoteBuildableClassOffset = GetOffset(*RemoteClientInfo, "RemoteBuildableClass");
						auto BuildingClass = (UObject**)(__int64(*RemoteClientInfo) + RemoteBuildableClassOffset);

						// std::cout << ("BuildLocation Offset: ") << BuildLocOffset << '\n';
						auto BuildLoc = Params->CreateBuildingData.BuildLoc; // (FVector*)(__int64(Params->CreateBuildingData) + BuildLocOffset)// Get<FVector>(BuildLocOffset, Params->CreateBuildingData);
						auto BuildRot = Params->CreateBuildingData.BuildRot; // Get<FRotator>(BuildRotOffset, Params->CreateBuildingData);
						// auto bMirrored = Get<bool>(bMirroredOffset, Params->CreateBuildingData);

						/*
						std::cout << ("BuildingClassOffset: ") << BuildingClassOffset << '\n';
						std::cout << ("BuildingClassData: ") << BuildingClassDataOffset << '\n';
						std::cout << ("BuildingClass: ") << BuildingClass << '\n';
						*/

						if (BuildingClass && *BuildingClass)
						{
							auto BuildingClassName = (*BuildingClass)->GetFullName();

							// TODO: figure out a better way

							if (BuildingClassName.contains(("W1")))
								MatDefinition = WoodItemData;
							else if (BuildingClassName.contains(("S1")))
								MatDefinition = StoneItemData;
							else if (BuildingClassName.contains(("M1")))
								MatDefinition = MetalItemData;

							if (MatDefinition)
							{
								auto MatInstance = Inventory::FindItemInInventory(Controller, MatDefinition);

								if (*FFortItemEntry::GetCount(GetItemEntryFromInstance(MatInstance)) < 10)
									return false;

								// std::cout << ("Goofy class: ") << (*BuildingClass)->GetFullName();
								UObject* BuildingActor = Easy::SpawnActor(*BuildingClass, BuildLoc, BuildRot); // Helper::GetActorLocation(Pawn), Helper::GetActorRotation(Pawn));

								if (BuildingActor)
								{
									if (CanBuild(BuildingActor))
									{
										if (bDoubleBuildFix)
											ExistingBuildings.push_back(BuildingActor);

										Helper::InitializeBuildingActor(Controller, BuildingActor, true);

										bSuccessful = true;

										// if (!Helper::IsStructurallySupported(BuildingActor))
											// bSuccessful = false;
									}
									else
										bSuccessful = false;

									if (!bSuccessful)
									{
										Helper::SetActorScale3D(BuildingActor, {});
										Helper::SilentDie(BuildingActor);
									}
								}
							}
						}
						else
							std::cout << ("Unable to get BuildingClass!\n");
					}
					else
						std::cout << ("Unable to get RemoteClientInfo!\n");
				}
			}
		}
		else
		{
			struct FBuildingClassData {
				UObject* BuildingClass;
				int                                                PreviousBuildingLevel;                                    // 0x0008(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
				int                                                UpgradeLevel;
			};

			struct SCBAParams {
				FBuildingClassData BuildingClassData; // FBuildingClassData&
				FVector BuildLoc;
				FRotator BuildRot;
				bool bMirrored;
				float SyncKey; // does this exist below 7.4
			};

			auto Params = (SCBAParams*)Parameters;

			// static auto BuildingClassOffset = FindOffsetStruct(("ScriptStruct /Script/FortniteGame.BuildingClassData"), ("BuildingClass"));

			// auto BuildingClass = Get<UObject*>(BuildingClassOffset, __int64(Params->BuildingClassData));
			
			// auto RemoteClientInfo = Controller->Member<UObject*>(("BroadcastRemoteClientInfo"));

			// if (RemoteClientInfo && *RemoteClientInfo)
			// if (false)
			{
				// auto BuildingClass = *Controller->Member<UObject*>(("CurrentBuildableClass"));
				// auto BuildingClass = (*RemoteClientInfo)->Member<UObject*>(("RemoteBuildableClass"));

				auto BuildingClass = Params->BuildingClassData.BuildingClass;

				if (BuildingClass) // && *BuildingClass)
				{
					auto BuildingClassName = BuildingClass->GetFullName();

					// TODO:" figure out a better way

					if (BuildingClassName.contains(("W1")))
						MatDefinition = WoodItemData;
					else if (BuildingClassName.contains(("S1")))
						MatDefinition = StoneItemData;
					else if (BuildingClassName.contains(("M1")))
						MatDefinition = MetalItemData;

					if (MatDefinition)
					{
						auto MatInstance = Inventory::FindItemInInventory(Controller, MatDefinition);

						if (*FFortItemEntry::GetCount(GetItemEntryFromInstance(MatInstance)) < 10)
							return false;

						UObject* BuildingActor = Easy::SpawnActor(BuildingClass, Params->BuildLoc, Params->BuildRot); // Helper::GetActorLocation(Pawn), Helper::GetActorRotation(Pawn));

						if (BuildingActor)
						{
							if (CanBuild(BuildingActor))
							{
								ExistingBuildings.push_back(BuildingActor);
								Helper::InitializeBuildingActor(Controller, BuildingActor, true);

								bSuccessful = true;

								// if (!Helper::IsStructurallySupported(BuildingActor))
									// bSuccessful = false;
							}
							else
								bSuccessful = false;

							if (!bSuccessful)
							{
								Helper::SetActorScale3D(BuildingActor, {});
								Helper::SilentDie(BuildingActor);
							}
						}
						else
							std::cout << ("Unable to summon the building!\n");
					}
				}
				else
					std::cout << ("No BuildingClass!\n");
			}
		}

		if (bSuccessful && !bIsPlayground)
		{
			// TEnumAsByte<EFortResourceType>                     ResourceType;
			// auto ResourceType = *BuildingActor->Member<TEnumAsByte<EFortResourceType>>(("ResourceType"));

			if (MatDefinition)
				Inventory::DecreaseItemCount(Controller, MatDefinition, 10);
			else
				std::cout << ("Is bro using permanite!?!?!?");
		}
		// else
			// std::cout << ("failed to build!\n");
	}

	return false;
}

inline bool ServerBeginEditingBuildingActorHook(UObject* Controller, UFunction* Function, void* Parameters)
{
	struct Parms { UObject* BuildingActor; };
	static UObject* EditToolDefinition = FindObject(("FortEditToolItemDefinition /Game/Items/Weapons/BuildingTools/EditTool.EditTool"));

	auto EditToolInstance = Inventory::FindItemInInventory(Controller, EditToolDefinition);

	auto Pawn = *Controller->Member<UObject*>(("Pawn"));

	UObject* BuildingToEdit = ((Parms*)Parameters)->BuildingActor;

	if (Controller && BuildingToEdit)
	{
		if (EditToolInstance)
		{
			auto EditTool = Inventory::EquipWeaponDefinition(Pawn, EditToolDefinition, Inventory::GetItemGuid(EditToolInstance));

			if (EditTool)
			{
				auto PlayerState = *Controller->Member<UObject*>(("PlayerState"));
				*BuildingToEdit->Member<UObject*>(("EditingPlayer")) = PlayerState;
				static auto OnRep_EditingPlayer = BuildingToEdit->Function(("OnRep_EditingPlayer"));

				if (OnRep_EditingPlayer)
					BuildingToEdit->ProcessEvent(OnRep_EditingPlayer);

				*EditTool->Member<UObject*>(("EditActor")) = BuildingToEdit;
				static auto OnRep_EditActor = EditTool->Function(("OnRep_EditActor"));

				if (OnRep_EditActor)
					EditTool->ProcessEvent(OnRep_EditActor);
			}
		}
		else
			std::cout << ("No Edit Tool Instance?\n");
	}

	return false;
}

inline bool ServerEditBuildingActorHook(UObject* Controller, UFunction* Function, void* Parameters)
{
	struct Parms {
		UObject* BuildingActorToEdit;                                      // (Parm, ZeroConstructor, IsPlainOldData)
		UObject* NewBuildingClass;                                         // (Parm, ZeroConstructor, IsPlainOldData)
		int                                                RotationIterations;                                       // (Parm, ZeroConstructor, IsPlainOldData)
		bool                                               bMirrored;                                                // (Parm, ZeroConstructor, IsPlainOldData)
	};

	auto Params = (Parms*)Parameters;

	if (Params && Controller)
	{
		auto BuildingActor = Params->BuildingActorToEdit;
		auto NewBuildingClass = Params->NewBuildingClass;

		static auto bMirroredOffset = FindOffsetStruct("Function /Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", "bMirrored");
		auto bMirrored = *(bool*)(__int64(Parameters) + bMirroredOffset);

		static auto RotationIterationsOffset = FindOffsetStruct("Function /Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", "RotationIterations");
		auto RotationIterations = *(int*)(__int64(Parameters) + RotationIterationsOffset);

		if (BuildingActor && NewBuildingClass)
		{
			IsDestroyedBitField* BitField = Params->BuildingActorToEdit->Member<IsDestroyedBitField>(("bDestroyed"));

			if (!BitField || BitField->bDestroyed || RotationIterations > 3) 
				return false;

			auto Location = Helper::GetActorLocation(BuildingActor);
			auto Rotation = Helper::GetActorRotation(BuildingActor);

			// class UBuildingEditModeMetadata*             EditModePatternData;

			/* auto EditModeMetaData = BuildingActor->Member<UObject*>("EditModePatternData");

			if (EditModeMetaData && *EditModeMetaData)
			{
				std::cout << "Bruh: " << *(int*)(__int64(*EditModeMetaData) + 72) << '\n';;

				if ((*EditModeMetaData)->GetFullName().contains("_Stair"))
				{
					auto TileData = (*EditModeMetaData)->Member<TArray<int32_t>>("TileData");
					std::cout << "TileData Num: " << TileData->Num() << '\n';

					// *(int32_t*)(__int64(TileData) + 8) = (RotationIterations * RotationIterations);

					// TileData->Add(RotationIterations);

					RotationIterations = sqrt(TileData->Num());
				}

				*(int*)(__int64(*EditModeMetaData) + 72) = RotationIterations;

				__int64 (*ahh)(UObject * EditMetaData) = decltype(ahh)((*EditModeMetaData)->VFTable[0x49]); // rotate

				std::cout << "Ahh: " << ahh(*EditModeMetaData) << '\n';;

				__int64 (*ahh2)(UObject* EditMetaData) = decltype(ahh)((*EditModeMetaData)->VFTable[0x4A]); // mirror

				std::cout << "Ahh2: " << ahh2(*EditModeMetaData) << '\n';
			}
			else
				std::cout << "Invalid metadata!\n"; */

			auto BuildingSMActorReplaceBuildingActorAddr = FindPattern("4C 8B DC 55 57 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B 85 ? ? ? ? 33 FF 40 38 3D ? ? ? ?");

			if (Engine_Version >= 426 && !BuildingSMActorReplaceBuildingActorAddr)
				BuildingSMActorReplaceBuildingActorAddr = FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 44 0F 29 40 ? 44 0F 29 48 ? 44 0F 29 90 ? ? ? ? 44 0F 29 B8 ? ? ? ? 48 8B 05");

			if (!BuildingSMActorReplaceBuildingActorAddr || Engine_Version <= 421)
				BuildingSMActorReplaceBuildingActorAddr = FindPattern("48 8B C4 44 89 48 20 55 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 89 70 E8 33 FF 40 38 3D ? ? ? ? 48 8B F1 4C 89 60 E0 44 8B E2");

			UObject* (__fastcall* BuildingSMActorReplaceBuildingActor)(UObject* BuildingSMActor, unsigned int a2, UObject* a3, unsigned int a4, int a5, unsigned __int8 bMirrored, UObject* Controller);

			BuildingSMActorReplaceBuildingActor = decltype(BuildingSMActorReplaceBuildingActor)(BuildingSMActorReplaceBuildingActorAddr);

			if (BuildingSMActorReplaceBuildingActor)
				BuildingSMActorReplaceBuildingActor(BuildingActor, 1, NewBuildingClass, 0, RotationIterations, bMirrored, Controller);
			else
				std::cout << "No BuildingSMActorReplaceBuildingActor!\n";
		}
	}

	return false;
}

inline bool ServerEndEditingBuildingActorHook(UObject* Controller, UFunction* Function, void* Parameters)
{
	if (Controller && Parameters && !Helper::IsInAircraft(Controller))
	{
		// TODO: Check if the controller is in aircraft, if they edit on spawn island, it will make them end on the battle bus, which will not go well.

		struct Parms {
			UObject* BuildingActorToStopEditing;
		};

		auto Params = (Parms*)Parameters;

		auto Pawn = Controller->Member<UObject*>(("Pawn"));

		if (Pawn && *Pawn)
		{
			auto CurrentWep = (*Pawn)->Member<UObject*>(("CurrentWeapon"));

			if (CurrentWep && *CurrentWep)
			{
				auto CurrentWepItemDef = *(*CurrentWep)->Member<UObject*>(("WeaponData"));
				static UObject* EditToolDefinition = FindObject(("FortEditToolItemDefinition /Game/Items/Weapons/BuildingTools/EditTool.EditTool"));

				if (CurrentWepItemDef == EditToolDefinition) // Player CONFIRMED the edit
				{
					// auto EditToolInstance = Inventory::FindItemInInventory(Controller, EditToolDefinition);
					auto EditTool = *CurrentWep;// Inventory::EquipWeaponDefinition(*Pawn, EditToolDefinition, Inventory::GetItemGuid(EditToolInstance));

					*EditTool->Member<bool>(("bEditConfirmed")) = true;
					*EditTool->Member<UObject*>(("EditActor")) = nullptr;
					static auto OnRep_EditActorFn = EditTool->Function(("OnRep_EditActor"));

					if (OnRep_EditActorFn)
						EditTool->ProcessEvent(OnRep_EditActorFn);
				}
			}

			if (Params->BuildingActorToStopEditing)
			{
				*Params->BuildingActorToStopEditing->Member<UObject*>(("EditingPlayer")) = nullptr;
				static auto OnRep_EditingPlayer = Params->BuildingActorToStopEditing->Function(("OnRep_EditingPlayer"));

				if (OnRep_EditingPlayer)
					Params->BuildingActorToStopEditing->ProcessEvent(OnRep_EditingPlayer);
			}
		}
	}

	return false;
}

void InitializeBuildHooks()
{
	// if (Engine_Version < 426)
	{
		AddHook(("Function /Script/FortniteGame.FortPlayerController.ServerCreateBuildingActor"), ServerCreateBuildingActorHook);

		// if (Engine_Version < 424)
		{
			AddHook(("Function /Script/FortniteGame.FortPlayerController.ServerBeginEditingBuildingActor"), ServerBeginEditingBuildingActorHook);
			AddHook(("Function /Script/FortniteGame.FortPlayerController.ServerEditBuildingActor"), ServerEditBuildingActorHook);
			AddHook(("Function /Script/FortniteGame.FortPlayerController.ServerEndEditingBuildingActor"), ServerEndEditingBuildingActorHook);
		}
	}
}