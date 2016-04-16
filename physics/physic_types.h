#pragma once
#include <Vector.h>

namespace ds {

	struct AABBox {

		v2 position;
		v2 extent;
		v2 min;
		v2 max;

		AABBox() {}

		AABBox(const v2& pos, const v2& ext) {
			position = pos;
			extent = ext * 0.5f;
			min = vec_min(position - extent, position + extent);
			max = vec_max(position - extent, position + extent);
		}

		void scale(const v2& s) {
			extent.x *= s.x;
			extent.y *= s.y;
			min = vec_min(position - extent, position + extent);
			max = vec_max(position - extent, position + extent);
		}

		void transpose(const v2& pos) {
			position = pos;
			min = vec_min(position - extent, position + extent);
			max = vec_max(position - extent, position + extent);
		}

		v2 findClosestPoint(const v2& p) const {
			v2 ret(0, 0);
			ret.x = (p.x < min.x) ? min.x : (p.x > max.x) ? max.x : p.x;
			ret.y = (p.y < min.y) ? min.y : (p.y > max.y) ? max.y : p.y;
			return ret;
		}

		bool contains(const v2& point) const {
			if (point.x < min.x || max.x < point.x) {
				return false;
			}
			else if (point.y < min.y || max.y < point.y) {
				return false;
			}
			return true;
		}

		const bool overlaps(const AABBox& b) const {
			const v2 T = b.position - position;
			return fabs(T.x) <= (extent.x + b.extent.x) && fabs(T.y) <= (extent.y + b.extent.y);
		}
		/*
		const bool overlaps(const Sphere& sphere) const {
			float s, d = 0;
			//find the square of the distance
			//from the sphere to the box
			for (int i = 0; i < 2; ++i) {
				if (sphere.position[i] < min_value(i)) {
					s = sphere.position[i] - min_value(i);
					d += s*s;
				}
				else if (sphere.position[i] > max_value(i)) {
					s = sphere.position[i] - max_value(i);
					d += s*s;
				}

			}
			return d <= sphere.radius * sphere.radius;
		}
		*/
		float min_value(int index) const {
			return min[index];
		}

		float max_value(int index) const {
			return max[index];
		}
	};

}