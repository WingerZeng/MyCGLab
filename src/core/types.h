#pragma once
#include "mcl.h"
#include <iostream>
#include <QVector3D>
#include <glog/logging.h>
#include <Eigen/Eigen>
#include <QMatrix4x4>
#include <QColor>
#include <QFile>
#include <cctype>
#include "mcl.h"

#define PRINTINFOMATION
#ifdef PRINTINFOMATION
#define PRINTINFO(viable)\
	std::cout<<#viable<<": "<<(viable)<<std::endl
#else
#define PRINTINFO(viable)
#endif // DEBUG


namespace mcl {
	template <typename T>
	inline bool isNaN(const T x) {
		return std::isnan(x);
	}
	template <>
	inline bool isNaN(const int x) {
		return false;
	}

	/**
	 * @class 封装底层数据的接口，目前底层数据采用eigen库
	 */
	template <class dataT,class typeT>
	class TypeWithData 
	{
	public:
		TypeWithData(const dataT& data):dat(data){}
		TypeWithData() = default;
		typeT operator+(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			return typeT(dat + v.dat);
		}
		typeT operator-(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			return typeT(dat - v.dat);
		}
		template <typename U>
		typeT operator*(U f) const {
			return typeT(dat*f);
		}
		template <typename U>
		typeT operator+(U f) const {
			return typeT((*this) + typeT(f));
		}
		template <typename U>
		typeT operator-(U f) const {
			return typeT((*this) - typeT(f));
		}

		template <typename U>
		typeT operator/(U f) const {
			CHECK_NE(f, 0);
			return typeT(dat/f);
		}
		//#PERF1 下面这些类型转换后续需要优化，可以把模板类换成宏定义
		typeT& operator+=(const typeT &v){
			DCHECK(!v.HasNaNs());
			dat += v.dat;
			return *(typeT*)(this);
		}
		typeT& operator-=(const typeT &v){
			DCHECK(!v.HasNaNs());
			dat -= v.dat;
			return *(typeT*)(this);
		}
		template <typename U>
		typeT& operator*=(U f) {
			DCHECK(!isNaN(f));
			dat *= f;
			return *(typeT*)(this);
		}
		template <typename U>
		typeT &operator/=(U f) {
			CHECK_NE(f, 0);
			dat /= f;
			return *(typeT*)(this);
		}
		bool operator==(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			return dat == v.dat;
		}

		bool operator!=(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			dat -= v.dat;
			return *(typeT*)(this);
		}

		typeT operator-() const {
			return typeT(-dat);
		}

		dataT& rawData() {
			return dat;
		}

		const dataT& rawData() const {
			return dat;
		}
	protected:
		static const int tolerance = MachineEpsilon*1e2;
		dataT dat;
	};

	/**
	 * @class 针对向量类进一步进行封装
	 */
	template <typename elemT, typename dataT, typename typeT, int Size>
	class TypeWithSizedRawData : public TypeWithData<dataT, typeT> {
	};

	template <typename elemT, typename dataT, typename typeT>
	class TypeWithSizedRawData <typename elemT, typename dataT, typename typeT, 2> : public TypeWithData<dataT, typeT>
	{
	public:
		using TypeWithData<dataT, typeT>::TypeWithData;
		TypeWithSizedRawData()
			: TypeWithData<dataT, typeT>() { dat << 0, 0; }
		bool operator<(const TypeWithSizedRawData& rhs) const {
			if (rhs.x() - x() > tolerance) return true;
			if (x() - rhs.x() > tolerance) return false;
			if (rhs.y() - y() > tolerance) return true;
			return false;
		}
		explicit TypeWithSizedRawData(const elemT& xx, const elemT& yy)
			: TypeWithData<dataT, typeT>()
		{
			dat << xx, yy;
			DCHECK(!HasNaNs());
		}
		explicit TypeWithSizedRawData(std::istream& in)
			: TypeWithData<dataT, typeT>()
		{
			in >> x() >> y();
			DCHECK(!HasNaNs());
		}
		explicit TypeWithSizedRawData(elemT val) {
			dat << val, val;
			DCHECK(!HasNaNs());
		}
		bool HasNaNs() const {
			return isNaN((*this)[0]) || isNaN((*this)[1]);
		}
		elemT operator[](int i) const {
			DCHECK(i >= 0 && i <= 1);
			return dat[i];
		}
		elemT &operator[](int i) {
			DCHECK(i >= 0 && i <= 1);
			return dat[i];
		}
		elemT& x() {
			return dat.x();
		}
		elemT& y() {
			return dat.y();
		}
		const elemT& x() const {
			return dat.x();
		}
		const elemT& y() const {
			return dat.y();
		}
		elemT dot(const typeT& rhs)const {
			return dat.dot(rhs.dat);
		}
		elemT absDot(const typeT& rhs)const {
			return std::abs(dat.dot(rhs.dat));
		}
		typeT cross(const typeT& rhs) const {
			return typeT(dat.cross(rhs.dat));
		}
		void normalize() {
			(*this) /= length();
		}
		bool normalized() const{
			return std::abs(this->length() - 1) <= FloatZero * 1e1;
		}
		elemT lengthSquared() const { return x() * x() + y() * y(); }
		elemT length() const { return std::sqrt(LengthSquared()); }
		typeT inverse() const {
			return typeT(1 / x(), 1 / y());
		}
		int absMaxDim() const{
			if (std::abs(x()) >= std::abs(y())) {
				return 0;
			}
			return 1;
		}
		elemT max() const {
			return std::max(x(), y());
		}
		typeT maxWith(const typeT& rhs) const {
			return typeT(std::max(x(),rhs.x()), std::max(y(),rhs.y()));
		}

		typeT operator&(const typeT& rhs) {
			return typeT(x()*rhs.x(), y()*rhs.y());
		}

		inline static std::vector<typeT> fromFloatVec(std::vector<Float> vec);
	};

	template <typename elemT, typename dataT, typename typeT>
	std::vector<typeT> mcl::TypeWithSizedRawData<elemT, dataT, typeT, 2>::fromFloatVec(std::vector<Float> vec)
	{
		std::vector<typeT> retvec;
		for (int i = 0; i < vec.size()/2; i++) {
			retvec.push_back(typeT(vec[2 * i], vec[2 * i + 2]));
		}
		return retvec;
	}

	template <typename elemT, typename dataT, typename typeT>
	class TypeWithSizedRawData <typename elemT, typename dataT, typename typeT, 3>: public TypeWithData<dataT, typeT>
	{
	public:
		using TypeWithData<dataT, typeT>::TypeWithData;
		TypeWithSizedRawData() 
			: TypeWithData<dataT, typeT>() { dat << 0, 0, 0; }
		bool operator<(const TypeWithSizedRawData& rhs) const {
			// 比较x
			if (rhs.x() - x() > tolerance) return true;
			if (x() - rhs.x() > tolerance) return false;
			// x相等，比较y
			if (rhs.y() - y() > tolerance) return true;
			if (y() - rhs.y() > tolerance) return false;
			// y相等，比较z
			if (rhs.z() - z() > tolerance) return true;
			return false;
		}
		explicit TypeWithSizedRawData(const elemT& xx, const elemT& yy, const elemT& zz)
			: TypeWithData<dataT, typeT>() 
		{
			dat << xx, yy, zz;
			DCHECK(!HasNaNs());
		}
		explicit TypeWithSizedRawData(const QVector3D& qvec)
			:TypeWithData<dataT, typeT>()
		{
			dat << qvec.x(), qvec.y(), qvec.z();
			DCHECK(!HasNaNs());
		}
		explicit TypeWithSizedRawData(std::istream& in)
			: TypeWithData<dataT, typeT>()
		{
			in >> x() >> y() >> z();
			DCHECK(!HasNaNs());
		}
		explicit TypeWithSizedRawData(elemT val) {
			dat << val, val, val;
			DCHECK(!HasNaNs());
		}
		bool HasNaNs()  const {
			return isNaN((*this)[0]) || isNaN((*this)[1]) || isNaN((*this)[2]);
		}
		elemT operator[](int i) const {
			DCHECK(i >= 0 && i <= 2);
			return dat[i];
		}
		elemT &operator[](int i) {
			DCHECK(i >= 0 && i <= 2);
			return dat[i];
		}
		elemT& x() {
			return dat.x();
		}
		elemT& y() {
			return dat.y();
		}
		elemT& z() {
			return dat.z();
		}
		const elemT& x() const {
			return dat.x();
		}
		const elemT& y() const {
			return dat.y();
		}
		const elemT& z() const {
			return dat.z();
		}
		elemT dot(const typeT& rhs)const {
			return dat.dot(rhs.dat);
		}
		elemT absDot(const typeT& rhs)const {
			return std::abs(dat.dot(rhs.dat));
		}
		typeT cross(const typeT& rhs) const {
			return typeT(dat.cross(rhs.dat));
		}
		void normalize(){
			(*this) /= length();
		}
		bool normalized() const{
			return std::abs(this->length() - 1) <= FloatZero;
		}
		elemT lengthSquared() const { return x() * x() + y() * y() + z() * z(); }
		elemT length() const { return std::sqrt(lengthSquared()); }
		typeT inverse() const {
			return typeT(1 / x(), 1 / y(), 1 / z());
		}
		int absMaxDim() const{
			if (std::abs(x()) >= std::abs(y()) && std::abs(x()) >= std::abs(z())) {
				return 0;
			}
			if (std::abs(y()) >= std::abs(z())) {
				return 1;
			}
			return 2;
		}
		void swapDim(const int& dim1, const int& dim2) {
			std::swap(operator[](dim1), operator[](dim2));
		}

		using TypeWithData<dataT, typeT>::operator *;
		typeT operator*(const typeT& rhs) {
			return typeT(rhs.x()*x(), rhs.y()*y(), rhs.z()*z());
		}

		using TypeWithData<dataT, typeT>::operator *=;
		typeT& operator*=(const typeT& rhs) {
			x() *= rhs.x();
			y() *= rhs.y();
			z() *= rhs.z();
			return  *(typeT*)(this);
		}
		elemT max() const {
			return std::max({ x(), y(), z() });
		}
		typeT maxWith(const typeT& rhs) const {
			return typeT(std::max(x(), rhs.x()), std::max(y(), rhs.y()), std::max(z(), rhs.z()));
		}

		typeT operator&(const typeT& rhs) {
			return typeT(x()*rhs.x(), y()*rhs.y(), z()*rhs.z());
		}

		inline static std::vector<typeT> fromFloatVec(std::vector<Float> vec);

		QString toQString() {
			return QString::number(x()) + "," + QString::number(y()) + "," + QString::number(z());
		}
	};

	template <typename elemT, typename dataT, typename typeT>
	std::vector<typeT> mcl::TypeWithSizedRawData<elemT, dataT, typeT, 3>::fromFloatVec(std::vector<Float> vec)
	{
		std::vector<typeT> retvec;
		for (int i = 0; i < vec.size() / 3; i++) {
			retvec.push_back(typeT(vec[3 * i], vec[3 * i + 1], vec[3 * i + 2]));
		}
		return retvec;
	}

	template<class T>
	T  Normalize(const T& rhs) {
		return rhs / rhs.length();
	}

	// 将向量、点、法向以不同类进行封装，主要目的是为了区分不同的仿射变换逻辑
	// Vector Declarations
	template <typename T>
	class Vector2: public TypeWithSizedRawData<T,Eigen::Matrix<T, 2, 1>,Vector2<T>,2 >
	{
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 2, 1>, Vector2<T>, 2 > father_t;
		using father_t::father_t;
		Vector2() :father_t() {};


	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Vector2<T> &v) {
		os << "[ " << v[0] << ", " << v[1] << " ]";
		return os;
	}

	template <typename T>
	class Vector3 : public TypeWithSizedRawData<T,Eigen::Matrix<T, 3, 1>, Vector3<T>, 3 >
	{
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Vector3<T>, 3 > father_t;
		using father_t::father_t;
		Vector3() :father_t() {};
		
		explicit Vector3(const Normal3<T> &n)
			:father_t(n.x(), n.y(), n.z())
		{
			DCHECK(!n.HasNaNs());
		}

		explicit Vector3(const QColor& qc)
			:father_t(qc.redF(), qc.greenF(), qc.blueF())
		{

		}

		explicit operator QVector3D() const { return QVector3D(x(), y(), z()); }

		explicit Vector3(std::initializer_list<T> list)
		{
			auto it = list.begin();
			x() = *it;
			y() = *(++it);
			z() = *(++it);
		}

		Eigen::Matrix<T, 4, 1> to4d() const{
			Eigen::Matrix<T, 4, 1> ret;
			ret[0] = x();
			ret[1] = y();
			ret[2] = z();
			ret[3] = 0;
			return ret;
		}
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Vector3<T> &v) {
		os << "[ " << v.x() << ", " << v.y() << ", " << v.z() << " ]";
		return os;
	}

	// Point Declarations
	template <typename T>
	class Point2 : public TypeWithSizedRawData<T, Eigen::Matrix<T, 2, 1>, Point2<T>, 2 > 
	{
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 2, 1>, Point2<T>, 2 >  father_t;
		using father_t::father_t;
		Point2() :father_t() {};

		template <typename U>
		explicit Point2(const Point2<U> &p)
		{
			dat << T(p.x()), T(p.y());
			DCHECK(!HasNaNs());
		}

		explicit Point2(const Vector2<T>& v)
			:Point2(v.x(), v.y()) {};

		using father_t::operator -;
		Vector2<T> operator-(const Point2<T>& rhs) const{
			return Vector2<T>(dat - rhs.dat);
		}

		using father_t::operator +;
		Point2<T> operator+(const Vector2<T>& rhs) const {
			return father_t::operator+(Point2<T>(rhs));
		}
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Point2<T> &v) {
		os << "[ " << v.x() << ", " << v.y()<< " ]";
		return os;
	}

	template <typename T>
	class Point3 : public TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Point3<T>, 3 > {
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Point3<T>, 3 > father_t;
		using father_t::father_t;
		Point3() :father_t() {};

		explicit Point3(const Vector3<T> v)
			:Point3(v.x(), v.y(), v.z()) {
		}

		template <typename U>
		explicit Point3(const Point3<U> &p)
		{
			dat << p.x(), p.y(), p.z();
			DCHECK(!HasNaNs());
		}

		explicit Point3(const QVector3D& qvec)
		{
			dat << qvec.x(), qvec.y(), qvec.z();
		}

		explicit operator QVector3D() const { return QVector3D(x(), y(), z()); }
		template <typename U>
		explicit operator Vector3<U>() const {
			return Vector3<U>(x(), y(), z());
		}

		using father_t::operator -;
		Vector3<T> operator-(const Point3<T>& rhs) const{
			return Vector3<T>(dat - rhs.dat);
		}

		Point3<T> operator+(const Vector3<T>& rhs) const {
			return father_t::operator+(Point3<T>(rhs));
		}

		Eigen::Matrix<T, 4, 1> to4d() const{
			Eigen::Matrix<T, 4, 1> ret;
			ret[0] = x();
			ret[1] = y();
			ret[2] = z();
			ret[3] = 1;
			return ret;
		}

		using father_t::operator +;
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Point3<T> &v) {
		os << "[ " << v.x() << ", " << v.y() << ", " << v.z() << " ]";
		return os;
	}

	// Normal Declarations
	template <typename T>
	class Normal3 : public TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Normal3<T>, 3 > {
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Normal3<T>, 3 > father_t;
		using father_t::father_t;
		Normal3() :father_t() {};

		explicit Normal3<T>(const Vector3<T> &v) {
			dat << v.x(), v.y(), v.z();
			DCHECK(!v.HasNaNs());
		}

		explicit Normal3<T>(const Point3<T> &v) {
			dat << v.x(), v.y(), v.z();
			DCHECK(!v.HasNaNs());
		}

		Eigen::Matrix<T, 4, 1> to4d() const {
			Eigen::Matrix<T, 4, 1> ret;
			ret[0] = x();
			ret[1] = y();
			ret[2] = z();
			ret[3] = 0;
			return ret;
		}
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Normal3<T> &v) {
		os << "[ " << v.x() << ", " << v.y() << ", " << v.z() << " ]";
		return os;
	}

	typedef Normal3<Float> Normal3f;

	// Geometry Inline Functions
	template <typename T, typename U>
	inline Vector3<T> operator*(U f, const Vector3<T> &v) {
		return Vector3<T>(T(f*v.x()), T(f*v.y()), T(f*v.z()));
	}

	template <typename T, typename U>
	inline Vector2<T> operator*(U f, const Vector2<T> &v) {
		return Vector2<T>(T(f*v.x()), T(f*v.y()));
	}

	template <typename T, typename U>
	inline Point3<T> operator*(U f, const Point3<T> &p) {
		DCHECK(!p.HasNaNs());
		return Point3<T>(T(f*p.x()), T(f*p.y()), T(f*p.z()));
	}

	template <typename T, typename U>
	inline Point2<T> operator*(U f, const Point2<T> &p) {
		DCHECK(!p.HasNaNs());
		return Point2<T>(T(f*p.x()), T(f*p.y()));
	}

	template <typename T, typename U>
	inline Normal3<T> operator*(U f, const Normal3<T> &n) {
		return Normal3<T>(T(f * n.x()), T(f * n.y()), T(f * n.z()));
	}

	template <typename T>
	inline Point3<T> operator+(const Point3<T> & pt, const Vector3<T> & vec) {
		return Point3<T>(pt.x() + vec.x(), pt.y() + vec.y(), pt.z() + vec.z());
	}

	template <typename T>
	inline Point3<T> operator+(const Vector3<T> & vec, const Point3<T> & pt) {
		return Point3<T>(pt.x() + vec.x(), pt.y() + vec.y(), pt.z() + vec.z());
	}

	//global typedef
	typedef Vector2<Float> Vector2f;
	typedef Vector2<int> Vector2i;
	typedef Vector3<Float> Vector3f;
	typedef Vector3<int> Vector3i;
	typedef Point2<Float> Point2f;
	typedef Point2<int> Point2i;
	typedef Point2<short> Point2s;
	typedef Point3<Float> Point3f;
	typedef Point3<int> Point3i;
	typedef Vector3f Vec3f;
	typedef Point3f PType3f;
	typedef Vec3f Color3f;
	template<class T> class ListElement;

	inline int genOrthoCoordinateSystem(const Vector3f& vec1, Vector3f& vec2, Vector3f& vec3) {
		if (vec1 == Vector3f(0.0f)) {
			return -1;
		}
		int dim = vec1.absMaxDim();
		Vector3f tempvec(1, 1, 1);
		tempvec[dim] = 0;
		vec2 = (vec1.cross(tempvec));
		assert(vec2.length() >= 1e-6);
		vec2.normalize();
		vec3 = vec1.cross(vec2);
		vec3.normalize();
		return 0;
	}

	//Color Functions
	inline bool isDark(const Color3f& color) {
		return color.x() <= 0 && color.y() <= 0 && color.z() <= 0;
	}

	inline Color3f operator&(const Color3f& lhs,const Color3f& rhs) {
		return Color3f(lhs.x()*rhs.x(), lhs.y()*rhs.y(), lhs.z()*rhs.z());
	}

	inline Color3f operator/(const Color3f& lhs, const Color3f& rhs) {
		return Color3f(lhs.x()/rhs.x(), lhs.y()/rhs.y(), lhs.z()/rhs.z());
	}

	inline Color3f limitColor(const Color3f& color) {
		return Color3f(std::clamp(color.x(), 0.0f, 1.0f),
			std::clamp(color.y(), 0.0f, 1.0f),
			std::clamp(color.z(), 0.0f, 1.0f));
	}

	//链表类
	template<class T>
	class ListElementIterator {
	public:
		ListElementIterator(ListElement<T>* elem)
			:elem_(elem->get()) {}
		ListElementIterator<T>& operator++() {
			elem_ = elem_->next;
			return *this;
		}
		ListElementIterator<T> operator++(int) {
			T* pevElem = elem_;
			elem_ = elem_->next;
			return ListElementIterator (pevElem);
		}
		ListElementIterator<T>& operator--() {
			elem_ = elem_->prev;
			return *this;
		}
		ListElementIterator<T> operator--(int) {
			T* nxtElem = elem_;
			elem_ = elem_->prev;
			return ListElementIterator(nxtElem);
		}
		bool operator==(const ListElementIterator<T>& rhs) const{
			return elem_ == rhs.elem_;
		}
		bool operator!=(const ListElementIterator<T>& rhs) const {
			return elem_ != rhs.elem_;
		}
		ListElementIterator<T>& operator=(const ListElementIterator<T>& rhs) {
			elem_ = rhs.elem_;
			return *this;
		}
		T*& operator*() {
			return elem_;
		}
		explicit operator bool() {
			return bool(this->elem_);
		}
		bool operator!() {
			return !(elem_);
		}
	private:
		T* elem_;
	};

	template<class T>
	class ListElement {
	public:
		typedef ListElementIterator<T> iterator;
		iterator begin() {
			ListElement<T>* it = this;
			while (it->prev)
			{
				it = it->prev;
			}
			return iterator(it);
		}

		iterator end() {
			return iterator(nullptr);
		}

		void printList() {
			for (auto it = begin(); it != end() ;it++)
			{
				qDebug() << (*it);
			}
		}

		iterator before_end() {
			ListElement<T>* it = this;
			while (it->next)
			{
				it = it->next;
			}
			return iterator(it);
		}

		int size() {
			int i = 0;
			for (auto it = begin(); it != end(); it++, i++);
			return i;
		}

		T* get() {
			return (T*)this;
		}

		void push_back(T* val) {
			if (val) {
				val->del();
				iterator it = before_end();
				val->prev = *it;
				val->next = nullptr;
				(*it)->next = val;
			}
		}

		void del() {
			if (prev) prev->next = next;
			if (next) next->prev = prev;
			this->prev = nullptr;
			this->next = nullptr;
		}
		bool findInList(iterator elem) {
			if (!elem) return false;
			for (auto it = this->begin(); it != this->end(); it++) {
				if (elem == it) return true;
			}
			return false;
		}
		~ListElement() {
			del();
		}
		friend iterator;
		T* Prev() const { return prev; }
		T* Next() const { return next; }
	private:
		T* prev = nullptr, *next = nullptr;
	};

	template<class T>
	std::istream& operator>>(std::iostream& lhs,Vector3<T>& rhs) {
		T x, y, z;
		char c;
		std::istream& ret = (lhs >> x >> c >> y >> c >> z);
		if (ret) {
			rhs = Vector3<T>(x, y, z);
		}
		return ret;
	}

	template<class T>
	std::istream& operator>>(std::iostream& lhs, Point3<T>& rhs) {
		T x, y, z;
		char c;
		auto& ret = (lhs >> x >> c >> y >> c >> z);
		if (ret) {
			rhs = Point3<T>(x, y, z);
		}
		return ret;
	}

	inline std::istream& operator>>(std::iostream& lhs, std::vector<std::vector<Point3f>>& rhs) {
		std::string str;
		std::istream& is = std::getline(lhs,str,'}');
		int idx = str.find('{');
		if (idx == -1) {  //return error iostream
			lhs.clear();
			return lhs>>str;
		}
		str = str.substr(idx + 1);
		while (true) {
			int idx1 = str.find('(');
			if (idx1 == -1) break;
			int idx2 = str.find(')');
			if (idx2 == -1) {	//return error iostream
				lhs.clear();
				return lhs >> str;
			}
			rhs.push_back(std::vector<Point3f>());
			std::string subs = str.substr(idx1 + 1, idx2 - idx1 - 1);
			str = str.substr(idx2 + 1);
			std::stringstream sin(subs);
			Point3f pt;
			while (sin>>pt) {
				rhs.back().push_back(pt);
			}
			if (rhs.back().empty()) DLOG(WARNING) << "blank Point3f vector input!";
		}
		return is;
	}

	template<typename T>
	class Bound3 {
	public:
		Bound3() :pmin(std::numeric_limits<T>::max()),pmax(-std::numeric_limits<T>::max()){} //使用最大负包围盒进行初始化

		template<typename U>
		explicit Bound3(Bound3<U> bdu)
			:pmin(bdu.pMin()), pmax(bdu.pMax()) {

		}

		explicit Bound3(Point3<T> p)
			:pmin(p), pmax(p) {};
		explicit Bound3(Point3<T> p1, Point3<T> p2);

		Bound3& unionBd(const Bound3<T>& bound) {
			pmin = findMin(pmin, bound.pmin);
			pmax = findMax(pmax, bound.pmax);
			return *this;
		}

		Bound3& unionPt(const Point3<T>& point) {
			pmin = findMin(pmin, point);
			pmax = findMax(pmax, point);
			return *this;
		}

		Point3<T> mid()  const {
			return (pmin + pmax) / 2;
		}
		Point3<T> point(int index)  const {
			DCHECK(index >= 0 && index < 8);
			T x = (index & 1) ? pmax.x() : pmin.x();
			T y = (index & (1<<1)) ? pmax.y() : pmin.y();
			T z = (index & (1<<2)) ? pmax.z() : pmin.z();
			return Point3<T>(x,y,z);
		}
		const Point3<T>& pMin()  const {
			return pmin;
		}
		const Point3<T>& pMax()  const {
			return pmax;
		}
		Bound3<T> octreeSplit(int index)  const{
			DCHECK(index >= 0 && index < 8);
			Point3<T> p = point(index);
			return Bound3<T>((p + pmin) / 2, (p + pmax) / 2);
		}
		bool overlap(Bound3<T> rhs)  const {
			bool ox = std::max(pmin.x(), rhs.pmin.x()) <= std::min(pmax.x(), rhs.pmax.x());
			bool oy = std::max(pmin.y(), rhs.pmin.y()) <= std::min(pmax.y(), rhs.pmax.y());
			bool oz = std::max(pmin.z(), rhs.pmin.z()) <= std::min(pmax.z(), rhs.pmax.z());
			return ox && oy && oz;
		}
		bool include(const Bound3<T>& rhs) const {
			if (pMin().x() <= rhs.pMin().x() &&
				pMin().y() <= rhs.pMin().y() &&
				pMin().z() <= rhs.pMin().z() &&
				pMax().x() >= rhs.pMax().x() &&
				pMax().y() >= rhs.pMax().y() &&
				pMax().z() >= rhs.pMax().z()
				)
				return true;
			else
				return false;
		}
		Bound3 extend(const T& offset) const {
			return Bound3(pmin - Vector3f(offset, offset, offset), pmax + Vector3f(offset, offset, offset));
		}
	private:
		inline Point3<T> findMin(const Point3<T> p1, const Point3<T> p2);
		inline Point3<T> findMax(const Point3<T> p1, const Point3<T> p2);

		Point3<T> pmin, pmax;
	};

	template<typename T>
	class Bound2 {
	public:
		Bound2() :pmin(0), pmax(0) {}
		explicit Bound2(Point2<T> p1, Point2<T> p2)
			:pmin(std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y())),
			pmax(std::max(p1.x(), p2.x()), std::max(p1.y(), p2.y())) {};
		explicit Bound2(Point2<T> p)
			:pmin(p), pmax(p) {};

		explicit Bound2(Bound3<T> bd3) 
			:pmin(bd3.pMin().x(), bd3.pMin().y()),
			pmax(bd3.pMax().x(), bd3.pMax().y())
		{
		}

		template<typename U>
		explicit Bound2(Bound2<U> bdu) 
		:pmin(bdu.pMin()),pmax(bdu.pMax()){
			
		}

		const Point2<T>& pMin()  const {
			return pmin;
		}
		const Point2<T>& pMax()  const {
			return pmax;
		}

		Bound2& unionBd(const Bound2<T>& bound) {
			pmin = findMin(pmin, bound.pmin);
			pmax = findMax(pmax, bound.pmax);
			return *this;
		}

		Point2<T> mid()  const {
			return (pmin + pmax) / 2;
		}
		Point2<T> point(int index)  const {
			DCHECK(index >= 0 && index < 4);
			T x = (index & 1) ? pmax.x() : pmin.x();
			T y = (index & (1 << 1)) ? pmax.y() : pmin.y();
			Point2<T> ret(x, y);
		}
		Bound2<T> quadtreeSplit(int index)  const {
			DCHECK(index >= 0 && index < 4);
			Point2<T> p = point(index);
			return Bound2<T>((p + pmin) / 2, (p + pmax) / 2);
		}
		bool overlap(Bound2<T> rhs) const{
			bool ox = std::max(pmin.x(), rhs.pmin.x()) <= std::min(pmax.x(), rhs.pmax.x());
			bool oy = std::max(pmin.y(), rhs.pmin.y()) <= std::min(pmax.y(), rhs.pmax.y());
			return ox && oy;
		}
		bool operator==(const Bound2<T>& rhs) const{
			return (pmin == rhs.pmin) && (pmax == rhs.pmax);
		}
		bool include(const Bound2<T>& rhs) const{
			if (pMin().x() <= rhs.pMin().x() &&
				pMin().y() <= rhs.pMin().y() &&
				pMax().x() >= rhs.pMax().x() &&
				pMax().y() >= rhs.pMax().y() 
				)
				return true;
			else
				return false;
		}
	protected:
		Point2<T> pmin, pmax;
	};

	template <class T>
	class Bound2int: public Bound2<T> //面向整型的bound
	{
	public:
		using Bound2<T>::Bound2;
		explicit Bound2int(Point2f pt)
			:Bound2<T>(Point2i(std::floor(pt.x()), std::floor(pt.y())),Point2i(std::ceil(pt.x()), std::ceil(pt.y()))) {};
		
		Bound2int() :Bound2<T>() {};
	};

	typedef Bound2int<int> Bound2i;
	typedef Bound2int<short> Bound2s;
	typedef Bound2<Float> Bound2f;

	template<typename T>
	mcl::Point3<T> mcl::Bound3<T>::findMax(const Point3<T> p1, const Point3<T> p2)
	{
		return Point3<T>(std::max(p1.x(), p2.x()), std::max(p1.y(), p2.y()), std::max(p1.z(), p2.z()));
	}

	template<typename T>
	mcl::Point3<T> mcl::Bound3<T>::findMin(const Point3<T> p1, const Point3<T> p2)
	{
		return Point3<T>(std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y()), std::min(p1.z(), p2.z()));
	}

	template<typename T>
	Bound3<T>::Bound3(Point3<T> p1, Point3<T> p2) 
		:pmin(std::min(p1.x(),p2.x()), std::min(p1.y(), p2.y()), std::min(p1.z(), p2.z())),
		 pmax(std::max(p1.x(),p2.x()), std::max(p1.y(), p2.y()), std::max(p1.z(), p2.z()))
	{
	}

	typedef Bound3<Float> Bound3f;
	typedef Bound3<int> Bound3i;

	class Ray
	{
	public:
		Ray(const Point3f& o, Vector3f d, Float t0, Float t1, Float time, int depth)
			:o(o), d(d), t0(t0), t1(t1), time(time), depth(depth) {}
		Ray() = default;
		Point3f o;
		Vector3f d;
		Float time;
		Float t0, t1;
		int depth;

		mcl::Point3f pos(const Float& t) const
		{
			return o + t * d;
		}

		bool intersect(const Bound3f& bd) const
		{
			for (int i = 0; i < 3; ++i) {
				Float invd = 1 / d[i];
				Float tmin = (bd.pMin()[i] - o[i]) * invd;
				Float tmax = (bd.pMax()[i] - o[i]) * invd;
				if(invd < 0) std::swap(tmin, tmax);
				if (std::max(tmin,t0) > std::min(tmax,t1)) 
					return false;
			}
			return true;
		}

		bool intersect(const Bound3f& bd, Float& ht0, Float& ht1) const
		{
			ht0 = t0;
			ht1 = t1;
			for (int i = 0; i < 3; ++i) {
				Float invd = 1 / d[i];
				Float tmin = (bd.pMin()[i] - o[i]) * invd;
				Float tmax = (bd.pMax()[i] - o[i]) * invd;
				if (invd < 0) std::swap(tmin, tmax);
				ht0 = std::max(tmin, ht0);
				ht1 = std::min(tmax, ht1);
				if (ht0 > ht1)
					return false;
			}
			return true;
		}
	};

	class Transform :public TypeWithData<Eigen::Matrix4f, Transform> {
	public:
		Transform(const Float data[16]) {
			dat << data[0], data[1], data[2], data[3],
				data[4], data[5], data[6], data[7],
				data[8], data[9], data[10], data[11],
				data[12], data[13], data[14], data[15];
		}
		Transform(QMatrix4x4 qmat) {
			dat << qmat(0, 0), qmat(0, 1), qmat(0, 2), qmat(0, 3),
				qmat(1, 0), qmat(1, 1), qmat(1, 2), qmat(1, 3),
				qmat(2, 0), qmat(2, 1), qmat(2, 2), qmat(2, 3),
				qmat(3, 0), qmat(3, 1), qmat(3, 2), qmat(3, 3);
		}

		QMatrix4x4 toQMatrix() {
			return QMatrix4x4(	dat(0, 0), dat(0, 1), dat(0, 2), dat(0, 3),
								dat(1, 0), dat(1, 1), dat(1, 2), dat(1, 3),
								dat(2, 0), dat(2, 1), dat(2, 2), dat(2, 3),
								dat(3, 0), dat(3, 1), dat(3, 2), dat(3, 3));
		}

		using TypeWithData<Eigen::Matrix4f, Transform>::operator*;
		Transform operator*(const Transform& rhs) const {
			return Transform(dat*rhs.dat);
		}

		using TypeWithData<Eigen::Matrix4f, Transform>::operator*=;
		const Transform& operator*=(const Transform& rhs) {
			this->dat = this->dat * rhs.dat;
			return *(this);
		}

		template <typename T>
		Point3<T> operator*(const Point3<T> &p) const {
			return operator()(p);
		}

		template <typename T>
		Vector3<T> operator*(const Vector3<T> &v) const {
			return operator()(v);
		}

		template <typename T>
		Bound3<T> operator*(const Bound3<T> &b) const {
			return operator()(b);
		}

		Ray operator*(const Ray& ray)const {
			return operator()(ray);
		}

		Transform(const Eigen::Matrix4f& mat)
			:TypeWithData<Eigen::Matrix4f, Transform>(mat) {}
		Transform() {
			dat = dat.Identity(); //#PERF5 matrix的默认构造是否初始化为单位矩阵？
		}
		static inline Transform scale(const Vector3f& factor);
		static inline Transform translate(const Vector3f& delta);

		/**
		 * @brief 创建旋转变换，绕特定轴特定角度，角度单位为度
		 */
		static inline Transform rotate(const Vector3f& axis, Float angle);
		static inline Transform rotate(const Vector3f& from, const Vector3f& to);

		static inline Transform lookAt(const Point3f& pos, const Vector3f& front, const Vector3f& up);

		static inline Transform tranToCoordinateSystem(const Vector3f& vec1, const Vector3f& vec2, const Vector3f& vec3)
		{
			return Transform({	vec1.x(),vec1.y(),vec1.z(),0,
								vec2.x(),vec2.y(),vec2.z(),0,
								vec3.x(),vec3.y(),vec3.z(),0,
								0,0,0,1 });
		}

		static inline Transform tranToCoordinateSystem(const Point3f& pos, Vector3f& vec1, const Vector3f& vec2, const Vector3f& vec3)
		{
			return Transform({ vec1.x(),vec1.y(),vec1.z(),0,
								vec2.x(),vec2.y(),vec2.z(),0,
								vec3.x(),vec3.y(),vec3.z(),0,
								0,0,0,1 }) * Transform::translate(Point3f(0)-pos);
		}

		static inline Transform tranFromCoordinateSystem(const Vector3f& vec1, const Vector3f& vec2, const Vector3f& vec3)
		{
			return Transform({	vec1.x(),vec2.x(),vec3.x(),0,
								vec1.y(),vec2.y(),vec3.y(),0,
								vec1.z(),vec2.z(),vec3.z(),0,
								0,0,0,1 });
		}

		Transform inverse() const{
			return Transform(dat.inverse());
		}

		Transform transpose()  const {
			return Transform(dat.transpose());
		}

		template <typename T>
		inline Point3<T> operator()(const Point3<T> &p) const;
		template <typename T>
		inline Vector3<T> operator()(const Vector3<T> &v) const;
		template <typename T>
		inline Normal3<T> operator()(const Normal3<T> &v) const;
		inline Ray operator()(const Ray& ray) const;
		inline Bound3f operator()(const Bound3f &b) const;
		inline PixelData operator()(const PixelData& lhs);
	};

	inline Transform Transform::scale(const Vector3f& factor)
	{
#ifdef DEBUG
		std::cout << Eigen::Scaling(factor.x(), factor.y(), factor.z()) << std::endl;
#endif // DEBUG
		Float arr[16];
		for (int i = 0; i < 16; i++) {
			arr[i] = 0;
		}
		arr[0] = factor.x();
		arr[5] = factor.y();
		arr[10] = factor.z();
		arr[15] = 1;
		return Transform(arr);
	}

	inline Transform Transform::translate(const Vector3f& delta)
	{
#ifdef DEBUG
		std::cout << Eigen::Matrix4f(Eigen::Translation3f(delta.x(), delta.y(), delta.z())) << std::endl;
#endif // DEBUG
		QMatrix4x4 qmat;
		qmat.translate(QVector3D(delta));
		return Transform(qmat);
	}

	inline mcl::Transform Transform::rotate(const Vector3f& axis, Float angle)
	{
#ifdef DEBUG
		std::cout << Eigen::Matrix4f(Eigen::AngleAxisf(angle, Eigen::Vector3f(axis.x(), axis.y(), axis.z()))) << std::endl;
#endif // DEBUG
		QMatrix4x4 qmat;
		qmat.rotate(QQuaternion::fromAxisAndAngle(QVector3D(axis), angle));
		return Transform(qmat);
	}

	inline mcl::Transform Transform::rotate(const Vector3f& from, const Vector3f& to)
	{
#ifdef DEBUG
		std::cout << Eigen::Matrix4f(Eigen::AngleAxisf(angle, Eigen::Vector3f(axis.x(), axis.y(), axis.z()))) << std::endl;
#endif // DEBUG
		QMatrix4x4 qmat;
		qmat.rotate(QQuaternion::rotationTo(QVector3D(from), QVector3D(to)));
		return Transform(qmat);
	}


	inline mcl::Transform Transform::lookAt(const Point3f& pos, const Vector3f& front, const Vector3f& up) {
		Vector3f right = Normalize(front.cross(up));
		Vector3f trueup = right.cross(front);
		return tranToCoordinateSystem(pos, right, trueup, -front);
	}

	template <typename T>
	inline Normal3<T>
		Transform::operator()(const Normal3<T> &n) const
	{
		Eigen::Matrix<T, 4, 1> tempvec = n.to4d();
		Eigen::Matrix<Float, 4, 1> result = dat.inverse().transpose() * tempvec;
		return Normal3<T>(T(result.x()), T(result.y()), T(result.z()));
	}

	template <typename T>
	inline Vector3<T>
		mcl::Transform::operator()(const Vector3<T> &v) const
	{
		Eigen::Matrix<T, 4, 1> tempvec = v.to4d();
		Eigen::Matrix<Float, 4, 1> result = dat * tempvec;
		return Vector3<T>(T(result.x()), T(result.y()), T(result.z()));
	}

	template <typename T>
	inline Point3<T>
		mcl::Transform::operator()(const Point3<T> &p) const
	{
		Eigen::Matrix<T, 4, 1> tempvec = p.to4d();
		Eigen::Matrix<Float, 4, 1> result = dat * tempvec;
		Float invw = 1 / result.w();
		return Point3<T>(T(result.x()*invw), T(result.y()*invw), T(result.z()*invw));
	}

	inline Ray mcl::Transform::operator()(const Ray& ray) const{
		return Ray(operator()(ray.o), operator()(ray.d), ray.t0, ray.t1, ray.time, ray.depth);
	}

	inline Bound3f mcl::Transform::operator()(const Bound3f &b) const
	{
		Bound3f bound;
		for (int i = 0; i < 8; i++) {
			bound.unionPt((*this)(b.point(i)));
		}
		return bound;
	}

	class ReadRemainString {
	public:
		ReadRemainString() = default;
		ReadRemainString(std::string astr)
			:str(astr) {};
		ReadRemainString(const char * astr)
			:str(astr) {};
		bool empty() {
			for (const auto& c : str) {
				if (!std::isspace(c))
					return false;
			}
			return true;
		}
		std::string str;
	};

	inline bool operator>>(std::istream &is, ReadRemainString &str) {
		std::stringstream sio;
		sio << is.rdbuf();
		str.str = sio.str();
		return true;
	}

	class PathString {
	public:
		PathString() = default;
		PathString(std::string astr)
			:str(astr) {}
		bool exist() { return QFile(QString::fromStdString(str)).exists(); }
		bool empty() { return str == ""; }

		std::string str;
	};

	inline std::istream &operator>>(std::istream &is, PathString &str) {
		char c;
		str.str.clear();
		while (true)
		{
			if (!(is >> c))
				return is;
			if (c == '"')
				break;
		}
		while (true)
		{
			if (!(is >> c))
				return is;
			if (c == '"')
				break;
			str.str.push_back(c);
		}
		str.str = QString::fromStdString(str.str).replace("\\", "/").toStdString();
		return is;
	}

	struct PixelData
	{
		Color3f color;
		Normal3f normal;
		Color3f albedo;
	};

	inline PixelData& operator+=(PixelData& lhs, const PixelData& rhs){
		lhs.color += rhs.color;
		lhs.normal += rhs.normal;
		lhs.albedo += rhs.albedo;
		return lhs;
	}

	template<typename T>
	PixelData& operator/=(PixelData& lhs, const T& rhs){
		lhs.color /= rhs;
		lhs.normal /= rhs;
		lhs.albedo /= rhs;
		return lhs;
	}

	inline PixelData Transform::operator()(const PixelData& lhs){
		PixelData npd = lhs;
		npd.normal = (*this)(lhs.normal);
		return npd;
	}

}