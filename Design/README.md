# 新エンジン設計メモ

`ClassDiagram.drawio`（draw.io、11 ページ）の設計で決めたことと、その理由をまとめる。
ProjectBeast をもとに、エンジンとゲームをきちんと分けて作り直す。

参考にしたもの:

- ProjectBeast（このリポジトリ）
- Test（TatsuyukiSugahara/Test の aqEngine）
- FANGS-OF-RAGNAROK-ecs（Xbox 向けに途中まで作ったもの。入力・遮蔽回避・当たり判定）

## 前提

| 項目 | 内容 |
|---|---|
| ジャンル | Elden Ring のようなソウルライク |
| 対象 | Xbox One S。UWP Dev Mode（道A）で「Game」分類にしてデプロイする |
| 制約 | CPU は 4 コア占有 + 2 コア共有（Jaguar 1.75GHz）、RAM 5GB、D3D12 FL12_0 |
| マルチプレイ | 入れるかもしれない。入れるならローカル（画面分割） |
| ECS | 使わない。IGameObject の継承方式 |
| アロケーター | 自前では管理しない（`new` と `CreateCommittedResource` をそのまま使う） |

## 1. GameObject と進行（図の 2 ページ目）

- `Application`（エンジン）が `vector<unique_ptr<IGameObject>>` で所有する。
  - 生成は `CreateObject<T>()`。既定でアクティブレベルに所属し、`Persistent` を付けると常駐する。
  - 破棄は `RequestDestroy` で予約し、フレームの最後にまとめて行う。
- ライフサイクルは Unity と同じ名前にそろえる。
  - `Awake(params)`：生成直後に呼ぶ。**Load Thread やジョブから呼ばれることがあるので、自分のことだけをする**（他のオブジェクトの検索や生成、シングルトンの書き換えは禁止）。
  - `Start`：Main で、登録された次のフレームの最初の Update の直前に 1 回呼ぶ。他のオブジェクトとの連携はここで行う。
  - `Update` / `OnDestroy`：Main で呼ぶ。
- 更新順は `UpdateGroup`（PreUpdate / Gameplay / ParallelGameplay / PostPhysics / Camera / Late）で決める。どのグループかはクラス側が決め、同じグループの中は登録順。
- `ParallelGameplay` は `JobSystem::ParallelFor` で分割して実行する。
  - 書き換えてよいのは自分だけ。
  - 読んでよいのは、前のグループで確定した値だけ。
  - `CreateObject` と `RequestDestroy` はキューに予約するだけ。
- フレームをまたいで覚える参照は `ObjectRef<T>`（ID と世代番号）を使う。
  - 破棄済みなら `Get()` が `nullptr` を返す。
  - 実際の破棄はフレームの最後の Main だけなので、ジョブの中で `Get()` しても安全。
- 進行はエンジンの `GameStateMachine` と `IGameState`（予約制で遷移）。中身の状態（Title / Loading / InGame / Result）はゲームが作る。

## 2. レベルとロード（3 ページ目）

- シーン方式ではなくレベル方式にする。同じ世界のまま進行や画面だけを変えたいため。
- `LevelManager` が `.level.json` を読み、`GameObjectRegistry`（型名から生成関数を引く表）でオブジェクトを作る。
- JSON のオブジェクトは `type` / `name` / `transform` / `params`。`params` は `Awake` に渡す。
- 実行中に生成する物のリソースは、JSON の `preload` に書いて先読みする。
- ロードの流れ：IO スレッドでファイルを読む → ジョブでパース・生成・Awake → Main の同期点で登録する。

## 3. スレッド（6 ページ目）

- 専用スレッドは Main（Core0）、Render（Core1）、IO（共有コア）の 3 本だけ。
- それ以外の並列処理はすべて `JobSystem`（`Schedule` / `ParallelFor` / `Wait`）で行う。ワーカーは 4 本で、コアに固定する。

## 4. 描画（4 ページ目）

- 案C：ゲーム側は `RenderContext` に触らない。コマンドを積み、`RenderThread` が実行する。
- `RenderScene` は保持方式にし、全ビューで共有する。
  - ModelRender は Start でプロキシを登録し、動いたときだけ `UpdateTransform` を呼ぶ。
  - 行列は `SceneBuffer<T>` のダブルバッファで持つ。入れ替えた直後に、直前フレームの dirty 分を書く側へコピーする。
- ワールド行列の転送とスキニングは、1 フレームに 1 回だけ行う。
- `RenderView` はビューごとに持つ。中身はカメラ、描画先、ビューポート、パイプライン、影（Own / ShareMain / None）、オクルージョン、レイヤーマスク、更新頻度、カリング結果。
- 2 つ目のカメラの用途：キャラクターの 3D プレビュー（`Preview` レイヤー、RenderTexture）と、画面分割。

## 5. UI（5 ページ目）

- IGameObject とは別系統にする：`UIContext` / `UIScreen` / `UIObject`。
- UIObject はコンポーネント式。Transform は必須、描画系は 1 つまで、振る舞い系は任意。
- レイアウトは JSON で持つ。
- ゲームから UI へは Push 型（`Screens().Find<T>()->SetXxx()`）。UI はゲームオブジェクトを参照しない。
- UI 表示中はマウスカーソルを出し、クリックで操作できる。

## 6. リソース（7 ページ目）

- `Load<T>` はどのスレッドからでも呼べ、`ResourceHandle<T>` を返す。
- Decode と CreateGpu はジョブで行い、Copy Queue で転送する。Fence の完了で Ready になる。
- 依存しているリソースがすべて Ready になって初めて、親も Ready になる。
- レベルを Unload した後に `UnloadUnused` を呼ぶ。GPU リソースは Fence を待ってから遅延解放する。

## 7. 入力と操作（8・9 ページ目）

- エンジンは仮想パッド（`ControlState`）までを担当する。
  - 短押し、長押し、押した時刻（先行入力用）を判定する。
- キーボードとマウスは、JSON でパッドのボタン名に割り当てる。
  - 修飾キー付きの割り当てを優先する。
  - カメラの既定はマウス。矢印キーでのカメラは `KeyboardOnly` プロファイルで選ぶ。
- パッドは 1 台（ローカルマルチを入れるなら `LocalPlayer` ごとに増やす）。振動も扱う。
- キャラクターは入力を直接読まない。
  - `ICharacterController`（Player / AI）が、ゲーム定義の `CharacterCommand`（意図）を作る。
  - Character のステートは `CharacterCommand` だけを読む。
  - プレイヤーとエネミーで同じコードになる。

## 8. カメラ（10 ページ目）

- `CameraManager` はカメラ枠ごとに、モードの切り替えと補間、演出（Modifier）、遮蔽物の回避を行う。
- モードは FreeLook / LockOn / Event / Debug。
- ロックオンは役割を分ける。
  - 状態（誰を狙うか）：Character
  - 選択：LockOnSystem（視点を引数で受け取るので AI も使える）
  - 演出：Camera と UI
- `LocalPlayer`：ローカルマルチの継ぎ目。今は 1 人分だけ作る。

## 9. 物理（11 ページ目）

- Jolt Physics を使う。ヘッダーに Jolt の型は出さない。
- 固定タイムステップは 60Hz、1 フレームの上限は 2 ステップ、描画側で補間する。
- クエリは const。ステップ中以外はジョブから同時に呼べる。
- 本体の種類は Static / Kinematic / Dynamic / Trigger。剛体の力学とラグドールも使う。
- キャラクターは `CharacterMover`（CharacterVirtual を包む）で動かす。
- 攻撃の当たり判定は、武器のカプセルを前のフレームから今のフレームまで掃引して行う。

## 未決・次に決めること

- アニメーション（ブレンド、ルートモーション、攻撃判定を有効にするイベント）
- サウンド、エフェクト、セーブ
- 画面分割を入れる場合の、パッドの割り当てと HUD の配置
