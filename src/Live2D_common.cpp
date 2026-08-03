// ---------------------------------------------------------
// 表情設定
// 識別名で指定
int
CLASSNAME::getExpressionCount() const
{
  if (mModel) {
		return mModel->GetExpressionCount();
  }
	return 0;
}

ttstr
CLASSNAME::getExpressionName(int no)
{
  if (mModel) {
		return convertUtf8StringToTtstr(mModel->GetExpressionName(no));
  }
  return "";
}

void
CLASSNAME::setExpression(const ttstr expressionId)
{
  if (mModel) {
		std::string exp = convertTtstrToUtf8String(expressionId);
		mModel->SetExpression(exp.c_str());
  }
}

ttstr
CLASSNAME::getExpression()
{
  if (mModel) {
		return convertUtf8StringToTtstr(mModel->GetExpression());
  }
  return "";
}

void
CLASSNAME::fixExpression(const ttstr expressionId)
{
  if (mModel) {
		std::string exp = convertTtstrToUtf8String(expressionId);
		mModel->FixExpression(exp.c_str());
  }
}

// ---------------------------------------------------------
// モーション設定
// グループ名と番号で指定

int
CLASSNAME::getMotionGroupCount()
{
  if (mModel) {
		return mModel->GetMotionGroupCount();
  }
	return 0;
}

ttstr
CLASSNAME::getMotionGroupName(int index)
{
  if (mModel) {
		return convertUtf8StringToTtstr(mModel->GetMotionGroupName(index));
  }
  return "";
}

int
CLASSNAME::getMotionCount(const ttstr groupName)
{
  if (mModel) {
		std::string group = convertTtstrToUtf8String(groupName);
		return mModel->GetMotionCount(group.c_str());
  }
  return 0;
}

ttstr
CLASSNAME::getMotionName(const ttstr groupName, int no)
{
  if (mModel) {
		std::string group = convertTtstrToUtf8String(groupName);
	return convertUtf8StringToTtstr(mModel->GetMotionName(group.c_str(), no));
  }
  return "";
}

int 
CLASSNAME::getMotionType(const ttstr groupName)
{
  if (mModel) {
	std::string group = convertTtstrToUtf8String(groupName);
	return mModel->GetMotionType(group.c_str());
  }
  return 0;
}

void 
CLASSNAME::setMotionType(const ttstr groupName,  int type)
{
  if (mModel) {
	std::string group = convertTtstrToUtf8String(groupName);
	return mModel->SetMotionType(group.c_str(), type);
  }
}

tjs_error
CLASSNAME::startMotion(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 2) {
		return TJS_E_BADPARAMCOUNT;
	}
	if (self->mModel) {
		int loop = -1;
		if (numparams >= 3) {
			loop = params[2]->AsInteger();
		}
		float fadeIn = -1.0;
		if (numparams >= 4) {
			float fade = params[3]->AsReal();
			if (fade > 0.0) {
				fadeIn = fade;
			}
		}
		float fadeOut  = fadeIn;
		if (numparams >= 5) {
			float fade = params[4]->AsReal();
			if (fade >= 0.0) {
				fadeOut = fade;
			}
		}
		std::string group = convertTtstrToUtf8String(ttstr(params[0]->GetString()));
		if (params[1]->Type() == tvtInteger) {
			self->mModel->StartMotion(group.c_str(), params[0]->AsInteger(), loop, fadeIn, fadeOut);
		} else if (params[1]->Type() == tvtString) {
			std::string name = convertTtstrToUtf8String(ttstr(params[1]->GetString()));
			self->mModel->StartMotion(group.c_str(), name.c_str(), loop, fadeIn, fadeOut);
		}
	}
	return TJS_S_OK;
}

void
CLASSNAME::stopMotion(const ttstr groupName)
{
  if (mModel) {
	std::string group = convertTtstrToUtf8String(groupName);
	mModel->StopMotion(group.c_str());
  }
}


tTJSVariant 
CLASSNAME::getCurrentMotions()
{
	static tjs_uint32 addHint;
	tTJSVariant result;
	if (mModel) {
		iTJSDispatch2 *obj;
		if ((obj = TJSCreateArrayObject()) != NULL) {	
			int count = mModel->GetCurrentMotionCount();
			for (int no=0; no< count;no++ ) {
				iTJSDispatch2 *dict;
				if ((dict = TJSCreateDictionaryObject()) != NULL) {	
					ttstr name = convertUtf8StringToTtstr(mModel->GetCurrentMotionName(no));
					int loop = mModel->GetCurrentMotionLoop(no);
					setStrProp(dict, TJS_W("name"), name);
					setIntProp(dict, TJS_W("loop"), loop);
					tTJSVariant value = tTJSVariant(dict, dict);
					tTJSVariant *param = &value;
					obj->FuncCall(0, TJS_W("add"), &addHint, 0, 1, &param, obj);
					dict->Release();
				}
			}
			result = tTJSVariant(obj, obj);
			obj->Release();
		}
	}
	return result;
}

// -----------------------------------------------------------
// パラメータ用処理
// -----------------------------------------------------------

int
CLASSNAME::getParameterCount()
{
  if (mModel) {
		return mModel->GetParameterCount();
  }
	return 0;
}

tTJSVariant 
CLASSNAME::getParameterInfo(int no)
{
	tTJSVariant result;
	if (mModel) {
		iTJSDispatch2 *obj;
		if ((obj = TJSCreateDictionaryObject()) != NULL) {	
			setStrProp(obj, TJS_W("id"), convertUtf8StringToTtstr(mModel->GetParameterId(no)));
			setStrProp(obj, TJS_W("name"), convertUtf8StringToTtstr(mModel->GetParameterName(no)));
			setFloatProp(obj, TJS_W("min"), mModel->GetParameterMinimumValue(no));
			setFloatProp(obj, TJS_W("max"), mModel->GetParameterMaximumValue(no));
			setFloatProp(obj, TJS_W("default"), mModel->GetParameterDefaultValue(no));
			result = tTJSVariant(obj, obj);
			obj->Release();
		}
	}
	return result;
}


tjs_error 
CLASSNAME::getParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 1) {
		return TJS_E_BADPARAMCOUNT;
	}
	if (self->mModel && result) {
		if (params[0]->Type() == tvtInteger) {
			*result = self->mModel->GetParameterValue(params[0]->AsInteger());
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			*result =  self->mModel->GetParameterValue(_label.c_str());
		}
	}
	return TJS_S_OK;
}

tjs_error
CLASSNAME::setParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 2) {
		return TJS_E_BADPARAMCOUNT;
	}
  if (self->mModel) {
	  float value = (float)params[1]->AsReal();
		if (params[0]->Type() == tvtInteger) {
			self->mModel->SetParameterValue(params[0]->AsInteger(), value);
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			self->mModel->SetParameterValue(_label.c_str(), value);
		}
	}
	return TJS_S_OK;
}

tjs_error
CLASSNAME::setParameterType(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 2) {
		return TJS_E_BADPARAMCOUNT;
	}
  if (self->mModel) {
	  float value = (int)params[1]->AsInteger();
		if (params[0]->Type() == tvtInteger) {
			self->mModel->SetParameterType(params[0]->AsInteger(), value);
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			self->mModel->SetParameterType(_label.c_str(), value);
		}
	}
	return TJS_S_OK;
}

tjs_error 
CLASSNAME::getDiffParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 1) {
		return TJS_E_BADPARAMCOUNT;
	}
	if (self->mModel && result) {
		if (params[0]->Type() == tvtInteger) {
			*result = self->mModel->GetDiffParameterValue(params[0]->AsInteger());
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			*result =  self->mModel->GetParameterValue(_label.c_str());
		}
	}
	return TJS_S_OK;
}

tjs_error
CLASSNAME::setDiffParameterValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 2) {
		return TJS_E_BADPARAMCOUNT;
	}
	if (self->mModel) {
	  float value = (float)params[1]->AsReal();
		if (params[0]->Type() == tvtInteger) {
			self->mModel->SetDiffParameterValue(params[0]->AsInteger(), value);
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			self->mModel->SetDiffParameterValue(_label.c_str(), value);
		}
	}
	return TJS_S_OK;
}

void
CLASSNAME::clearDiffParameters()
{
  if (mModel) {
	mModel->ClearDiffParameters();
  }
}

tTJSVariant 
CLASSNAME::getParameters()
{
	tTJSVariant result;
	if (mModel) {
		iTJSDispatch2 *obj;
		if ((obj = TJSCreateDictionaryObject()) != NULL) {	
			int count = mModel->GetParameterCount();
			for (int no=0; no< count;no++ ) {
				ttstr name = convertUtf8StringToTtstr(mModel->GetParameterName(no));
				setFloatProp(obj, name.c_str(), mModel->GetParameterValue(no));
			}
			result = tTJSVariant(obj, obj);
			obj->Release();
		}
	}
	return result;
}

void
CLASSNAME::progress(float tick)
{
  if (mModel) {
	mModel->Progress(tick);
  }
}

bool
CLASSNAME::canSync()
{
  if (mModel) {
	return mModel->CanSync();
  }
  return false;
}

void
CLASSNAME::sync()
{
  if (mModel) {
	mModel->Sync();
  }
}

tjs_error
CLASSNAME::addLipSyncId(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 1) {
		return TJS_E_BADPARAMCOUNT;
	}
  if (self->mModel) {
		if (params[0]->Type() == tvtInteger) {
			self->mModel->AddLipSyncId(params[0]->AsInteger());
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			self->mModel->AddLipSyncId(_label.c_str());
		}
	}
	return TJS_S_OK;
}

tjs_error
CLASSNAME::addEyeBlinkId(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 1) {
		return TJS_E_BADPARAMCOUNT;
	}
  if (self->mModel) {
		if (params[0]->Type() == tvtInteger) {
			self->mModel->AddEyeBlinkId(params[0]->AsInteger());
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			self->mModel->AddEyeBlinkId(_label.c_str());
		}
	}
	return TJS_S_OK;
}

bool
CLASSNAME::isPlaying(const ttstr groupName)
{
  if (mModel) {
	std::string group = convertTtstrToUtf8String(groupName);
	return mModel->IsPlaying(group.c_str());
  }
  return false;
}

// -----------------------------------------------------------
// パーツ用処理
// -----------------------------------------------------------

int
CLASSNAME::getPartCount()
{
  if (mModel) {
		return mModel->GetPartCount();
  }
	return 0;
}

tTJSVariant 
CLASSNAME::getPartInfo(int no)
{
	tTJSVariant result;
	if (mModel) {
		iTJSDispatch2 *obj;
		if ((obj = TJSCreateDictionaryObject()) != NULL) {	
			setStrProp(obj, TJS_W("id"), convertUtf8StringToTtstr(mModel->GetPartId(no)));
			setStrProp(obj, TJS_W("name"), convertUtf8StringToTtstr(mModel->GetPartName(no)));
			setIntProp(obj, TJS_W("value"), mModel->GetPartValue(no));
			result = tTJSVariant(obj, obj);
			obj->Release();
		}
	}
	return result;
}


tjs_error 
CLASSNAME::getPartValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 1) {
		return TJS_E_BADPARAMCOUNT;
	}
	if (self->mModel && result) {
		if (params[0]->Type() == tvtInteger) {
			*result = self->mModel->GetPartValue(params[0]->AsInteger());
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			*result =  self->mModel->GetPartValue(_label.c_str());
		}
	}
	return TJS_S_OK;
}

tjs_error
CLASSNAME::setPartValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 2) {
		return TJS_E_BADPARAMCOUNT;
	}
  if (self->mModel) {
	    bool value = params[1]->AsInteger() != 0;
		if (params[0]->Type() == tvtInteger) {
			self->mModel->SetPartValue(params[0]->AsInteger(), value);
		} else if (params[0]->Type() == tvtString) {
			ttstr s = params[0]->GetString();
			std::string _label = convertTtstrToUtf8String(s);
			self->mModel->SetPartValue(_label.c_str(), value);
		}
	}
	return TJS_S_OK;
}

void 
CLASSNAME::setPartFadeTime(float fadeTime)
{
	if (mModel) {
		mModel->SetPartFadeTime(fadeTime);
	}
}

int 
CLASSNAME::getEventCount()
{
	if (mModel) {
		return mModel->GetEventCount();
	}
	return 0;
}

ttstr 
CLASSNAME::getEventName(int no)
{
  if (mModel) {
		return convertUtf8StringToTtstr(mModel->GetEventName(no));
  }
  return "";
}

tjs_error 
CLASSNAME::addVariableMotion(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, CLASSNAME *self)
{
	if (numparams < 3) {
		return TJS_E_BADPARAMCOUNT;
	}
	if (self->mModel) {
		float min = 0.0;
		if (numparams >= 4) {
			min = params[3]->AsReal();
		}
		float max = 1.0;
		if (numparams >= 5) {
			max = params[4]->AsReal();
		}
		bool decrete = false;
		if (numparams >= 6) {
			decrete = params[5]->AsInteger() != 0;
		}
		std::string vname = convertTtstrToUtf8String(ttstr(params[0]->GetString()));
		std::string group = convertTtstrToUtf8String(ttstr(params[1]->GetString()));
		if (params[2]->Type() == tvtInteger) {
			int no = params[2]->AsInteger();
			self->mModel->AddVariableMotion(vname.c_str(), group.c_str(), no, min, max, decrete);
		} else if (params[2]->Type() == tvtString) {
			std::string name = convertTtstrToUtf8String(ttstr(params[2]->GetString()));
			self->mModel->AddVariableMotion(vname.c_str(), group.c_str(), name.c_str(), min, max, decrete);
		}
	}
	return TJS_S_OK;
}

void 
CLASSNAME::delVariableMotion(const ttstr varname)
{
	if (mModel) {
		std::string vname = convertTtstrToUtf8String(varname);
		mModel->DelVariableMotion(vname.c_str());
	}
}

int 
CLASSNAME::getVariableMotionCount()
{
    if (mModel) { 
		return mModel->VariableMotionCount();
	}
	return 0;
}
  
ttstr 
CLASSNAME::getVariableMotionName(int index)
{
    if (mModel) { 
		return convertUtf8StringToTtstr(mModel->VariableMotionName(index));
    }
	return "";
}

tTJSVariant
CLASSNAME::getVariableMotionInfo(int index)
{
	tTJSVariant result;
    if (mModel) { 
		iTJSDispatch2 *obj;
		if ((obj = TJSCreateDictionaryObject()) != NULL) {	
			setStrProp(obj, TJS_W("name"), convertUtf8StringToTtstr(mModel->VariableMotionName(index)));
			float min;
			float max;
			bool descrete;
			mModel->VariableMotionInfo(index, min, max, descrete);
			setFloatProp(obj, TJS_W("min"), min);
			setFloatProp(obj, TJS_W("max"), max);
			setIntProp(obj, TJS_W("descrete"), descrete?1:0);
			result = tTJSVariant(obj, obj);
			obj->Release();
		}
    }
	return result;
}

void 
CLASSNAME::setVariable(const ttstr varname, tjs_real value)
{
    if (mModel) { 
		std::string vname = convertTtstrToUtf8String(varname);
		return mModel->SetVariable(vname.c_str(), (float)value);
    }
}

tjs_real
CLASSNAME::getVariable(const ttstr varname)
{
    if (mModel) { 
		std::string vname = convertTtstrToUtf8String(varname);
		return mModel->GetVariable(vname.c_str());
    }
	return 0.0;
}

bool
CLASSNAME::isMosaicModel()
{
    if (mModel) { 
		return mModel->IsMosaicModel();
    }
	return false;
}

