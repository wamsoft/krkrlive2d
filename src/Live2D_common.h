  // ---------------------------------------------------------
  // 表情設定
  // 識別名で指定
  int getExpressionCount() const;
  ttstr getExpressionName(int no);

  void setExpression(const ttstr expressionId);
  ttstr getExpression();

  void fixExpression(const ttstr expressionId);

  // ---------------------------------------------------------
  // モーション設定
  // グループ名と番号で指定

  int getMotionGroupCount();
  ttstr getMotionGroupName(int index);

  int getMotionCount(const ttstr groupName);
  ttstr getMotionName(const ttstr groupName, int no);

  int getMotionType(const ttstr groupName);
  void setMotionType(const ttstr groupName,  int type);

  static tjs_error startMotion(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);

  tTJSVariant getCurrentMotions();
  
  void stopMotion(const ttstr groupName);

  // -----------------------------------------------------------
  // パラメータ用処理
  // -----------------------------------------------------------

  int getParameterCount();
  tTJSVariant getParameterInfo(int no);

  static tjs_error getParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);
  static tjs_error setParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);

  static tjs_error setParameterType(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);
  static tjs_error getDiffParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);
  static tjs_error setDiffParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);
  void clearDiffParameters();

  tTJSVariant getParameters();

  // -----------------------------------------------------------
  // 全体操作
  // -----------------------------------------------------------

  void progress(float tick);
  bool canSync();
  void sync();

  static tjs_error addLipSyncId(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);
  static tjs_error addEyeBlinkId(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);

  bool isPlaying(const ttstr groupName);

  // -----------------------------------------------------------
  // パーツ用処理
  // -----------------------------------------------------------

  int getPartCount();
  tTJSVariant getPartInfo(int no);

  static tjs_error getPartValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);
  static tjs_error setPartValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);

  void setPartFadeTime(float fadeTime);

  // -----------------------------------------------------------
  // イベント処理
  // -----------------------------------------------------------

  int getEventCount();
  ttstr getEventName(int no);

  // -----------------------------------------------------------
  // 変数モーション
  // -----------------------------------------------------------

  static tjs_error addVariableMotion(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self);
  void delVariableMotion(const ttstr varname);
  int getVariableMotionCount();
  ttstr getVariableMotionName(int index);
  tTJSVariant getVariableMotionInfo(int index);
  void setVariable(const ttstr varname, tjs_real value);
  tjs_real getVariable(const ttstr varname);

  // -----------------------------------------------------------
  // 特殊
  // -----------------------------------------------------------
  bool isMosaicModel();
