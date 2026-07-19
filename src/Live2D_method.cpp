	NCB_METHOD(getExpressionCount);
	NCB_METHOD(getExpressionName);
	NCB_METHOD(setExpression);
	NCB_METHOD(getExpression);
	NCB_METHOD(fixExpression);

	NCB_METHOD(getMotionGroupCount);
	NCB_METHOD(getMotionGroupName);
	NCB_METHOD(getMotionCount);
	NCB_METHOD(getMotionName);
	RawCallback(L"startMotion", &Class::startMotion, 0);
	NCB_METHOD(stopMotion);
	NCB_METHOD(getCurrentMotions);

	NCB_METHOD(getParameterCount);
	NCB_METHOD(getParameterInfo);
	RawCallback(L"getParameterValue", &Class::getParameterValue, 0);
	RawCallback(L"setParameterValue", &Class::setParameterValue, 0);
	RawCallback(L"setParameterType", &Class::setParameterType, 0);
	RawCallback(L"setDiffParameterValue", &Class::setDiffParameterValue, 0);

	NCB_METHOD(progress);
	NCB_METHOD(canSync);
	NCB_METHOD(sync);

	RawCallback(L"addLipSyncId", &Class::addLipSyncId, 0);
	RawCallback(L"addEyeBlinkId", &Class::addEyeBlinkId, 0);

	NCB_METHOD(isPlaying);

	NCB_METHOD(getPartCount);
	NCB_METHOD(getPartInfo);
	RawCallback(L"getPartValue", &Class::getPartValue, 0);
	RawCallback(L"setPartValue", &Class::setPartValue, 0);
	NCB_METHOD(setPartFadeTime);

	NCB_METHOD(getEventCount);
	NCB_METHOD(getEventName);

	RawCallback(L"addVriableMotion", &Class::addVariableMotion, 0);
	NCB_METHOD(delVariableMotion);
	NCB_METHOD(getVariableMotionCount);
	NCB_METHOD(getVariableMotionName);
	NCB_METHOD(getVariableMotionInfo);
	NCB_METHOD(getVariable);
	NCB_METHOD(setVariable);

	NCB_METHOD(isMosaicModel);
