import os
import json
import struct

# ── struct.pack 書式指定子 チートシート ──────────────────────────
#  指定子    C++側の型         バイト数   説明
#  --------  ---------------  -------  --------------------------
#  i         int              4        符号付き整数
#  I         unsigned int     4        符号なし整数
#  f         float            4        浮動小数点数
#  s         char             1        文字（バイト列） ※ 4s なら char[4]
#  16s       char[16]         16       16バイト固定長文字列
#  <         —                —        先頭に必ず付ける（リトルエンディアン）
# ─────────────────────────────────────────────────────────────────
#
# 【書き方の例】
#
#   ▼ int と float だけの場合（→ pack_daddy_penguin 参照）
#   # "<iiffffffffi" = i(4)×2 + f(4)×8 + i(4)×1 = 44 bytes
#   struct.pack("<iiffffffffi",
#       int(data.get("maxHp", 0)),          # i
#       int(data.get("hp", 0)),             # i
#       float(data.get("walkSpeed", 0.0)),  # f
#       ...                                 # f × 7
#       int(data.get("enableCommandRange", 0))  # i
#   )
#
#   ▼ int と float が途中で切り替わる場合（→ pack_enemy_parameter 参照）
#   # "<iifffffifff" = i(4)×2 + f(4)×5 + i(4)×1 + f(4)×3 = 44 bytes
#   struct.pack("<iifffffifff",
#       int(...),  int(...),                # i × 2
#       float(...), ..., float(...),        # f × 5
#       int(data.get("maxEat", 0)),         # i  ← 途中で int が来てもOK
#       float(...), float(...), float(...), # f × 3
#   )
#
#   ▼ 文字列を含む場合（→ CPReaction の name フィールドなど）
#   # "<16sii" = 16s(16) + f(4)×2 = 24 bytes
#   name = data.get("name", "").encode("utf-8")[:16].ljust(16, b"\x00")
#   struct.pack("<16sff",
#       name,                               # 16s（16バイト固定長）
#       float(data.get("width",  0)),         # f
#       float(data.get("height", 0)),         # f
#   )
#
# ─────────────────────────────────────────────────────────────────

def pack_daddy_penguin(data):
    # < : リトルエンディアン
    # i : int (4 bytes) × 2
    # f : float (4 bytes) × 8
    # i : int (4 bytes) × 1
    # 計 44 bytes
    return struct.pack(
        "<iiffffffffi",
        int(data.get("maxHp", 0)),
        int(data.get("hp", 0)),
        float(data.get("walkSpeed", 0.0)),
        float(data.get("runSpeed", 0.0)),
        float(data.get("swimSpeed", 0.0)),
        float(data.get("sneakSpeed", 0.0)),
        float(data.get("slideSpeed", 0.0)),
        float(data.get("jumpPower", 0.0)),
        float(data.get("radius", 0.0)),
        float(data.get("height", 0.0)),
        int(data.get("enableCommandRange", 0))
    )

def pack_penguin_effect_parameter(data):
    # 全フィールドが float (get<float>()) のみ
    # Vector3 フィールドは x/y/z それぞれ float × 3 に展開する
    # 合計 23 × float = 92 bytes

    def v3(key):
        """Vector3フィールドを辞書で取得（デフォルト {x:0,y:0,z:0}）"""
        d = data.get(key, {"x": 0.0, "y": 0.0, "z": 0.0})
        return float(d.get("x", 0.0)), float(d.get("y", 0.0)), float(d.get("z", 0.0))

    sx, sy, sz = v3("splashEffectScale")
    lx, ly, lz = v3("landingEffectScale")
    fx, fy, fz = v3("slideFrostEffectScale")
    wx, wy, wz = v3("slideLineEffectScale")

    return struct.pack(
        "<23f",
        sx, sy, sz,                                                  # fff : splashEffectScale (Vector3)
        float(data.get("effectOffsetForward",      0.0)),            # f   : effectOffsetForward
        float(data.get("splashEffectInterval",     0.0)),            # f   : splashEffectInterval
        float(data.get("minMoveVelocitySq",        0.0)),            # f   : minMoveVelocitySq
        float(data.get("minSplashScaleRatio",      0.0)),            # f   : minSplashScaleRatio
        float(data.get("maxSplashScaleRatio",      0.0)),            # f   : maxSplashScaleRatio
        float(data.get("minSpeed",                 0.0)),            # f   : minSpeed
        float(data.get("maxSpeed",                 0.0)),            # f   : maxSpeed
        lx, ly, lz,                                                  # fff : landingEffectScale (Vector3)
        fx, fy, fz,                                                  # fff : slideFrostEffectScale (Vector3)
        float(data.get("slideEffectInterval",      0.0)),            # f   : slideEffectInterval
        float(data.get("minSlideFrostScaleRatio",  0.0)),            # f   : minSlideFrostScaleRatio
        float(data.get("maxSlideFrostScaleRatio",  0.0)),            # f   : maxSlideFrostScaleRatio
        wx, wy, wz,                                                  # fff : slideLineEffectScale (Vector3)
        float(data.get("slideLineEffectOffsetForward", 0.0)),        # f   : slideLineEffectOffsetForward
    )


def pack_child_penguin_parameter(data):
    # type / maxHp / hp : get<int>()   → i
    # それ以外           : get<float>() → f
    # タイプ固有の任意フィールドは存在しない型では 0.0 で埋める
    # 合計: int×3 + float×33 = 144 bytes / エントリ（5エントリ分 = 720 bytes + count 4 bytes）

    r = data.get("randomRanges", {})
    color = data.get("color", [0.0, 0.0, 0.0, 1.0])

    def rmin(key): return float(r.get(key, [0.0, 0.0])[0])
    def rmax(key): return float(r.get(key, [0.0, 0.0])[1])

    return struct.pack(
        "<iii33f",
        int(data.get("type",   0)),                     # i  : type (0=まじめ 1=甘えん坊 2=やんちゃ 3=おっちょこちょい 4=世話焼き)
        int(data.get("maxHp",  0)),                     # i  : maxHp
        int(data.get("hp",     0)),                     # i  : hp
        float(data.get("radius", 0.0)),                 # f  : radius
        float(data.get("height", 0.0)),                 # f  : height
        float(color[0]), float(color[1]),               # ff : colorR, colorG
        float(color[2]), float(color[3]),               # ff : colorB, colorA
        rmin("runSpeed"),       rmax("runSpeed"),        # ff : runSpeed.min/max
        rmin("swimSpeed"),      rmax("swimSpeed"),       # ff : swimSpeed.min/max
        rmin("sneakSpeed"),     rmax("sneakSpeed"),      # ff : sneakSpeed.min/max
        rmin("slideSpeed"),     rmax("slideSpeed"),      # ff : slideSpeed.min/max
        rmin("jumpPower"),      rmax("jumpPower"),       # ff : jumpPower.min/max
        rmin("stopDistance"),   rmax("stopDistance"),    # ff : stopDistance.min/max
        rmin("walkDistance"),   rmax("walkDistance"),    # ff : walkDistance.min/max
        rmin("runDistance"),    rmax("runDistance"),     # ff : runDistance.min/max
        rmin("joinDistance"),   rmax("joinDistance"),    # ff : joinDistance.min/max
        rmin("giveUpDistance"), rmax("giveUpDistance"),  # ff : giveUpDistance.min/max
        rmin("roamTriggerDistance"), rmax("roamTriggerDistance"),  # ff : やんちゃのみ（他は 0.0）
        rmin("roamRadius"),     rmax("roamRadius"),      # ff : やんちゃのみ（他は 0.0）
        float(data.get("tripChancePerSec",   0.0)),      # f  : おっちょこちょいのみ（他は 0.0）
        float(data.get("slipChance",         0.0)),      # f  : おっちょこちょいのみ（他は 0.0）
        float(data.get("interventionRange",  0.0)),      # f  : 世話焼きのみ（他は 0.0）
    )

def pack_enemy_parameter(data):
    # i(2), f(5), i(1), f(3)  計44 bytes
    return struct.pack(
        "<iifffffifff",
        int(data.get("maxHp", 0)),
        int(data.get("hp", 0)),
        float(data.get("walkSpeed", 0.0)),
        float(data.get("runSpeed", 0.0)),
        float(data.get("radius", 0.0)),
        float(data.get("height", 0.0)),
        float(data.get("swimSpeed", 0.0)),
        int(data.get("maxEat", 0)),
        float(data.get("maxStamina", 0.0)),
        float(data.get("staminaDrainRate", 0.0)),
        float(data.get("lostChaseDistance", 0.0))
    )

def convert_enemy_layout(json_path, bin_path):
    """
    EnemyLayout 系 JSON 専用コンバーター
    可変長のパトロールポイントに対応するため、FILE_RULE_BOOK 経由で呼ばれる。

    バイナリ構造:
      [int:  エネミー数]
      per enemy:
        [float: spawnX, spawnY, spawnZ]   ← spawnPosition (Vector3)
        [char[32]: nestName]              ← 固定長文字列
        [int:  パトロール点数]
        [float: x, y, z] × n             ← patrolPoints (Vector3 × n)
    """
    try:
        with open(json_path, "r", encoding="utf-8-sig") as f:
            root = json.load(f)
    except Exception as e:
        print(f"  [エラー] 読み込み失敗: {e}")
        return

    enemies = root.get("enemies", [])
    buf = bytearray()

    # エネミー総数
    buf += struct.pack("<i", len(enemies))

    for enemy in enemies:
        # スポーン位置 (float × 3)
        pos = enemy.get("spawnPosition", [0, 0, 0])
        buf += struct.pack("<fff",
            float(pos[0]),  # f : spawnPosition.x (float)
            float(pos[1]),  # f : spawnPosition.y (float)
            float(pos[2]),  # f : spawnPosition.z (float)
        )

        # 巣の名前 (32バイト固定文字列)
        nest = enemy.get("nestName", "").encode("utf-8")[:32].ljust(32, b"\x00")
        buf += nest  # 32s

        # パトロールポイント（可変長）
        points = enemy.get("patrolPoints", [])
        buf += struct.pack("<i", len(points))  # i : パトロール点数

        for pt in points:
            buf += struct.pack("<fff",
                float(pt[0]),  # f : patrolPoint.x (float)
                float(pt[1]),  # f : patrolPoint.y (float)
                float(pt[2]),  # f : patrolPoint.z (float)
            )

    with open(bin_path, "wb") as f:
        f.write(buf)

    print(f"  -> [成功] {len(enemies)}体のエネミーデータを書き出しました。")

def pack_ocean_parameter(data):
    # 全フィールドが float のみ
    # "<8f" = float(4) × 8 = 32 bytes
    return struct.pack(
        "<8f",
        float(data.get("baseReflectance", 0.0)),  # f : baseReflectance（基本反射率）
        float(data.get("wave1Amplitude",  0.0)),  # f : wave1Amplitude（波①の振幅）
        float(data.get("wave1Frequency",  0.0)),  # f : wave1Frequency（波①の空間周波数）
        float(data.get("wave2Amplitude",  0.0)),  # f : wave2Amplitude（波②の振幅）
        float(data.get("wave2Frequency",  0.0)),  # f : wave2Frequency（波②の空間周波数）
        float(data.get("specularPower",   0.0)),  # f : specularPower（Phong指数）
        float(data.get("specularScale",   0.0)),  # f : specularScale（スペキュラ強度倍率）
        float(data.get("ambientScale",    0.0)),  # f : ambientScale（アンビエント強度倍率）
    )

def pack_pbr_parameter(data):
    # name     : char[32] = 32 bytes（固定長文字列）
    # 残4フィールド : float × 4   = 16 bytes
    # 合計 48 bytes
    name = data.get("name", "").encode("utf-8")[:32].ljust(32, b"\x00")
    return struct.pack(
        "<32s4f",
        name,                                           # 32s : name（オブジェクト識別名）
        float(data.get("dirLightScale",  0.0)),         # f   : dirLightScale（ディレクションライト強度倍率）
        float(data.get("ambientScale",   0.0)),         # f   : ambientScale（環境光強度倍率）
        float(data.get("metallicOffset", 0.0)),         # f   : metallicOffset（メタリックオフセット）
        float(data.get("smoothOffset",   0.0)),         # f   : smoothOffset（スムーズオフセット）
    )

def pack_whirlpool_parameter(data):
    # 全フィールドが float のみ
    # "<11f" = float(4) × 11 = 44 bytes
    return struct.pack(
        "<11f",
        float(data.get("whirlpoolRadius",      0.0)),  # f : whirlpoolRadius（影響範囲半径）
        float(data.get("attractSpeed",         0.0)),  # f : attractSpeed（引き寄せ速度）
        float(data.get("rotateSpeed",          0.0)),  # f : rotateSpeed（渦巻き回転速度 rad/s）
        float(data.get("uvRotationSpeed",      0.0)),  # f : uvRotationSpeed（UV回転速度 rad/s）
        float(data.get("scaleChangeTime",      0.0)),  # f : scaleChangeTime（拡大率変化時間）
        float(data.get("stayTime",             0.0)),  # f : stayTime（最大スケール維持時間）
        float(data.get("createInterval",       0.0)),  # f : createInterval（生成間隔）
        float(data.get("orbitRadius",          0.0)),  # f : orbitRadius（子ペンギン軌道半径）
        float(data.get("orbitRadiusVariation", 0.0)),  # f : orbitRadiusVariation（軌道半径ランダム変動幅）
        float(data.get("orbitOffsetVariation", 0.0)),  # f : orbitOffsetVariation（個体軌道オフセット最大幅）
        float(data.get("rotateScaleVariation", 0.0)),  # f : rotateScaleVariation（個体回転速度倍率変動幅）
    )




# ---------------------------------------------------------
# ルールブック（目次）
# ファイル名の一部と、上で作った関数を紐づけます
# ---------------------------------------------------------
RULE_BOOK = {
    "DaddyPenguinParameter":    pack_daddy_penguin,
    "PenguinEffectParameter":   pack_penguin_effect_parameter,
    "ChildPenguinParameter":    pack_child_penguin_parameter,
    "EnemyParameter":           pack_enemy_parameter,
    "oceanParameter":           pack_ocean_parameter,
    "PBRParameter":             pack_pbr_parameter,
    "whirlpoolParameter":       pack_whirlpool_parameter,
}

# ファイル全体を処理する特殊な変換テーブル（可変長データを持つ構造向け）
# 引数は (json_path, bin_path) を受け取る関数を登録する
FILE_RULE_BOOK = {
    "EnemyLayout": convert_enemy_layout,
    # 新しいレイアウト系JSONを追加する場合はここに1行追記する
    # "WhirlpoolPositions": convert_whirlpool_positions,
}


# =========================================================
# 2. 汎用コンバート処理（ここのコードは基本的に変更不要）
# =========================================================

# def convert_all(input_root_dir, output_root_dir):
#     os.makedirs(output_root_dir, exist_ok=True)
def convert_all(input_root_dir):
    
    # os.walk を使って、フォルダの中のフォルダも全て自動で探す
    for current_dir, dirs, files in os.walk(input_root_dir):
        for filename in files:
            if not filename.endswith(".json"):
                continue # JSON以外は無視
            
            # ─────────────────────────────────────────────────────────
            # FILE_RULE_BOOK（ファイル全体を処理する特殊変換）
            file_func = None
            for keyword, func in FILE_RULE_BOOK.items():
                if keyword in filename:
                    file_func = func
                    break

            if file_func is not None:
                json_path = os.path.join(current_dir, filename)
                bin_path = os.path.join(current_dir, filename.replace(".json", ".bin"))
                print(f"変換中: {json_path} -> {bin_path}")
                file_func(json_path, bin_path)
                continue  # 通常のRULE_BOOKには進まない
            # ─────────────────────────────────────────────────────────


            # 辞書(RULE_BOOK)から一致するルール(関数)を探す
            pack_function = None
            for keyword, func in RULE_BOOK.items():
                if keyword in filename:
                    pack_function = func
                    break
            
            if pack_function is None:
                # ルールブックに載っていないJSONは安全にスキップ
                continue

            # 入出力のパス設定（フォルダ階層をそのまま維持する）
            json_path = os.path.join(current_dir, filename)
            bin_path = os.path.join(current_dir, filename.replace(".json", ".bin"))
            
            print(f"変換中: {json_path} -> {bin_path}")
            
            # JSON読み込み
            try:
                with open(json_path, "r", encoding="utf-8-sig") as f:
                    data_list = json.load(f)
            except Exception as e:
                print(f"  [エラー] 読み込み失敗: {e}")
                continue

            # JSONのトップレベルがリストじゃない場合（単体データ等）はリストに包む
            if not isinstance(data_list, list):
                data_list = [data_list]

            count = len(data_list)
            
            # バイナリ書き出し
            with open(bin_path, "wb") as f:
                f.write(struct.pack("<i", count)) # 先頭に要素数を書く
                for data in data_list:
                    # ルールブックから取り出した専用関数を実行！
                    binary_data = pack_function(data)
                    f.write(binary_data)
            
            print(f"  -> [成功] {count}件のデータを書き出しました。")

if __name__ == "__main__":
    # 画像のパス構造に合わせるなら以下のように指定します
    input_folder = "../Game/Assets/parameter"
    convert_all(input_folder)