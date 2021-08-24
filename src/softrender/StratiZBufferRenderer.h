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
			RasPolygon data;	//��դ���ö��������
			Bound3f bound;  //��Ļ����ϵ�µİ�Χ��
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
		//sizeΪ����ĳ�������������Ϊ��������ҪΪ2���ݴΣ���1024
		StratiZBuffer(int size);
		struct Cell
		{
			Float maxdepth;
			Float mindepth;
			char mask; // ����λ����maxdepth���ļ���child���
			SoftRenderer::FragInfo info;

			bool needUpdateMax() {
				return !bool(mask);
			}
			
			void updateChild(int index) { //��index��������ȼ�С��Ҫ��mask�н���indexλ��Ϊ0
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
		 * @brief �жϰ�Χ���������Ƿ�ɱ��޳�
		 */
		bool isCull(const Bound3f& bound);

		/**
		 * @brief �ݹ�������ֵ
		 */
		bool updateDepth(const Point2i& childpos, Float depth);

		/**
		 * @��ȡȫ�ֱ��ʲ㵥Ԫ
		 */
		inline Cell& cell(const int& x, const int& y);

	private:
		inline int size(int layer); //���ظò��ÿά��Ԫ��
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
		std::vector<std::vector<Cell>> layers; //layers[0] ֻ��һ������
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

