#include "MyRigidBody.h"

Simplex::MyRigidBody::MyRigidBody(MyMesh* a_pMesh)
{
	// mesh param now passed as arg

	m_pBS = new Mesh();
	//m_pBS->GenerateIcoSphere(5.0f, 2, C_WHITE);
	
	if (a_pMesh == nullptr)
		return;

	// not copying object, just copying list of vertices
	std::vector<vector3> pointsList = a_pMesh->GetVertexList();

	// 0 points? no object silly!!!!
	if (pointsList.size() < 1)
		return;

	// how to determine????
	fRadius = 0.0f;

	// answer = what is avg b/w smallest and largest value for each of 3 axes
	v3Center;

	// these 2 must be initialized to an actual point in mesh
	v3Min = pointsList[0];
	v3Max = pointsList[0];

	uint size = pointsList.size();

	// loop thru whole cloud of points to find x and y
	// start at 1 b/c 0 assumed to be both max and min
	for (uint i = 1; i < size; i++)
	{
		if (pointsList[i].x < v3Min.x)
			v3Min.x = pointsList[i].x;
		// "else" needed b/c a point cannot be both max and min at once
		else if (pointsList[i].x > v3Max.x)
			v3Max.x = pointsList[i].x;

		if (pointsList[i].y < v3Min.y)
			v3Min.y = pointsList[i].y;
		else if (pointsList[i].y > v3Max.y)
			v3Max.y = pointsList[i].y;

		if (pointsList[i].z < v3Min.z)
			v3Min.z = pointsList[i].z;
		else if (pointsList[i].z > v3Max.z)
			v3Max.z = pointsList[i].z;
	}

	// now get center
	v3Center = (v3Min + v3Max) / 2.0f;
	// don't wanna sqrt for distances
	// another solution: distance from center to max
	fRadius = glm::distance(v3Center, v3Max);

	//m_pBS->GenerateIcoSphere(fRadius, 2, C_WHITE);
	m_pBS->GenerateIcoSphere(1.0f, 2, C_WHITE);

	m_pBB = new Mesh();
	m_pBB->GenerateCube(1.0f, C_WHITE);
}

void Simplex::MyRigidBody::Render(MyCamera* a_pCamera, matrix4 a_m4Transform)
{
	// how do we render??
	// call render from BS
	// not working with singleton, so we don't have access to camera
	// pass camera as pointer so you only need to copy address

	// glm::value_ptr
	// remember what it does? no
	// shader takes continuous array of 16 floats,
	// NOT A glm::mat4 or matrix4
	// so value_ptr translates matrix4 to array of 16 floats
	// returns a const float*
	// where is it stored????
	
	// formerly float*
	float* pTemp = new float[16];
	
	// memcpy(destination, origin)
	// takes one section of memory and copies whole block to another address
	matrix4 m4Resize = glm::scale(a_m4Transform, vector3(fRadius));

	memcpy(pTemp, glm::value_ptr(m4Resize), 16 * sizeof(float));

	m_pBS->RenderWire(a_pCamera->GetProjectionMatrix(), a_pCamera->GetViewMatrix(), pTemp, 1);
	// last arg = identity m4 for model matrix

	// how do we get this size??????
	vector3 v3Size = v3Max - v3Min;

	m4Resize = glm::scale(a_m4Transform, v3Size);
	memcpy(pTemp, glm::value_ptr(m4Resize), 16 * sizeof(float));

	m_pBB->RenderWire(a_pCamera->GetProjectionMatrix(), a_pCamera->GetViewMatrix(), pTemp, 1);

	// deallocate
	SafeDelete(pTemp);
	if (pTemp != nullptr)
	{
		delete[] pTemp;
		pTemp = nullptr;
	}
}
