/*----------------------------------------------
Programmer: Alberto Bobadilla (labigm@gmail.com)
Date: 2017/05
----------------------------------------------*/
#ifndef __MYRIGIDBODY_H_
#define __MYRIGIDBODY_H_

#include "MyMesh.h"
#include "MyCamera.h"
#include "Definitions.h"
namespace Simplex
{

	class MyRigidBody
{
	// everything is public for now (don't actually do this in real assignments)
public:
	// what does rigidbody need?
	// constructor, rule of 3
	// Alberto will do that in the video
	
	// need to render rigidbody in order to see it
	// BS for bounding sphere
	// must be at heap so rendering context will already have been created
	// if on stack, it'll break at compile time
	Mesh* m_pBS = nullptr;
	
	// bounding box
	Mesh* m_pBB= nullptr;
	
	float fRadius;

	vector3 v3Center;
	vector3 v3Min;
	vector3 v3Max;

	// constructor (shouldn't be void b/c data has to come from SOMEWHERE)
	// 
	MyRigidBody(MyMesh* a_pMesh);

	// 
	void Render(MyCamera* a_pCamera, matrix4 a_m4Transform);
};

} // namespace Simplex

#endif // __MYRIGIDBODY_H_
