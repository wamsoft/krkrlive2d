#include <Windows.h>
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
  PReadStream() : mRefCount(1), mStream(nullptr), mSize(0) {
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
      mStream->Release();
      mStream = nullptr;
      mSize = 0;
    }
  }

  bool OpenFile(const char *filename) {
    Close();
    // アーカイブ内ファイルの場合はストリーム経由アクセスでメモリ読み出し
    ttstr name = convertUtf8StringToTtstr(filename, strlen(filename));
    mStream = TVPCreateIStream(name, TJS_BS_READ);
    if (mStream) {
      STATSTG stat;
      mStream->Stat(&stat, STATFLAG_NONAME);
      mSize = (size_t)(stat.cbSize.QuadPart);
      return true;
    }
    return false;
  }

  virtual size_t Read(void *buf, size_t size) {
    if (mStream) {
      ULONG readed;
      if (SUCCEEDED(mStream->Read(buf, size, &readed))) {
        return readed;
      }
    }
    return 0;
  }

  virtual int64_t Tell() const {
    if (mStream) {
      LARGE_INTEGER offset;
      offset.QuadPart = 0;
      ULARGE_INTEGER pos;
      mStream->Seek(offset, SEEK_CUR, &pos);
      return pos.QuadPart;
    }
    return 0;
  }

  virtual void Seek(int64_t offset, int origin) {
    if (mStream) {
      LARGE_INTEGER _offset;
      _offset.QuadPart = offset;
      mStream->Seek(_offset, origin, nullptr);
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
  ULONG mRefCount;
  IStream *mStream;
  size_t mSize;
};

IMemoryAllocator *mAllocator = nullptr;

static void logfunc(const char *message) {
  ttstr msg = convertUtf8StringToTtstr(message);
  TVPAddLog(msg);
}

Live2DDevice::Live2DDevice(tTJSVariant glbase)
 : mLive2DDevice(nullptr)
 , mSurfaceWidth(0)
 , mSurfaceHeight(0)
{
  if (!mAllocator) {
    mAllocator = createMemoryAllocator();
  }

  tTJSVariantClosure clo = glbase.AsObjectClosureNoAddRef();
  tTJSVariant iface_v;
  if (TJS_FAILED(clo.PropGet(0, TJS_W("GLGetProcAddress"), NULL, &iface_v, NULL)))
      TVPThrowExceptionMessage(TJS_W("Can't Retrive GLGetProcAddress interface"));
  GetGLProcAddressFunc glGetProcInterface = reinterpret_cast<GetGLProcAddressFunc>((tjs_int64)iface_v);

  ILive2DDevice::InitParam param;
  memset(&param, 0, sizeof(param));
  param.open = PReadStream::open;
  param.log  = logfunc;
  param.getProcAddress = glGetProcInterface;
  param.memoryAllocator = mAllocator;
  mLive2DDevice = Live2DCreate(param);
}

Live2DDevice::~Live2DDevice()
{
  if (mLive2DDevice) {
    mLive2DDevice->Release();
    mLive2DDevice = nullptr;
  }
}

void
Live2DDevice::Done()
{
  if (mAllocator) {
    delete mAllocator;
    mAllocator = nullptr;
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

ILive2DPlayer *
Live2DDevice::CreatePlayer(const char *filename)
{
  return mLive2DDevice ? mLive2DDevice->CreatePlayer(filename) : nullptr;
}

// --------------------------------------------------------

Live2DModel::Live2DModel(tTJSVariant device)
: mDeviceObj(device), mDevice(nullptr), mModel(nullptr), mScale(1.0) 
{
  mDevice = ncbInstanceAdaptor<Live2DDevice>::GetNativeInstance(mDeviceObj.AsObjectNoAddRef());
}

Live2DModel::~Live2DModel() 
{
  clear();
  mDevice = nullptr;
  mDeviceObj.Clear();
}

// ---------------------

void Live2DModel::alloc() 
{
  if (mDevice && !mModel && mFilename != TJS_W("")) {
    std::string _filename = convertTtstrToUtf8String(mFilename);
    mModel = mDevice->CreatePlayer(_filename.c_str());
    if (mModel) {
      mModel->SetVisible(true);
    }
  }
}

void Live2DModel::free() 
{
  if (mModel) {
    mModel->Release();
    mModel = 0;
  }
}

void Live2DModel::render() {
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

void Live2DModel::clear() {
  free();
  mFilename = TJS_W("");
}

void Live2DModel::load(const tjs_char *filename) {
  clear();
  mFilename = filename;
  alloc();
}

Live2DModel *Live2DModel::clone() {
  Live2DModel *instance = new Live2DModel(mDeviceObj);
  instance->mFilename = mFilename;
  instance->mScale = mScale;
  instance->mMtx   = mMtx;
  if (mModel) {
    instance->mModel = mModel->Clone();
  }
  return instance;
}


void Live2DModel::setVoiceValue(tjs_real voice) {
  if (mModel) {
    return mModel->SetVoiceValue((float)voice);
  }
}

void Live2DModel::setVoiceWeight(tjs_real weight) {
  if (mModel) {
    return mModel->SetVoiceWeight((float)weight);
  }
}

void Live2DModel::setVoiceMode(tjs_int mode) {
  if (mModel) {
    return mModel->SetVoiceMode((float)mode);
  }
}

void Live2DModel::setBlinkingInterval(tjs_real blinkingInterval) {
  if (mModel) {
    return mModel->SetBlinkingInterval(blinkingInterval);
  }
}

void Live2DModel::setBlinkingSettings(tjs_real closing, tjs_real closed,
                                      tjs_real opening) {
  if (mModel) {
    return mModel->SetBlinkingSettings(closing, closed, opening);
  }
}

void Live2DModel::setBlinkingMode(tjs_int mode) {
  if (mModel) {
    return mModel->SetBlinkingMode(mode);
  }
}

void Live2DModel::setMosaicParam(tjs_real mosaicx, tjs_real mosaicy) {
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

//-----------------------------------------------------

NCB_REGISTER_CLASS(Live2DMatrix) {
  NCB_CONSTRUCTOR((float, float, float, float, float, float));
};

NCB_REGISTER_CLASS(Live2DDevice) {
  NCB_CONSTRUCTOR((tTJSVariant));
  NCB_METHOD(onBeginScene);
  NCB_METHOD(onEndScene);
};

NCB_REGISTER_CLASS(Live2DModel) {
  NCB_CONSTRUCTOR((tTJSVariant));

  NCB_PROPERTY_RO(device, getDevice);

  NCB_METHOD(clone);
  NCB_METHOD(clear);
  NCB_METHOD(load);

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