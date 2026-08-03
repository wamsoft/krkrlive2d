#include <ncbind.hpp>
#include <memory>

#include "Live2D.h"

// ttstrをUTF8文字列へ変換
extern std::string convertTtstrToUtf8String(const ttstr &buf);
extern ttstr convertUtf8StringToTtstr(const char *buf, size_t length);
extern ttstr convertUtf8StringToTtstr(const char *buf);
extern ttstr convertUtf8StringToTtstr(const std::string &buf);

extern IMemoryAllocator *createMemoryAllocator();

/**
 * @brief ファイルデータ読み込み処理用
 * LIVE2Dからファイル読み込みに対してファイルアクセス用のインターフェースを返す
 */
class PReadStream : public IReadStream {
public:
  PReadStream() : mRefCount(1), mStream(nullptr), mSize(0), mPosition(0) {
  }

  virtual ~PReadStream() {
    Close();
  }

	virtual int AddRef(void) {
    return ++mRefCount;
  }

	virtual int Release(void) {
    int result = --mRefCount;
    if (result == 0) delete this;
    return result;
  }

  void Close() {
    if (mStream) {
      mStream->Destruct();
      mStream = nullptr;
      mSize = 0;
      mPosition = 0;
    }
  }

  bool OpenFile(const char *filename) {
    Close();
    // 吉里吉里のストレージ経由でアクセス (XP3 アーカイブ内でも可)。
    // Win32 の IStream ではなく iTJSBinaryStream を使うことで、
    // 新旧いずれの tp_stub でもコンパイルできる。
    ttstr name = convertUtf8StringToTtstr(filename, strlen(filename));
    mStream = TVPCreateStream(name, TJS_BS_READ);
    if (mStream) {
      mSize = (size_t)mStream->GetSize();
      mPosition = 0;
      return true;
    }
    return false;
  }

  virtual size_t Read(void *buf, size_t size) {
    if (mStream) {
      size_t readed = (size_t)mStream->Read(buf, (tjs_uint)size);
      mPosition += readed;
      return readed;
    }
    return 0;
  }

  virtual int64_t Tell() const {
    // iTJSBinaryStream::GetPosition は tp_stub に実体が無いことがあるので自前で数える
    return (int64_t)mPosition;
  }

  virtual void Seek(int64_t offset, int origin) {
    if (mStream) {
      // origin(SET/CUR/END = 0/1/2)は iTJSBinaryStream の whence と一致する
      mStream->Seek((tjs_int64)offset, origin);
      switch (origin) {
        case TJS_BS_SEEK_SET: mPosition = (size_t)offset; break;
        case TJS_BS_SEEK_CUR: mPosition += (size_t)offset; break;
        case TJS_BS_SEEK_END: mPosition = (size_t)((int64_t)mSize + offset); break;
      }
    }
  }

  virtual size_t Size() const {
    return mSize;
  }

  static IReadStream *open(const char *file) {
    auto ret = new PReadStream();
    if (ret) {
      if (!ret->OpenFile(file)) {
        delete ret;
        ret = nullptr;
      }
    }
    return ret;
  }

private:
  int mRefCount;
  iTJSBinaryStream *mStream;
  size_t mSize;
  size_t mPosition;
};

static IMemoryAllocator *mAllocator = nullptr;
static ILive2DRuntime *sRuntime = nullptr;

static void logfunc(const char *message) {
  ttstr msg = convertUtf8StringToTtstr(message);
  TVPAddLog(msg);
}

// --------------------------------------------------------
// 共通ランタイム
// GLコンテキスト不要・初回利用時に遅延生成
// --------------------------------------------------------

ILive2DRuntime *
Live2DDevice::GetRuntime()
{
  if (!sRuntime) {
    if (!mAllocator) {
      mAllocator = createMemoryAllocator();
    }
    ILive2DRuntime::InitParam param;
    memset(&param, 0, sizeof(param));
    param.open = PReadStream::open;
    param.log  = logfunc;
    param.memoryAllocator = mAllocator;
    sRuntime = Live2DCreateRuntime(param);
  }
  return sRuntime;
}

void
Live2DDevice::Done()
{
  if (sRuntime) {
    sRuntime->Release();
    sRuntime = nullptr;
  }
  if (mAllocator) {
    delete mAllocator;
    mAllocator = nullptr;
  }
}

// --------------------------------------------------------
// デバイス
// --------------------------------------------------------

Live2DDevice::Live2DDevice(tTJSVariant glbase)
 : mLive2DDevice(nullptr)
 , mSurfaceWidth(0)
 , mSurfaceHeight(0)
{
  tTJSVariantClosure clo = glbase.AsObjectClosureNoAddRef();
  tTJSVariant iface_v;
  if (TJS_FAILED(clo.PropGet(0, TJS_W("GLGetProcAddress"), NULL, &iface_v, NULL)))
      TVPThrowExceptionMessage(TJS_W("Can't Retrive GLGetProcAddress interface"));
  GetGLProcAddressFunc glGetProcInterface = reinterpret_cast<GetGLProcAddressFunc>((tjs_int64)iface_v);

  ILive2DDevice::InitParam param;
  memset(&param, 0, sizeof(param));
  param.getProcAddress = glGetProcInterface;
  mLive2DDevice = GetRuntime()->CreateDevice(param);
}

Live2DDevice::~Live2DDevice()
{
  if (mLive2DDevice) {
    mLive2DDevice->Release();
    mLive2DDevice = nullptr;
  }
}

void
Live2DDevice::onBeginScene(int surfaceWidth, int surfaceHeight)
{
  mSurfaceWidth  = surfaceWidth;
  mSurfaceHeight = surfaceHeight;
  if (mLive2DDevice) {
    mLive2DDevice->OnBeginScene();
  }
}

void
Live2DDevice::onEndScene()
{
  if (mLive2DDevice) {
    mLive2DDevice->OnEndScene();
  }
}

void
Live2DDevice::prepareShaders()
{
  if (mLive2DDevice) {
    mLive2DDevice->PrepareShaders();
  }
}

ILive2DPlayer *
Live2DDevice::CreatePlayer(const char *filename)
{
  return mLive2DDevice ? mLive2DDevice->CreatePlayer(filename) : nullptr;
}

ILive2DPlayer *
Live2DDevice::CreatePlayer(const ILive2DModel *model)
{
  return mLive2DDevice ? mLive2DDevice->CreatePlayer(model) : nullptr;
}

// --------------------------------------------------------
// 情報参照・操作用モデル (デバイス不要)
// --------------------------------------------------------

Live2DModel::Live2DModel()
: mModel(nullptr)
{
}

Live2DModel::~Live2DModel()
{
  clear();
}

void Live2DModel::clear() {
  if (mModel) {
    mModel->Release();
    mModel = nullptr;
  }
  mFilename = TJS_W("");
}

void Live2DModel::load(const tjs_char *filename) {
  clear();
  mFilename = filename;
  std::string _filename = convertTtstrToUtf8String(mFilename);
  mModel = Live2DDevice::GetRuntime()->CreateModel(_filename.c_str());
}

Live2DModel *Live2DModel::clone() {
  Live2DModel *instance = new Live2DModel();
  instance->mFilename = mFilename;
  if (mModel) {
    instance->mModel = mModel->Clone();
  }
  return instance;
}

// --------------------------------------------------------
// 描画用プレイヤー
// --------------------------------------------------------

Live2DPlayer::Live2DPlayer(tTJSVariant device)
: mDeviceObj(device), mDevice(nullptr), mModel(nullptr), mScale(1.0)
{
  mDevice = ncbInstanceAdaptor<Live2DDevice>::GetNativeInstance(mDeviceObj.AsObjectNoAddRef());
}

Live2DPlayer::~Live2DPlayer()
{
  clear();
  mDevice = nullptr;
  mDeviceObj.Clear();
}

// ---------------------

void Live2DPlayer::alloc()
{
  if (mDevice && !mModel && mFilename != TJS_W("")) {
    std::string _filename = convertTtstrToUtf8String(mFilename);
    mModel = mDevice->CreatePlayer(_filename.c_str());
    if (mModel) {
      mModel->SetVisible(true);
    }
  }
}

void Live2DPlayer::free()
{
  if (mModel) {
    mModel->Release();
    mModel = 0;
  }
}

void Live2DPlayer::render() {
  if (mDevice && mModel) {
    int width = mDevice->SurfaceWidth();
    int height = mDevice->SurfaceHeight();
    mModel->SetScreenSize(width, height);
    mModel->SetScale(mScale, -mScale);
    tjs_real w = (tjs_real)width / 2.0;
    mModel->SetMatrix(mMtx.m11, -mMtx.m21, 0, 0,
                      mMtx.m12, -mMtx.m22, 0, 0,
                      0, 0, 1, 0,
                      mMtx.m13 / w, -mMtx.m23 / w, 0, 1);
    mModel->Render();
  }
}

// ---------------------

void Live2DPlayer::clear() {
  free();
  mFilename = TJS_W("");
}

void Live2DPlayer::load(const tjs_char *filename) {
  clear();
  mFilename = filename;
  alloc();
}

void Live2DPlayer::loadModel(tTJSVariant model) {
  clear();
  Live2DModel *live2dModel = ncbInstanceAdaptor<Live2DModel>::GetNativeInstance(model.AsObjectNoAddRef());
  if (live2dModel && live2dModel->GetIModel() && mDevice) {
    mModel = mDevice->CreatePlayer(live2dModel->GetIModel());
    if (mModel) {
      mModel->SetVisible(true);
    }
  }
}

Live2DPlayer *Live2DPlayer::clone() {
  Live2DPlayer *instance = new Live2DPlayer(mDeviceObj);
  instance->mFilename = mFilename;
  instance->mScale = mScale;
  instance->mMtx   = mMtx;
  if (mModel) {
    instance->mModel = mModel->Clone();
  }
  return instance;
}


void Live2DPlayer::setVoiceValue(tjs_real voice) {
  if (mModel) {
    return mModel->SetVoiceValue((float)voice);
  }
}

void Live2DPlayer::setVoiceWeight(tjs_real weight) {
  if (mModel) {
    return mModel->SetVoiceWeight((float)weight);
  }
}

void Live2DPlayer::setVoiceMode(tjs_int mode) {
  if (mModel) {
    return mModel->SetVoiceMode((float)mode);
  }
}

void Live2DPlayer::setBlinkingInterval(tjs_real blinkingInterval) {
  if (mModel) {
    return mModel->SetBlinkingInterval(blinkingInterval);
  }
}

void Live2DPlayer::setBlinkingSettings(tjs_real closing, tjs_real closed,
                                      tjs_real opening) {
  if (mModel) {
    return mModel->SetBlinkingSettings(closing, closed, opening);
  }
}

void Live2DPlayer::setBlinkingMode(tjs_int mode) {
  if (mModel) {
    return mModel->SetBlinkingMode(mode);
  }
}

void Live2DPlayer::setMosaicParam(tjs_real mosaicx, tjs_real mosaicy) {
  if (mModel) {
    return mModel->SetMosaicParam(mosaicx, mosaicy);
  }
}

// オブジェクトに数値を格納
static void setIntProp(iTJSDispatch2 *obj, const tjs_char *name,
                         int value) {
  tTJSVariant var = value;
  obj->PropSet(TJS_MEMBERENSURE, name, NULL, &var, obj);
}

// オブジェクトに数値を格納
static void setFloatProp(iTJSDispatch2 *obj, const tjs_char *name,
                         float value) {
  tTJSVariant var = value;
  obj->PropSet(TJS_MEMBERENSURE, name, NULL, &var, obj);
}

// オブジェクトに文字列を格納
static void setStrProp(iTJSDispatch2 *obj, const tjs_char *name, ttstr &value) {
  tTJSVariant var = value;
  obj->PropSet(TJS_MEMBERENSURE, name, NULL, &var, obj);
}

#undef CLASSNAME
#define CLASSNAME Live2DModel
#include "Live2D_common.cpp"

#undef CLASSNAME
#define CLASSNAME Live2DPlayer
#include "Live2D_common.cpp"

//-----------------------------------------------------

NCB_REGISTER_CLASS(Live2DMatrix) {
  NCB_CONSTRUCTOR((float, float, float, float, float, float));
};

NCB_REGISTER_CLASS(Live2DDevice) {
  NCB_CONSTRUCTOR((tTJSVariant));
  NCB_METHOD(onBeginScene);
  NCB_METHOD(onEndScene);
  NCB_METHOD(prepareShaders);
};

NCB_REGISTER_CLASS(Live2DModel) {
  NCB_CONSTRUCTOR(());

  NCB_METHOD(clone);
  NCB_METHOD(clear);
  NCB_METHOD(load);

#include "Live2D_method.cpp"
};

NCB_REGISTER_CLASS(Live2DPlayer) {
  NCB_CONSTRUCTOR((tTJSVariant));

  NCB_PROPERTY_RO(device, getDevice);

  NCB_METHOD(clone);
  NCB_METHOD(clear);
  NCB_METHOD(load);
  NCB_METHOD(loadModel);

  NCB_METHOD(setScale);
  NCB_METHOD(getScale);
  NCB_METHOD(setMatrix);
  NCB_METHOD(render);

  NCB_METHOD(setVoiceValue);
  NCB_METHOD(setVoiceWeight);
  NCB_METHOD(setVoiceMode);
  NCB_METHOD(setBlinkingInterval);
  NCB_METHOD(setBlinkingSettings);
  NCB_METHOD(setBlinkingMode);
  NCB_METHOD(setMosaicParam);

#include "Live2D_method.cpp"
};


static void PostUnregistCallback()
{
  Live2DDevice::Done();
}

NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
