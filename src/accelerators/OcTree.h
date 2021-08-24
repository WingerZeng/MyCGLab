#pragma once
#include "mcl.h"
#include "types.h"

namespace mcl {
	class OcTree
	{
	};

	template<class T, typename BoundFunction, typename RayIntersectFunction>
	class OctreeNode;

	template<class T, typename BoundFunction, typename RayIntersectFunction>
	class OctreeQuerier;

	template<class T>
	class DefaultBoundFunction{
		Bound3f operator()(const T& rhs) {
			return Bound3f();
		}
	};

	template<class T>
	class DefaultRayIntersectFunction {
		bool operator()(const T& obj, const Ray& ray, Float& thit) {
			return false;
		}
	};

	/**
	 * @brief 八叉树模板类定义
	 */
	template <class T, typename BoundFunction = DefaultBoundFunction<T>, typename RayIntersectFunction = DefaultRayIntersectFunction<T>>
	class Octree {
	public:
		Octree(int maxLevel, int minCountPerNode = 1)
			:maxLevel_(maxLevel), minCount_(minCountPerNode) {
			DCHECK(maxLevel > 0);
		};

		typedef BoundFunction bdf_t;
		typedef OctreeNode<T, bdf_t, RayIntersectFunction> node_t;

		void addElement(const T&& elems);
		void createTree();
		void clear();

		class Querier
		{
		public:
			friend Octree<T, BoundFunction, RayIntersectFunction>;
			typedef typename std::vector<int>::iterator iterator;

			iterator begin() {
				return tree_->objIndices.begin() + begin_;
			}
			iterator end() {
				return tree_->objIndices.begin() + begin_ + size_;
			}

		private:
			Querier(int begin, int size, Octree* tree)
				:begin_(begin), size_(size), tree_(tree) {}
			int begin_, size_;
			Octree* tree_;
		};
		Querier query(const node_t& node);
		friend Querier;

		const T& getObj(int index) {
			return objs[index];
		}

		node_t* getRoot() { return root.get(); }

		bool intersect(const Ray& ray, T& obj);
	private:
		int maxLevel_;
		int minCount_;
		std::unique_ptr<node_t> root;
		std::vector<T> objs;
		std::vector<int> objIndices;
	};


	template <class T, typename BoundFunction, typename RayIntersectFunction>
	void mcl::Octree<T, BoundFunction, RayIntersectFunction>::clear()
	{
		root.reset();
		objs.clear();
		objIndices.clear();
	}

	template <class T, typename BoundFunction, typename RayIntersectFunction>
	typename mcl::Octree<T, BoundFunction, RayIntersectFunction>::Querier mcl::Octree<T, BoundFunction, RayIntersectFunction>::query(const node_t& node)
	{
		return mcl::Octree<T, BoundFunction, RayIntersectFunction>::Querier(node.offset(), node.N(), this);
	}

	template <class T, typename BoundFunction, typename RayIntersectFunction>
	void Octree<T, BoundFunction, RayIntersectFunction>::createTree()
	{
		/* initialize */
		std::vector<int> sindices; //各层节点的相交元素indices堆栈
		sindices.reserve(maxLevel_ * objs.size());
		for (int i = 0; i < objs.size(); i++) {
			sindices.push_back(i);
		}

		struct NodeIterator //前序遍历构造树时所需的数据结构
		{
			node_t* node;
			int undoChild; //first index of undo child
			int indicesOffset;
		};

		std::vector<NodeIterator> snode; //node stack
		snode.reserve(maxLevel_);

		//理论最大值是(1<<(3*maxLevel_))*objs.size()，但在实际情况中，考虑到内存问题，取一个较小值
		objIndices.reserve((1 << (maxLevel_))*objs.size());

		//计算总包围盒
		Bound3f totbound;
		bdf_t bdf;
		for (const auto& obj : objs) {
			totbound = totbound.unionBd(bdf(obj));
		}
		//#TODO2 
		totbound = totbound.extend(RayTraceZero * 1e2);

		//初始化root
		root.reset(new node_t);
		if (objs.size() <= minCount_) {
			for (int i = 0; i < objs.size(); i++) {
				objIndices.push_back(i);
			}
			objIndices.shrink_to_fit();
			root->initLeaf(totbound, 0, objs.size());
			return;
		}
		root->initInner(totbound);
		snode.push_back({ root.get(),0,0 });

		/* 前序遍历构造树 */
		while (!snode.empty())
		{
			auto& curit = snode.back();
			//边界情况
			if (curit.undoChild == 8) {
				sindices.erase(sindices.begin() += curit.indicesOffset, sindices.end());
				snode.pop_back();
				continue;
			}
			//进入下个子节点，统计相交的元素
			const auto& bound = curit.node->bound().octreeSplit(curit.undoChild);
			const int curOffset = sindices.size();
			for (int i = curit.indicesOffset; i < curOffset; i++) {
				if (bdf(objs[sindices[i]]).overlap(bound)) {
					sindices.push_back(sindices[i]);
				}
			}
			//分叶节点和内部节点情况处理
			int nlevel = snode.size();
			node_t* nxtNode = curit.node->child(curit.undoChild);

			//判断若子节点的单元大部分横跨多个单元，则设该子节点为叶节点
			int overlapCount = 0;
			for (int i = 0; i < 8; i++) {
				const auto& cbound = bound.octreeSplit(i);
				for (int iobj = curOffset; iobj < sindices.size(); iobj++) {
					if(bdf(objs[sindices[iobj]]).overlap(cbound));
						overlapCount++;
				}
			}
			CHECK(overlapCount >= sindices.size()-curOffset);

			//当到达最大层数，或元素数小于阈值时，为叶节点
			if (nlevel == maxLevel_ || sindices.size() - curOffset <= minCount_ || overlapCount/(sindices.size() - curOffset) > 8) {
				nxtNode->initLeaf(bound, objIndices.size(), sindices.size() - curOffset);
				objIndices.insert(objIndices.end(), sindices.begin() + curOffset, sindices.end());
				sindices.erase(sindices.begin() + curOffset, sindices.end());
				DCHECK(nxtNode->N() + nxtNode->offset() == objIndices.size());
			}
			//其他情况为内部节点
			else {


				nxtNode->initInner(bound);
				//进入内部节点时，要压入堆栈
				snode.push_back({ nxtNode,0,curOffset });
			}

			++(curit.undoChild);
		}

		//释放多余空间
		objIndices.shrink_to_fit();
	}

	template <class T, typename BoundFunction, typename RayIntersectFunction>
	void Octree<T, BoundFunction, RayIntersectFunction>::addElement(const T&& elems)
	{
		objs.push_back(elems);
	}

	template<class T, typename BoundFunction, typename RayIntersectFunction>
	class OctreeNode {
	public:
		friend Octree<T, BoundFunction, RayIntersectFunction>;

		Bound3f bound() const {
			return bound_;
		}
		OctreeNode* child(int index) const {
			DCHECK(index >= 0 && index < 8);
			return &(children[index]);
		}

		bool isLeaf() {
			return !bool(children);
		}

		bool isInner() {
			return bool(children);
		}

		bool empty() {
			return !n_;
		}
	private:
		OctreeNode() = default;

		inline void initLeaf(const Bound3f& bound, int offset, int n) {
			DCHECK(n >= 0 && n < (1 << 30));
			bound_ = bound;
			n_ = n;
			offset_ = offset;
		}

		inline void initInner(const Bound3f& bound) {
			bound_ = bound;
			offset_ = 0;
			n_ = 0;
			children.reset(new OctreeNode[8]);
		}

		int offset() const {
			return offset_;
		}
		int N() const {
			return n_;
		}
	private:
		//#PERF2 内部节点不应该有n和offset，叶子节点不应该有children（使用union）
		int offset_;
		int n_;
		std::unique_ptr<OctreeNode[]> children;
		Bound3f bound_;
	};


	template <class T, typename BoundFunction /*= DefaultBoundFunction<T>*/, typename RayIntersectFunction /*= DefaultRayIntersectFunction<T>*/>
	bool Octree<T, BoundFunction, RayIntersectFunction>::intersect(const Ray& ray, T& hitobj /*= nullptr*/)
	{
		struct RayBoxHit
		{
			RayBoxHit(Float t0, Float t1, node_t* node)
				:t0(t0), t1(t1), node(node) {}
			Float t0, t1;
			node_t* node;

			bool operator<(const RayBoxHit& rhs) {
				return this->t0 > rhs.t0; //t0小的排后面
			}
		};

		//记得先检查是否为leaf
		auto calboxhit = [this](node_t* root,const Ray& ray) {
			std::vector<RayBoxHit> vec;
			vec.reserve(4);
			for (int i = 0; i < 8; i++) {
				Float t0, t1;
				if (ray.intersect(root->child(i)->bound(), t0, t1)) {
					vec.emplace_back(t0, t1, root->child(i));
				}
			}
			std::sort(vec.begin(), vec.end());
			return vec;
		};

		std::vector<std::vector<RayBoxHit>> stack;
		stack.reserve(maxLevel_+1);
		stack.emplace_back(calboxhit(root.get(),ray));
		bool find = false;
		Ray tempray = ray;
		while (!stack.empty()) {
			if (stack.back().empty()) {
				stack.pop_back();
				continue;
			}
			node_t* curnode = stack.back().back().node;
			if (curnode->isLeaf()) {
				auto q = query(*curnode);
				bool found = false;
				//由于精度问题，此处求交时扩展了一部分
				tempray.t0 = std::max(stack.back().back().t0 * Float(1 - RayTraceZero), ray.t0);
				tempray.t1 = std::min(stack.back().back().t1 * Float(1 + RayTraceZero), ray.t1);
				//tempray.t0 = ray.t0;
				//tempray.t1 = ray.t1;
				for (auto it = q.begin(); it != q.end(); it++) {
					const T& obj = getObj(*it);
					Float t;
					if (RayIntersectFunction()(obj, tempray, t)) {
						tempray.t1 = t;
						hitobj = obj;
						find = true;
					}
				}
				if (find)
					return true;
				stack.back().pop_back();
			}
			if (curnode->isInner()) {
				tempray.t0 = stack.back().back().t0;
				tempray.t1 = stack.back().back().t1;
				stack.back().pop_back();
				stack.push_back(calboxhit(curnode,tempray));
			}
		}

		return false;
	}
}

