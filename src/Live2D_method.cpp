	NCB_METHOD(getExpressionCount);
	NCB_METHOD(getExpressionName);
	NCB_METHOD(setExpression);
	NCB_METHOD(getExpression);
	NCB_METHOD(fixExpression);

	NCB_METHOD(getMotionGroupCount);
	NCB_METHOD(getMotionGroupName);
	NCB_METHOD(getMotionCount);
	NCB_METHOD(getMotionName);
	RawCallback(TJS_W("startMotion"), &Class::startMotion, 0);
	NCB_METHOD(stopMotion);
	NCB_METHOD(getCurrentMotions);

	NCB_METHOD(getParameterCount);
	NCB_METHOD(getParameterInfo);
	RawCallback(TJS_W("getParameterValue"), &Class::getParameterValue, 0);
	RawCallback(TJS_W("setParameterValue"), &Class::setParameterValue, 0);
	RawCallback(TJS_W("setParameterType"), &Class::setParameterType, 0);
	RawCallback(TJS_W("setDiffParameterValue"), &Class::setDiffParameterValue, 0);

	NCB_METHOD(progress);
	NCB_METHOD(canSync);
	NCB_METHOD(sync);

	RawCallback(TJS_W("addLipSyncId"), &Class::addLipSyncId, 0);
	RawCallback(TJS_W("addEyeBlinkId"), &Class::addEyeBlinkId, 0);

	NCB_METHOD(isPlaying);

	NCB_METHOD(getPartCount);
	NCB_METHOD(getPartInfo);
	RawCallback(TJS_W("getPartValue"), &Class::getPartValue, 0);
	RawCallback(TJS_W("setPartValue"), &Class::setPartValue, 0);
	NCB_METHOD(setPartFadeTime);

	NCB_METHOD(getEventCount);
	NCB_METHOD(getEventName);

	RawCallback(TJS_W("addVriableMotion"), &Class::addVariableMotion, 0);
	NCB_METHOD(delVariableMotion);
	NCB_METHOD(getVariableMotionCount);
	NCB_METHOD(getVariableMotionName);
	NCB_METHOD(getVariableMotionInfo);
	NCB_METHOD(getVariable);
	NCB_METHOD(setVariable);

	NCB_METHOD(isMosaicModel);
