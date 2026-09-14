<div align="center">

<img width="340" height="292" alt="Project Wither logo" src="https://github.com/user-attachments/assets/6c753aad-9eed-4885-bf0c-dbc27c7f0568" />

# Project: Wither

**근거리와 원거리 무기를 오가며 전투하는 3인칭 액션 게임**

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0E1128?style=flat-square&logo=unrealengine&logoColor=white)](https://www.unrealengine.com/)
[![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![License](https://img.shields.io/github/license/danger10043/Unreal10thTeamProject-ProjectWither?style=flat-square)](LICENSE)
[![Release](https://img.shields.io/badge/Release-v1.0.1-2ea44f?style=flat-square&logo=github)](https://github.com/danger10043/Unreal10thTeamProject-ProjectWither/releases/tag/V1.0.1)

</div>

## 소개

Project: Wither는 Unreal Engine 5로 제작한 팀 프로젝트입니다. 검과 총을 상황에 맞게 전환하는 전투, 다양한 몬스터와 보스, 아이템 파밍과 성장 시스템을 중심으로 구성했습니다. 핵심 로직은 C++ 컴포넌트로 모듈화하고, 콘텐츠와 연출은 Blueprint로 확장할 수 있도록 설계했습니다.

최신 배포 버전은 [Project: Wither V1.0.1](https://github.com/danger10043/Unreal10thTeamProject-ProjectWither/releases/tag/V1.0.1)에서 확인할 수 있습니다.

## 플레이 영상

<!--
YouTube 영상 업로드 후 아래 YOUR_VIDEO_ID 두 곳을 실제 영상 ID로 교체하고 주석을 해제하세요.

[![Project: Wither 플레이 영상](https://img.youtube.com/vi/YOUR_VIDEO_ID/maxresdefault.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)
-->

> 플레이 영상은 추후 공개 예정입니다.

## 주요 기능

| 시스템 | 내용 |
| --- | --- |
| 전투 | 근접 콤보, 방어, 회피, 원거리 사격 및 재장전 |
| 무기 | 검과 총 전환, 무기·방어구 장착, 탄약 타입 관리 |
| 플레이어 | 이동, 달리기, 카메라 줌, 타깃 락온, 포션 사용 |
| 몬스터 | AI Controller 기반 행동, 스폰 존, 투사체 공격, 오브젝트 풀링 |
| 보스 | 전투 상태와 이동 페이즈, 보스 전용 체력 UI |
| 성장 | 스탯 확인 및 강화, 인벤토리, 아이템 획득과 소비 |
| 상호작용 | NPC 대화, 대장간 제작, 세이브 포인트와 리스폰 |
| UI | HUD, 조준점, 탄약·포션 수량, 인벤토리, 로비 및 각종 메뉴 |

## 기술 스택

- Unreal Engine 5.8
- C++ / Blueprint
- Enhanced Input
- UMG / Slate
- Unreal AI Module / Navigation System
- Niagara

## 시작하기

### 요구 사항

- Unreal Engine 5.8
- Visual Studio 2022 및 **Game development with C++** 워크로드

### 실행

```bash
git clone https://github.com/danger10043/Unreal10thTeamProject-ProjectWither.git
cd Unreal10thTeamProject-ProjectWither
```

1. `ProjectWither.uproject`를 우클릭하고 **Generate Visual Studio project files**를 선택합니다.
2. 생성된 솔루션을 열어 `Development Editor` / `Win64` 구성으로 빌드합니다.
3. `ProjectWither.uproject`를 Unreal Editor 5.8로 엽니다.
4. 시작 레벨인 `Content/Main/Levels/LV_Lobby`에서 플레이합니다.

> 프로젝트가 열리지 않으면 설치된 엔진 버전이 5.8인지 먼저 확인해 주세요.

## 프로젝트 구조

```text
ProjectWither/
├─ Config/                         # 프로젝트·입력·패키징 설정
├─ Content/
│  └─ Main/                        # 레벨, Blueprint, UI, VFX, 데이터 에셋
├─ Source/ProjectWither/
│  ├─ Public/                      # 공개 헤더 및 인터페이스
│  └─ Private/                     # 시스템 구현
├─ Plugins/                        # 프로젝트 플러그인
└─ ProjectWither.uproject
```

주요 C++ 영역은 `Player`, `Component`, `Equipment`, `Monster`, `BossMonster`, `NPC`, `Item`, `Widget`, `World`로 나뉩니다.

## 개발 참여

이 저장소의 브랜치와 커밋 규칙을 확인한 뒤 기능 브랜치를 만들어 작업해 주세요. 버그 제보와 개선 제안은 [Issues](https://github.com/danger10043/Unreal10thTeamProject-ProjectWither/issues)에 남길 수 있습니다.

기여 내역은 [Contributors](https://github.com/danger10043/Unreal10thTeamProject-ProjectWither/graphs/contributors)에서 확인할 수 있습니다.

## 라이선스

이 프로젝트는 [MIT License](LICENSE)를 따릅니다.
