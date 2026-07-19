#include <ncbind.hpp>
#include <string>

// ttstrをUTF8文字列へ変換
std::string
convertTtstrToUtf8String(const ttstr &buf)
{
	int maxlen = buf.length() * 6 + 1;
	char *dat = new char[maxlen];
	int len = TVPWideCharToUtf8String(buf.c_str(), dat);
	std::string result(dat, len);
	delete[] dat;
	return result;
}

// std::stringをttstrに変換
ttstr
convertUtf8StringToTtstr(const char *buf, size_t length) {
	tjs_uint maxlen = (tjs_uint)length * 2 + 1;
	tjs_char *dat = new tjs_char[maxlen];
	tjs_uint len = TVPUtf8ToWideCharString(buf, dat);
	ttstr result(dat, len);
	delete[] dat;
	return result;
}

ttstr
convertUtf8StringToTtstr(const char *buf) {
	return convertUtf8StringToTtstr(buf, ::strlen(buf));
}

ttstr
convertUtf8StringToTtstr(const std::string &buf) {
	return convertUtf8StringToTtstr(buf.c_str(), buf.length());
}

