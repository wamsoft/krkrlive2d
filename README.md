# krkrlive2d — 吉里吉里用 Live2D (Cubism) プラグイン

吉里吉里Z (KAG/TJS) 上で **Live2D Cubism** モデルを再生・描画するための
プラグインです。OpenGL (GLES2 / GLAD) ドライバ経由でレンダリングします。

## 構成

| プラグイン | 内容 | 主なTJSクラス |
|---|---|---|
| `krkrlive2d.dll` | Live2D Cubism モデルの再生・制御・描画 | `Live2DDevice` / `Live2DModel`<br>`Live2DPlayer` / `Live2DMatrix` |

- `Live2DDevice` … OpenGL コンテキストごとに1つ生成し、`onBeginScene`/`onEndScene` で
  フレーム描画を挟みます。`prepareShaders()` でシェーダを事前コンパイルできます。
- `Live2DModel` … **デバイス (GLコンテキスト) なしで生成できる**情報参照・操作用モデル。
  モデルの情報参照・パラメータ操作・モーション制御が可能ですが、描画はできません。
- `Live2DPlayer` … `Live2DModel` と同じインターフェースに描画用メソッドを加えたプレイヤー。
  生成には `Live2DDevice` が必要で、`loadModel(model)` で読み込み済み `Live2DModel` の
  状態を引き継いで生成することもできます。
- 配置は表示マトリクス (`setMatrix`) / スケール (`setScale`)、モデル内部の制御は
  パラメータ (`setParameterValue` 等) やモーション (`startMotion` 等) で行います。

`Live2DModel` と `Live2DPlayer` は情報参照・操作系のインターフェース
(表情・モーション・パラメータ・パーツ・変数モーション等) を共通で持ち、
`Live2DPlayer` のみ描画・表示変換・リップシンク/モザイク等の追加メソッドを持ちます。

各クラス・メソッドの詳細は **[`manual.tjs`](manual.tjs)** を参照してください
（全API・引数・戻り値を記載）。

## ビルド

### 前提
- Visual Studio (MSVC, C++17) / CMake 3.16+ / Ninja / Git
- [vcpkg](https://github.com/microsoft/vcpkg)（`VCPKG_ROOT` 環境変数を設定）。
  依存は `vcpkg.json` に記載（`minizip-ng`）。
- 吉里吉里 `tp_stub`（既定では `../tp_stub`。`TPSTUB_DIR` で変更可）
- **Live2D ドライバライブラリ**（GLES2 / GLAD 版）。取得方法は下記の2通りで、
  `CMakeLists.txt` が自動判定します。
  - 環境変数 **`LIVE2DLIB_FOLDER`** にローカルのソースパスが設定されていれば、
    それを参照してビルドします（ローカル開発用）。
  - 未設定・またはパスが存在しない場合は、CMake の FetchContent で
    <https://github.com/wtnbgo/live2d.git>（`main`）を**サブモジュールごと自動取得**して
    ビルドします。この場合は Git とネットワーク接続が必要です。
- **Live2D Cubism Core**。ドライバライブラリが Cubism Core を静的リンクするため、
  環境変数 **`CUBISM_SDK`** に Cubism SDK（Cubism Native、例:
  `CubismNativeSamples`）のルートを設定してください。ドライバ側 CMake が
  `$ENV{CUBISM_SDK}/Core`（`Core/include`・`Core/lib/...`）を参照します。
  Cubism SDK / Core のライセンスについては下記「ライセンス」を参照。

> **環境変数について**
> `LIVE2DLIB_FOLDER` / `CUBISM_SDK` は**システム設定後に開いたシェル**にしか
> 反映されません。既存のシェルからビルドする場合は
> `set CUBISM_SDK=...`（PowerShell では `$env:CUBISM_SDK="..."`）で
> 上書きしてください。

### 手順 (Windows x86 の例)
```sh
# 環境変数
set VCPKG_ROOT=D:\vcpkg
set CUBISM_SDK=D:\work\live2d\CubismNativeSamples
# (任意) ローカルのドライバソースを使う場合のみ指定。未設定なら GitHub から自動取得。
set LIVE2DLIB_FOLDER=D:\work\live2d\live2d

# 構成（プリセットは CMakePresets.json 参照: x86-windows / x64-windows /
#       x64-linux / arm64-android / arm64-ios / arm64-nx 等）
cmake --preset x86-windows

# ビルド
cmake --build build/x86-windows --config Release
```
`Makefile` 経由でのビルドにも対応しています（`PRESET` / `BUILD_TYPE` を指定）。

```sh
make PRESET=x86-windows BUILD_TYPE=Release
```

生成物: `build/<preset>/Release/krkrlive2d.dll`

## 使い方（概略）

`Live2DDevice` のコンストラクタには **`oglbase`**（`GetProcAddress` を保持する
OpenGL コンテキストオブジェクト）を渡します。これには以下のいずれかを指定できます。

- **`krkrgles` プラグイン**が提供する OpenGL コンテキストオブジェクト
- **新しい吉里吉里Z の `OGLDrawDevice`**（本体組み込みの OpenGL 描画デバイス）

いずれの場合も、`Live2DDevice` は**生成時にカレントだった GL コンテキスト**に対して
初期化されます。`onBeginScene` / `onEndScene` / `prepareShaders` も**同じコンテキスト**で
呼び出してください。

```tjs
// --- 描画用プレイヤーとして使う ---
var dev = new Live2DDevice(oglbase);   // oglbase = krkrgles または OGLDrawDevice
dev.prepareShaders();                  // (任意) シェーダ事前コンパイル
var player = new Live2DPlayer(dev);
player.load("model.l2d");              // モデルファイル読み込み
player.setScale(1.0);
// ... onBeginScene / progress(tick) / render / onEndScene ...

// --- GL コンテキストなしで情報参照・操作だけ行う ---
var model = new Live2DModel();         // デバイス不要
model.load("model.l2d");
var n = model.getMotionGroupCount();
// 読み込み済みモデルからプレイヤーを生成する
var player2 = new Live2DPlayer(dev);
player2.loadModel(model);
```
詳細・全メソッドは [`manual.tjs`](manual.tjs) を参照。

## l2d ファイルの準備

吉里吉里ではパック時にフォルダ構成が失われるのと、パスを無視した
ファイル検索が行われるため、標準の Cubism のファイル構成だと
対象ファイルがうまく選べなくなります。そこで **Cubism が出力した
モデル一式のフォルダをまるごと zip で束ね、拡張子 `.l2d` にリネームした
ものを1ファイル**として読み込みます（`load("model.l2d")`）。

### 元フォルダの構成

Cubism からエクスポートしたモデルフォルダをそのまま使います。
`model3.json` を頂点に、テクスチャ・モーション・表情などが相対パスで
参照されている、標準的な構成です（例: `hiyori_pro_t10`）。

```
hiyori_pro_t10/
├── hiyori_pro_t10.model3.json      … モデル定義 (エントリポイント)
├── hiyori_pro_t10.moc3             … モデル本体
├── hiyori_pro_t10.physics3.json    … 物理演算
├── hiyori_pro_t10.pose3.json       … ポーズ
├── hiyori_pro_t10.cdi3.json        … パラメータ表示名情報
├── hiyori_pro_t10.2048/            … テクスチャ
│   ├── texture_00.png
│   └── texture_01.png
├── motion/                         … モーション
│   ├── hiyori_m01.motion3.json
│   ├── hiyori_m02.motion3.json
│   └── … (hiyori_m10 まで)
└── expressions/                    … 表情
    ├── exp_01.exp3.json
    ├── exp_02.exp3.json
    └── … (exp_10 まで)
```

### zip で束ねて .l2d にする

**フォルダの「中身」を zip のルート直下に入れる**のがポイントです
（`hiyori_pro_t10/` という階層を1段挟まず、`model3.json` が zip の
トップに来るようにします）。

```sh
# フォルダの中に入って、中身をすべて zip 化してから .l2d にリネーム
cd hiyori_pro_t10
zip -r ../hiyori_pro_t10.l2d .
```

PowerShell の場合:

```powershell
# 中身を zip 圧縮 (末尾の \* でフォルダ自身ではなく中身を対象にする)
Compress-Archive -Path .\hiyori_pro_t10\* -DestinationPath .\hiyori_pro_t10.zip
Rename-Item .\hiyori_pro_t10.zip hiyori_pro_t10.l2d
```

生成した `hiyori_pro_t10.l2d` を吉里吉里のアーカイブに含め、
`load("hiyori_pro_t10.l2d")` で読み込みます。zip 内の階層は保持されるため、
`model3.json` からの相対パス参照 (テクスチャ・モーション等) はそのまま解決されます。


## ライセンス（※仮記載・後日整備）

> 以下は項目の仮置きです。正式な文面・条件は別途準備します。

- **Live2D Cubism SDK / Core について**
  本プラグインがリンクする Live2D Cubism の SDK / Core ライブラリおよび
  ランタイム (`ilive2d.h` 等のインターフェース越しに利用) は
  **株式会社 Live2D の著作物**です。これらを含む形での**頒布・再配布には、
  株式会社 Live2D の使用許諾契約 (Cubism SDK Release License 等) への同意が
  必要**です。利用・配布の前に同社のライセンス条件を確認・取得してください。
  - Live2D 公式: https://www.live2d.com/
  - SDK ライセンス: https://www.live2d.com/download/cubism-sdk/

- **本プラグイン（krkrlive2d）のソースコードについて**
  本プラグイン自体のソースコードは **PUBLIC DOMAIN** として扱います。
  自由に利用・改変・再配布して構いません（無保証）。
  ただしリンクする Live2D Cubism SDK / Core 等については上記のとおり
  各権利者のライセンスに従う必要があります。

- **第三者コンポーネント**
  - 吉里吉里Z / tp_stub / ncbind: 各配布元のライセンスに従います。
  - minizip-ng / GLAD 等の依存: 各ライセンスに従います。
