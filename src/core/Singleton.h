#pragma once

template<class T>
class Singleton {
public:
	static T* getSingleton() {
		if (!singleton) singleton = new T;
		return singleton;
	};
protected:
	static T* singleton;
};

template<class T>
 T* Singleton<T>::singleton = nullptr;

