// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 创建Widget后调用这个函数来设置属性，尤其是LobbyPath，是加入/创建Session后进入的地图
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 10, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:
	virtual bool Initialize() override;

	// 关卡被移除时调用
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

//-----------------------------------------Callbacks--------------------------------------------------------------------
	// 绑定到MultiplayerSessionsSubsystem的回调中
	UFUNCTION()
	void OnCreateSession(bool bIsSuccseeful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bIsSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bIsSuccseeful);
	UFUNCTION()
	void OnStartSession(bool bIsSuccseeful);

private:
//-----------------------------------------Widget--------------------------------------------------------------------
	// 变量会与蓝图中的变量绑定，前提是名字一样
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	// 在这里调用创建会话的接口
	UFUNCTION()
	void HostButtonClicked();
	
	// 在这里调用加入会话的接口
	UFUNCTION()
	void JoinButtonClicked();

//-----------------------------------------Functions--------------------------------------------------------------------
	// 从UI模式进入游戏模式, 在OnLevelRemovedFromWorld中调用
	void MeunTeraDown();

//-----------------------------------------Parameters--------------------------------------------------------------------
	// The subsystem design to handle all online function
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	
	int32 NumPublicConnections{ 10 };

	// 选择同样MatchType的玩家会进入同一个游戏中,这个字符串只是搜索Session用的，并不代表实际的游戏模式
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("") };
};
