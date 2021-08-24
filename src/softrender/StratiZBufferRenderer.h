#pragma once
#include "types.h"
#include "mcl.h"
#include "SoftRenderer.h"
#include "RasPolygon.h"
namespace mcl {
	class StratiZBuffer;

	class StratiZBufferRenderer :public SoftRenderer
	{
	public:
		StratiZBufferRenderer(Scene* scene, int px, int py);

		virtual int paint(PaintInfomation* info = nullptr);

	private:
		struct MyPolygon {
			RasPolygon data;	//光栅化用多边形数据
			Bound3f bound;  //屏幕坐标系下的包围盒
		};

	protected:
		struct TransformInfo {
			Transform invTotTran;
			Vector3f screenxInWorld;
			Vector3f screenzInWorld;
		};
		int rasterPolygon(RasPolygon& polygon, const Bound2i& bound, StratiZBuffer& zbuffer, const TransformInfo& traninfo);
	};

	class StratiZBuffer {
	public:
		//size为缓冲的长宽方向像素数，为简便起见，要为2的幂次，如1024
		StratiZBuffer(int size);
		struct Cell
		{
			Float maxdepth;
			Float mindepth;
			char mask; // 低四位决定maxdepth由哪几个child组成
			SoftRenderer::FragInfo info;

			bool needUpdateMax() {
				return !bool(mask);
			}
			
			void updateChild(int index) { //第index个儿子深度减小后，要从mask中将第index位置为0
				mask = mask & ((1 << 4) - (1 << index) - 1);
			}

			void resetMask() {
				mask = 0;
			}

			void maskChild(int index) {
				mask = mask | (1 << index);
			}
		};
		
		/**
		 * @brief 判断包围盒内区域是否可被剔除
		 */
		bool isCull(const Bound3f& bound);

		/**
		 * @brief 递归更新深度值
		 */
		bool updateDepth(const Point2i& childpos, Float depth);

		/**
		 * @获取全分辨率层单元
		 */
		inline Cell& cell(const int& x, const int& y);

	private:
		inline int size(int layer); //返回该层的每维单元数
		inline Point2i StratiZBuffer::sizeOfLayer(int layer);
		inline int layerCount() { return nlayer; };
		inline Cell& cell(int layer, const Point2i& pos);
		inline Cell& cell(int layer, const int& x, const int& y);
		Bound2i cellBound(int layer, const Float& x, const Float& y);
		Bound2i cellBound(int layer, const Point2i& pos);
		inline void child(const Point2i& pos, const int& index, Point2i& childpos);
		inline int indexOfChild(const Point2i& pos);
		void father(const Point2i& pos, Point2i& fatherpos);

		int totsize;
		int nlayer;
		std::vector<std::vector<Cell>> layers; //layers[0] 只有一个像素
	};

	inline void StratiZBuffer::child(const Point2i& pos, const int& index, Point2i& childpos)
	{
		childpos = pos * 2 + Vector2i(index & 1, (index & 2) >> 1);
	}

	inline mcl::StratiZBuffer::Cell& StratiZBuffer::cell(int layer, const Point2i& pos)
	{
		DCHECK(layer < layers.size());
		return layers[layer][size(layer) * pos.y() + pos.x()];
	}

	inline mcl::StratiZBuffer::Cell& StratiZBuffer::cell(int layer, const int& x, const int& y)
	{
		return layers[layer][size(layer) * y + x];
	}

	inline mcl::StratiZBuffer::Cell& StratiZBuffer::cell(const int& x, const int& y)
	{
		return layers[nlayer - 1][y * totsize + x];
	}

	inline int StratiZBuffer::indexOfChild(const Point2i& pos)
	{
		return pos.x() % 2 + (pos.y() % 2) * 2;
	}

	inline int StratiZBuffer::size(int layer)
	{
		return 1 << layer;
	}

	inline Point2i StratiZBuffer::sizeOfLayer(int layer)
	{
		return Point2i(1 << layer, 1 << layer);
	}
}

