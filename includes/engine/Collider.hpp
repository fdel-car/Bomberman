#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#include "engine/Engine.hpp"

struct Collider final {
   public:
	enum Shape { Rectangle = 0, Circle };
	Collider(void);
	Collider(Shape shape, int layerTag, float width, float height,
			 bool isTrigger = false);
	Collider(Collider const &src);
	~Collider(void);

	Shape shape;
	int layerTag;
	float width;
	float height;
	bool isTrigger;

   private:
	Collider &operator=(Collider const &rhs);
};

#endif