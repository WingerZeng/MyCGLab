#pragma once
#include <vector>
#include <string>
#include <ctime>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
namespace mcl {
	/* Some template utilities */
	// 类型名称转字符串
	template <typename T>
	struct TypeName
	{
		static const char* Get()
		{
			return typeid(T).name();
		}
	};

	// 通过实参tuple调用给定函数
	template<typename Func, typename Tup>
	decltype(auto) invoke(Func&& func, Tup&& t)
	{
		constexpr auto size = std::tuple_size<typename std::decay<Tup>::type>::value;
		// 创建参数个数size大小的模版数组
		return invoke_impl(std::forward<Func>(func), std::forward<Tup>(t), std::make_index_sequence<size>{});
	}

	template<typename Func, typename Tup, std::size_t... Index>
	decltype(auto) invoke_impl(Func&& func, Tup&& t, std::index_sequence<Index...>)
	{
		// Index为之前打包的size大小模版数组0,1,2,3。解包扩展后即成为size个参数
		return func(std::get<Index>(std::forward<Tup>(t))...);
	}

	//需要先进行模版声明，才能进行下面的特化
	template<typename T>
	struct function_traits {};

	// 获取函数信息
	template<typename R, typename ...Args>
	struct function_traits<std::function<R(Args...)>>
	{
		function_traits() = delete;
		static const size_t nargs = sizeof...(Args); // 形参数量

		typedef R result_type; // 返回类型
		typedef std::tuple<Args...> argTuple_t; //形参tuple类型

		// 输入参数类型,i为从0开始的参数类型索引
		template <size_t i>
		struct arg
		{
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
		};

		// 构造函数实参tuple
		template <int CountLast, int ArgCount, typename ArgsTupel>
		struct TupleConstructor
		{
			typedef typename std::tuple_element<ArgCount - CountLast, ArgsTupel>::type argtype;
			// 通过ios输入构造函数实参
			static int construct(ArgsTupel& args, std::iostream& ios) {
				argtype aArg;
				if (!(ios >> aArg)) return -1;
				std::get<ArgCount - CountLast>(args) = aArg;
				return TupleConstructor<CountLast - 1, ArgCount, ArgsTupel>::construct(args, ios);
			}
			// 解析函数形参类型字符串数组
			static void getArgNames(std::vector<std::string>& namevec) {
				namevec.push_back(TypeName<argtype>::Get());
				return TupleConstructor<CountLast - 1, ArgCount, ArgsTupel>::getArgNames(namevec);
			}
		};

		// 构造函数实参tuple，模版递归终止条件
		template <int ArgCount, typename ArgsTupel>
		struct TupleConstructor<0, ArgCount, ArgsTupel>
		{
			// 通过ios输入构造函数实参
			static int construct(ArgsTupel& args, std::iostream& ios) {
				return 0;
			}
			// 解析函数形参类型字符串数组
			static void getArgNames(std::vector<std::string>& namevec) {
				return;
			}
		};
	};

	//将类的非静态成员函数转化为std::function形式，方便处理
	template<typename T,typename R, typename ...Args>
	std::function<R(Args...)> classFuncToStdFunc(T* PtrThis,R(T::*func)(Args...)) {
		auto lmda = [PtrThis, func](Args... args) -> R{ //注意可变实参args的声明与使用
			return std::bind(func, PtrThis, args...)();
		};
		return std::function<R(Args...)>(lmda);
	}

	// 通过标准库输入调用给定函数
	template<typename R, typename ...Args>
	int callFuncByInput(std::function<R(Args...)>& func, std::iostream& ios) {
		//此处去除参数中的const与reference，防止tuple默认构造失败
		typedef function_traits<std::function<R(std::remove_const_t<std::remove_reference_t<Args>>...)>> ftraits;
		typename ftraits::argTuple_t args;
		int ret = ftraits::TupleConstructor<ftraits::nargs, ftraits::nargs, ftraits::argTuple_t>::construct(args, ios);
		if (ret) return -1;
		//#TODO2 现在命令行自动调用不会检查函数是否失败，只检查输入是否正确
		return int(invoke(func, args));
	}

	//// 通过标准库输入调用给定函数
	template<typename R, typename ...Args>
	std::vector<std::string> getFuncArgTypeNames(std::function<R(Args...)>& func) {
		//此处去除参数中的const与reference，防止tuple默认构造失败
		typedef function_traits<std::function<R(std::remove_const_t<std::remove_reference_t<Args>>...)>> ftraits;
		std::vector<std::string> argTypeNames;
		ftraits::TupleConstructor<ftraits::nargs, ftraits::nargs, ftraits::argTuple_t>::getArgNames(argTypeNames);
		return argTypeNames;
	}

	enum ClockOwner {
		ZBUFFER_SWEEPLINE					= 0,		//扫描线zbuffer算法总耗时
		ZBUFFER_STRATIFY					,			//层次zbuffer算法总耗时
		ZBUFFER_STRATIFY_OCTREE				,			//层次zbuffer+Octree 算法总耗时

		ZBUFFER_SWEEPLINE_CULL				,			//扫描线zbuffer算法消隐与光栅化耗时
		ZBUFFER_STRATIFY_CULL				,			//层次zbuffer算法消隐与光栅化耗时
		ZBUFFER_STRATIFY_OCTREE_CULL		,			//层次zbuffer+Octree 算法消隐与光栅化耗时

		ZBUFFER_SWEEPLINE_SHADER,						//扫描线zbuffer算法着色耗时
		ZBUFFER_STRATIFY_SHADER,						//层次zbuffer算法着色耗时
		ZBUFFER_STRATIFY_OCTREE_SHADER,					//层次zbuffer+Octree 算法着色耗时
	
		RAYTRACER
	};

	/**
	 * @brief 时间统计工具
	 */
	template<int index>
	class Clock {
	public:
		static void clear();
		static void start();
		static void stop();
		static double millsecond(); //返回时间，以毫秒为单位
	private:
		static std::clock_t total_;
		static std::clock_t start_;
	};

	template<int index> std::clock_t Clock<index>::total_ = 0;
	template<int index> std::clock_t Clock<index>::start_ = 0;

	template<int index>
	inline void Clock<index>::start()
	{
		start_ = std::clock();
	}

	template<int index>
	inline void Clock<index>::clear()
	{
		total_ = 0;
	}

	template<int index>
	inline void Clock<index>::stop()
	{
		total_ += std::clock() - start_;
	}

	template<int index>
	inline double Clock<index>::millsecond()
	{
		return total_ * 1.0 / CLOCKS_PER_SEC * 1000;
	}


	/***************
		标准库相关工具
	***************/

	inline std::string readRemainAll(std::istream& in) {
		std::string ret;
		getline(in, ret, '\0');
		return ret;
	}

	/***************
		调试工具
	***************/
	template<int T>
	class DebugRecord {
	public:
		static int hitTime() { return hittime_; }
		static void hit() { hittime_++; }

	private:
		static int hittime_;
	};

	template<int T>
	__declspec(selectany) int DebugRecord<T>::hittime_ = 0;

}