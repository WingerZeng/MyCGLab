#pragma once
#include "mcl.h"
#include "types.h"

#define BP_STRUCTURE_FRIENDS(TypeName) \
	friend bpSolid;	  \
	friend bpFace;		  \
	friend bpLoop;		  \
	friend bpHalfEdge;	  \
	friend bpVertex;	  \
	friend bpEdge;		  \
	friend bpSolid* mvfs(const Point3f& coord, bpLoop** lp, bpVertex** vtx);\
	friend bpHalfEdge* mev(bpVertex* v1, bpLoop* lp, const Point3f& coord);\
	friend bpFace* mef(bpVertex* v1, bpVertex* v2, bpLoop* lp); \
	friend bpFace* mef(bpHalfEdge* pe1, bpHalfEdge* pe2, bpLoop*lp);\
	friend bpFace* kemr(bpLoop* lp,bpHalfEdge* he); \
	friend int kfmrh(bpFace* f1, bpFace* f2); \
	friend bpSolid* getSolid(bp##TypeName*);

//for the decalaration of operations method of element list
#define BP_ELEMENT_OP_DECL(TypeName,VarName) \
	void add##TypeName(bp##TypeName* elem);\
	int del##TypeName(bp##TypeName* elem);\
	bool find##TypeName(bp##TypeName* elem);\
	bp##TypeName* get##TypeName();


namespace mcl {
	class bpSolid;
	class bpFace;
	class bpLoop;
	class bpHalfEdge;
	class bpVertex;
	class bpEdge;
	class PPolygon;

	class bpSolid :public ListElement<bpSolid>
	{
		BP_STRUCTURE_FRIENDS(Solid)
	public:
		BP_ELEMENT_OP_DECL(Face, sfaces)
		BP_ELEMENT_OP_DECL(Edge, sedges)
		BP_ELEMENT_OP_DECL(Vertex, svertics)

	private:
		bpFace* sfaces = nullptr;
		bpEdge* sedges = nullptr;
		bpVertex* svertics = nullptr;
	};

	class bpFace :public ListElement<bpFace>
	{
		BP_STRUCTURE_FRIENDS(Face)
	public:
		mcl::bpLoop* Floops() const { return floops; }

		BP_ELEMENT_OP_DECL(Loop, floops)
	private:
		bpSolid* fsolid = nullptr;
		bpLoop* floops = nullptr;
	};

	class bpLoop :public ListElement<bpLoop>
	{
		BP_STRUCTURE_FRIENDS(Loop)
	public:
		inline bpHalfEdge* findVertexPevEdge(bpVertex* v);
		bpHalfEdge* getFirstHalfEdge() { return firstEg; }
		inline bool edgeExists(bpHalfEdge* he);
		mcl::bpFace* Lface() const { return lface; }
		void printEdgesDebug();
		void printEloopDebug();
		
		void getVertices(std::vector<bpVertex*>& vs);
	private:
		bpFace* lface = nullptr;
		bpHalfEdge* firstEg = nullptr;
	};

	class bpHalfEdge
	{
		BP_STRUCTURE_FRIENDS(HalfEdge)
	public:
		inline bpVertex* getBeginVtx() const { return beginVtx; }
		inline bpVertex* getEndVtx() const;

		mcl::bpEdge* Edge() const { return edge; }

		mcl::bpLoop* Eloop() const { return eloop; }

		mcl::bpHalfEdge* Nxt() const { return nxt; }

		mcl::bpHalfEdge* Pev() const { return pev; }

		mcl::bpHalfEdge* Adj() const { return adj; }

		mcl::bpVertex* BeginVtx() const { return beginVtx; }
	private:
		bpEdge* edge = nullptr;
		bpLoop* eloop = nullptr;
		bpHalfEdge* nxt = nullptr, *pev = nullptr;
		bpHalfEdge* adj = nullptr;
		bpVertex* beginVtx = nullptr;
	};

	class bpEdge :public ListElement<bpEdge>
	{
		BP_STRUCTURE_FRIENDS(Edge)
	public:
		inline int getHalfEdges(bpHalfEdge* he1, bpHalfEdge* he2);
	private:
		bpHalfEdge* firstHalf = nullptr;
	};

	class bpVertex :public ListElement<bpVertex>
	{
		BP_STRUCTURE_FRIENDS(Vertex)
	public:
		bpVertex(const Point3f& cd) :coord(cd) {}
		Point3f getCoord() { return coord; }
		mcl::bpHalfEdge* FirstEdge() const { return firstEdge; }
	private:
		Point3f coord;
		bpHalfEdge* firstEdge = nullptr;
	};

	/*Euler operations*/

	/**@brief Make new vertex, face and solid;
	 */
	bpSolid* mvfs(const Point3f& coord,bpLoop** lp=nullptr, bpVertex** vtx=nullptr);

	/**@brief Make new vertex, then connect exist vertex. The HalfEdge whose end point is the new vertex will be returned.
	 */
	bpHalfEdge* mev(bpVertex* v1, bpLoop* lp, const Point3f& coord);

	/**@brief Split loop make new edge and face.
	 *The new face include the HalfEdge from v1 to v2, that will be returned.
	 *lp will remain in the other one face.
	 */
	bpFace* mef(bpVertex* v1, bpVertex* v2, bpLoop* lp);

	/**@brief  Split loop make new edge and face
	 * The new face include the HalfEdge from end vertex of pe1 to end vertex of pe2, that will be returned.
	 * This function is used to determine the previous half edge of v1 and v2 in the original implementation, especial useful when vertex has several previous half edges;
	 */
	bpFace* mef(bpHalfEdge* pe1, bpHalfEdge* pe2, bpLoop*lp);

	/**@brief Delete f2. Define f2 as the inner loop of f1.
	*/
	int kfmrh(bpFace* f1, bpFace* f2);

	/**@brief Delete an edge from a loop, then create an inner loop.
	 *@param[in]	lp		The loop to operate. The begin vertex if he will remain in this loop after return.
	 *@param[in]	he		The one half of the edge to delete. Must point from outer loop to inner if possible.
	 */
	 bpFace* kemr(bpLoop* lp, bpHalfEdge* he);

	/*Advanced operations*/
	/**@brief Create new faces from sweep a exist face.
	 */
	 int sweep(bpSolid* solid, bpFace** face, Vector3f dir, Float dist);

	/**@brief Create a simple face from the indicated loops.
	 *@param[in]	lps		The points to indicate the loops.
	 */
	 bpSolid* createFaceFromLoop(const std::vector<std::vector<Point3f>>& lps,bpFace** pface=nullptr, bpFace** prface= nullptr);

	/*Some inline definitions follow*/
	inline int bpEdge::getHalfEdges(bpHalfEdge* he1, bpHalfEdge* he2)
	{
		he1 = this->firstHalf;
		he2 = this->firstHalf->adj;
	}

	inline bpHalfEdge* bpLoop::findVertexPevEdge(bpVertex* v)
	{
		bpHalfEdge* eg = firstEg;
		if (!eg) return nullptr;
		do {
			if (eg->getEndVtx() == v)
				return eg;
			eg = eg->nxt;
		} while (eg != firstEg);
		return nullptr;
	}

	inline bpVertex* bpHalfEdge::getEndVtx() const {
		return nxt->getBeginVtx();
	}

	inline bpSolid* getSolid(bpFace* fc) {
		return fc->fsolid;
	}

	inline bpSolid* getSolid(bpLoop* lp) {
		if (lp->lface) {
			return getSolid(lp->lface);
		}
		else return nullptr;
	}

	inline bpSolid* getSolid(bpHalfEdge* he) {
		if (he->Eloop()) {
			return getSolid(he->Eloop());
		}
		else return nullptr;
	}

	inline bpSolid* getSolid(bpEdge* eg) {
		if (eg->firstHalf) {
			getSolid(eg->firstHalf);
		}
		else return nullptr;
	}

	inline bool bpLoop::edgeExists(bpHalfEdge* he) {
		bpHalfEdge* nowhe = this->getFirstHalfEdge();
		if (!he || !nowhe) return false;
		bpHalfEdge* fhe = nowhe;
		do {
			if (he == nowhe) return true;
			nowhe = nowhe->nxt;
		} while (fhe != nowhe);
		return false;
	}
}

