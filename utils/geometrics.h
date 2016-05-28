#pragma once
#include "..\renderer\MeshBuffer.h"

namespace ds {	

	class MeshGen {

		struct Edge {
			uint16_t index;
			uint16_t next;
			uint16_t prev;
			uint16_t start;
			uint16_t end;
			uint16_t faceIndex;
			v2 uv;
		};

		struct Face {
			uint16_t edge;
			v3 n;
			Color color;
		};

	public:
		MeshGen();
		~MeshGen();
		void build(Mesh* mesh);
		uint16_t add_cube(const v3& position, const v3& size, uint16_t* faces = 0);
		uint16_t add_cube(const v3& position, const v3& size, const v3& rotation);
		void set_color(uint16_t faceIndex, const Color& color);
		uint16_t add_face(v3* positions);
		uint16_t combine(uint16_t first, uint16_t second);
		void add_face(const v3& position, const v2& size, const v3& normal);
		void move_edge(uint16_t edgeIndex, const v3& position);
		void move_face(uint16_t faceIndex, const v3& position);
		void texture_face(uint16_t faceIndex, const Texture& t);
		uint16_t split_edge(uint16_t edgeIndex, float factor = 0.5f);
		uint16_t get_edge_index(uint16_t faceIndex, int nr);
		uint16_t make_face(uint16_t* edges);
		void move_vertex(uint16_t edgeIndex, bool start, const v3& position);
		uint16_t extrude_edge(uint16_t edgeIndex, const v3& pos);
		void debug();
		void recalculate_normals();
		void parse(const char* fileName);
		void create_ring(float radius, float width, uint16_t segments);
	private:
		bool is_clock_wise(uint16_t index);
		void calculate_normal(Face* f);
		int add_vertex(const v3& pos);
		int find_edge(const v3& start, const v3& end);
		int find_vertex(const v3& pos);
		int find_opposite_edge(uint16_t edgeIndex);
		Array<v3> _vertices;
		Array<Edge> _edges;
		Array<Face> _faces;
	};

	namespace geometrics {

		void createCube(Mesh* mesh,const Rect& textureRect,const v3& center = v3(0,0,0), const v3& size = v3(1,1,1), const v3& rotation = v3(0,0,0));

		void createCube(Mesh* mesh, Rect* textureRects, const v3& center = v3(0, 0, 0), const v3& size = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0));

		void createGrid(Mesh* mesh, float cellSize, int countX, int countY, const Rect& textureRect,const v3& offset = v3(0,0,0), const Color& color = Color::WHITE);

		void createPlane(Mesh* mesh, const v3& position, const Rect& textureRect, const v2& size, float rotation = 0.0f, const Color& color = Color::WHITE);

		void createXYPlane(Mesh* mesh, const v3& position, const Rect& textureRect, const v2& size, float rotation = 0.0f, const Color& color = Color::WHITE);

	}
}

