#ifndef __LIVE2D_CLASS__
#define __LIVE2D_CLASS__

#include <vector>
#include <ilive2d.h>

class Live2DModel;

class Live2DMatrix {
  friend class Live2DModel;

 public:
  Live2DMatrix()
      : m11(1), m12(0), m13(0), m21(0), m22(1), m23(0) {}
  Live2DMatrix(float m11, float m12, float m13, float m21,
               float m22, float m23)
      : m11(m11), m12(m12), m13(m13), m21(m21), m22(m22), m23(m23) {}
  Live2DMatrix(const Live2DMatrix &src)
      : m11(src.m11),
        m12(src.m12),
        m13(src.m13),
        m21(src.m21),
        m22(src.m22),
        m23(src.m23) {}

  void set(float m11, float m12, float m13, float m21, float m22, float m23) {
    this->m11 = m11; this->m12 = m12; this->m13 = m13;
    this->m21 = m21; this->m22 = m22; this->m23 = m23;
  }

 protected:
  float m11, m12, m13;
  float m21, m22, m23;
};

class Live2DModel;

class Live2DDevice
{
  
public:
  Live2DDevice(tTJSVariant glbase);
  ~Live2DDevice();

  void onBeginScene(int surfaceWidth, int surfaceHeight);
  void onEndScene();

  ILive2DPlayer *CreatePlayer(const char *filename);
  int SurfaceWidth() const { return mSurfaceWidth; };
  int SurfaceHeight() const { return mSurfaceHeight; };

  static void Done();

private:
  ILive2DDevice *mLive2DDevice;
  int mSurfaceWidth;
  int mSurfaceHeight;
};

class Live2DModel {
 public:
  Live2DModel(tTJSVariant device);

  virtual ~Live2DModel();

  tTJSVariant getDevice() const {
    return mDeviceObj;
  }

  void clear();
  void load(const tjs_char *fileName);

  Live2DModel *clone();

  void setScale(tjs_real scale) { mScale = scale; }
  tjs_real getScale() { return mScale; }

  void setMatrix(float m11, float m12, float m13, float m21, float m22, float m23) { 
    mMtx.set(m11, m12, m13, m21, m22, m23);
  }

  void render();

  void setVoiceValue(tjs_real voice);
  void setVoiceWeight(tjs_real weight);
  void setVoiceMode(tjs_int mode);
  void setBlinkingInterval(tjs_real blinkigInterval);
  void setBlinkingSettings(tjs_real closing, tjs_real closed, tjs_real opening);
  void setBlinkingMode(tjs_int mode);

  void setMosaicParam(tjs_real mosaicx, tjs_real mosaicy);

#undef CLASSNAME
#define CLASSNAME Live2DModel
#include "Live2D_common.h"

 private:
  tTJSVariant mDeviceObj;
  Live2DDevice *mDevice;
  ILive2DPlayer *mModel;
  tjs_real mScale;
  Live2DMatrix mMtx;
  ttstr mFilename;

  void alloc();
  void free();
};

#endif
