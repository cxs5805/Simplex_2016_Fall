#include "MyRigidBody.h"
using namespace Simplex;
//Allocation
void MyRigidBody::Init(void)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_bVisibleBS = false;
	m_bVisibleOBB = true;
	m_bVisibleARBB = false;

	m_fRadius = 0.0f;

	m_v3ColorColliding = C_RED;
	m_v3ColorNotColliding = C_WHITE;

	m_v3Center = ZERO_V3;
	m_v3MinL = ZERO_V3;
	m_v3MaxL = ZERO_V3;

	m_v3MinG = ZERO_V3;
	m_v3MaxG = ZERO_V3;

	m_v3HalfWidth = ZERO_V3;
	m_v3ARBBSize = ZERO_V3;

	m_m4ToWorld = IDENTITY_M4;
}
void MyRigidBody::Swap(MyRigidBody& a_pOther)
{
	std::swap(m_pMeshMngr, a_pOther.m_pMeshMngr);
	std::swap(m_bVisibleBS, a_pOther.m_bVisibleBS);
	std::swap(m_bVisibleOBB, a_pOther.m_bVisibleOBB);
	std::swap(m_bVisibleARBB, a_pOther.m_bVisibleARBB);

	std::swap(m_fRadius, a_pOther.m_fRadius);

	std::swap(m_v3ColorColliding, a_pOther.m_v3ColorColliding);
	std::swap(m_v3ColorNotColliding, a_pOther.m_v3ColorNotColliding);

	std::swap(m_v3Center, a_pOther.m_v3Center);
	std::swap(m_v3MinL, a_pOther.m_v3MinL);
	std::swap(m_v3MaxL, a_pOther.m_v3MaxL);

	std::swap(m_v3MinG, a_pOther.m_v3MinG);
	std::swap(m_v3MaxG, a_pOther.m_v3MaxG);

	std::swap(m_v3HalfWidth, a_pOther.m_v3HalfWidth);
	std::swap(m_v3ARBBSize, a_pOther.m_v3ARBBSize);

	std::swap(m_m4ToWorld, a_pOther.m_m4ToWorld);

	std::swap(m_CollidingRBSet, a_pOther.m_CollidingRBSet);
}
void MyRigidBody::Release(void)
{
	m_pMeshMngr = nullptr;
	ClearCollidingList();
}
//Accessors
bool MyRigidBody::GetVisibleBS(void) { return m_bVisibleBS; }
void MyRigidBody::SetVisibleBS(bool a_bVisible) { m_bVisibleBS = a_bVisible; }
bool MyRigidBody::GetVisibleOBB(void) { return m_bVisibleOBB; }
void MyRigidBody::SetVisibleOBB(bool a_bVisible) { m_bVisibleOBB = a_bVisible; }
bool MyRigidBody::GetVisibleARBB(void) { return m_bVisibleARBB; }
void MyRigidBody::SetVisibleARBB(bool a_bVisible) { m_bVisibleARBB = a_bVisible; }
float MyRigidBody::GetRadius(void) { return m_fRadius; }
vector3 MyRigidBody::GetColorColliding(void) { return m_v3ColorColliding; }
vector3 MyRigidBody::GetColorNotColliding(void) { return m_v3ColorNotColliding; }
void MyRigidBody::SetColorColliding(vector3 a_v3Color) { m_v3ColorColliding = a_v3Color; }
void MyRigidBody::SetColorNotColliding(vector3 a_v3Color) { m_v3ColorNotColliding = a_v3Color; }
vector3 MyRigidBody::GetCenterLocal(void) { return m_v3Center; }
vector3 MyRigidBody::GetMinLocal(void) { return m_v3MinL; }
vector3 MyRigidBody::GetMaxLocal(void) { return m_v3MaxL; }
vector3 MyRigidBody::GetCenterGlobal(void){	return vector3(m_m4ToWorld * vector4(m_v3Center, 1.0f)); }
vector3 MyRigidBody::GetMinGlobal(void) { return m_v3MinG; }
vector3 MyRigidBody::GetMaxGlobal(void) { return m_v3MaxG; }
vector3 MyRigidBody::GetHalfWidth(void) { return m_v3HalfWidth; }
matrix4 MyRigidBody::GetModelMatrix(void) { return m_m4ToWorld; }
void MyRigidBody::SetModelMatrix(matrix4 a_m4ModelMatrix)
{
	//to save some calculations if the model matrix is the same there is nothing to do here
	if (a_m4ModelMatrix == m_m4ToWorld)
		return;

	//Assign the model matrix
	m_m4ToWorld = a_m4ModelMatrix;

	//Calculate the 8 corners of the cube
	vector3 v3Corner[8];
	//Back square
	v3Corner[0] = m_v3MinL;
	v3Corner[1] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z);
	v3Corner[2] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z);
	v3Corner[3] = vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z);

	//Front square
	v3Corner[4] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[5] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[6] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MaxL.z);
	v3Corner[7] = m_v3MaxL;

	//Place them in world space
	for (uint uIndex = 0; uIndex < 8; ++uIndex)
	{
		v3Corner[uIndex] = vector3(m_m4ToWorld * vector4(v3Corner[uIndex], 1.0f));
	}

	//Identify the max and min as the first corner
	m_v3MaxG = m_v3MinG = v3Corner[0];

	//get the new max and min for the global box
	for (uint i = 1; i < 8; ++i)
	{
		if (m_v3MaxG.x < v3Corner[i].x) m_v3MaxG.x = v3Corner[i].x;
		else if (m_v3MinG.x > v3Corner[i].x) m_v3MinG.x = v3Corner[i].x;

		if (m_v3MaxG.y < v3Corner[i].y) m_v3MaxG.y = v3Corner[i].y;
		else if (m_v3MinG.y > v3Corner[i].y) m_v3MinG.y = v3Corner[i].y;

		if (m_v3MaxG.z < v3Corner[i].z) m_v3MaxG.z = v3Corner[i].z;
		else if (m_v3MinG.z > v3Corner[i].z) m_v3MinG.z = v3Corner[i].z;
	}

	//we calculate the distance between min and max vectors
	m_v3ARBBSize = m_v3MaxG - m_v3MinG;
}
//The big 3
MyRigidBody::MyRigidBody(std::vector<vector3> a_pointList)
{
	Init();
	//Count the points of the incoming list
	uint uVertexCount = a_pointList.size();

	//If there are none just return, we have no information to create the BS from
	if (uVertexCount == 0)
		return;

	//Max and min as the first vector of the list
	m_v3MaxL = m_v3MinL = a_pointList[0];

	//Get the max and min out of the list
	for (uint i = 1; i < uVertexCount; ++i)
	{
		if (m_v3MaxL.x < a_pointList[i].x) m_v3MaxL.x = a_pointList[i].x;
		else if (m_v3MinL.x > a_pointList[i].x) m_v3MinL.x = a_pointList[i].x;

		if (m_v3MaxL.y < a_pointList[i].y) m_v3MaxL.y = a_pointList[i].y;
		else if (m_v3MinL.y > a_pointList[i].y) m_v3MinL.y = a_pointList[i].y;

		if (m_v3MaxL.z < a_pointList[i].z) m_v3MaxL.z = a_pointList[i].z;
		else if (m_v3MinL.z > a_pointList[i].z) m_v3MinL.z = a_pointList[i].z;
	}

	//with model matrix being the identity, local and global are the same
	m_v3MinG = m_v3MinL;
	m_v3MaxG = m_v3MaxL;

	//with the max and the min we calculate the center
	m_v3Center = (m_v3MaxL + m_v3MinL) / 2.0f;

	//we calculate the distance between min and max vectors
	m_v3HalfWidth = (m_v3MaxL - m_v3MinL) / 2.0f;

	//Get the distance between the center and either the min or the max
	m_fRadius = glm::distance(m_v3Center, m_v3MinL);
}
MyRigidBody::MyRigidBody(MyRigidBody const& a_pOther)
{
	m_pMeshMngr = a_pOther.m_pMeshMngr;

	m_bVisibleBS = a_pOther.m_bVisibleBS;
	m_bVisibleOBB = a_pOther.m_bVisibleOBB;
	m_bVisibleARBB = a_pOther.m_bVisibleARBB;

	m_fRadius = a_pOther.m_fRadius;

	m_v3ColorColliding = a_pOther.m_v3ColorColliding;
	m_v3ColorNotColliding = a_pOther.m_v3ColorNotColliding;

	m_v3Center = a_pOther.m_v3Center;
	m_v3MinL = a_pOther.m_v3MinL;
	m_v3MaxL = a_pOther.m_v3MaxL;

	m_v3MinG = a_pOther.m_v3MinG;
	m_v3MaxG = a_pOther.m_v3MaxG;

	m_v3HalfWidth = a_pOther.m_v3HalfWidth;
	m_v3ARBBSize = a_pOther.m_v3ARBBSize;

	m_m4ToWorld = a_pOther.m_m4ToWorld;

	m_CollidingRBSet = a_pOther.m_CollidingRBSet;
}
MyRigidBody& MyRigidBody::operator=(MyRigidBody const& a_pOther)
{
	if (this != &a_pOther)
	{
		Release();
		Init();
		MyRigidBody temp(a_pOther);
		Swap(temp);
	}
	return *this;
}
MyRigidBody::~MyRigidBody() { Release(); };
//--- a_pOther Methods
void MyRigidBody::AddCollisionWith(MyRigidBody* a_pOther)
{
	/*
		check if the object is already in the colliding set, if
		the object is already there return with no changes
	*/
	auto element = m_CollidingRBSet.find(a_pOther);
	if (element != m_CollidingRBSet.end())
		return;
	// we couldn't find the object so add it
	m_CollidingRBSet.insert(a_pOther);
}
void MyRigidBody::RemoveCollisionWith(MyRigidBody* a_pOther)
{
	m_CollidingRBSet.erase(a_pOther);
}
void MyRigidBody::ClearCollidingList(void)
{
	m_CollidingRBSet.clear();
}
bool MyRigidBody::IsColliding(MyRigidBody* const a_pOther)
{
	//check if spheres are colliding as pre-test
	bool bColliding = (glm::distance(GetCenterGlobal(), a_pOther->GetCenterGlobal()) < m_fRadius + a_pOther->m_fRadius);
	
	//if they are colliding check the SAT
	if (bColliding)
	{
		if(SAT(a_pOther) != eSATResults::SAT_NONE)
			bColliding = false;// reset to false
	}

	if (bColliding) //they are colliding
	{
		this->AddCollisionWith(a_pOther);
		a_pOther->AddCollisionWith(this);
	}
	else //they are not colliding
	{
		this->RemoveCollisionWith(a_pOther);
		a_pOther->RemoveCollisionWith(this);
	}

	return bColliding;
}
void MyRigidBody::AddToRenderList(void)
{
	if (m_bVisibleBS)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
		else
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
	}
	if (m_bVisibleOBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorColliding);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorNotColliding);
	}
	if (m_bVisibleARBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
	}
}

uint MyRigidBody::SAT(MyRigidBody* const a_pOther)
{
	/*
	Your code goes here instead of this comment;

	For this method, if there is an axis that separates the two objects
	then the return will be different than 0; 1 for any separating axis
	is ok if you are not going for the extra credit, if you could not
	find a separating axis you need to return 0, there is an enum in
	Simplex that might help you [eSATResults] feel free to use it.
	(eSATResults::SAT_NONE has a value of 0)
	//*/

	// ideas borrowed from E07
	// translate all 16 points (8 for my box, 8 for other box) from local to global
	vector3 myCornersL[8], otherCornersL[8], myCornersG[8], otherCornersG[8];

	// back face
	// 2 3
	// 0 1
	myCornersL[0] = m_v3MinL;
	myCornersL[1] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z);
	myCornersL[2] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z);
	myCornersL[3] = vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z);
	
	// front face
	// 6 7
	// 4 5
	myCornersL[4] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	myCornersL[5] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z);
	myCornersL[6] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MaxL.z);
	myCornersL[7] = m_v3MaxL;

	// back face
	// 2 3
	// 0 1
	otherCornersL[0] = a_pOther->m_v3MinL;
	otherCornersL[1] = vector3(a_pOther->m_v3MaxL.x, a_pOther->m_v3MinL.y, a_pOther->m_v3MinL.z);
	otherCornersL[2] = vector3(a_pOther->m_v3MinL.x, a_pOther->m_v3MaxL.y, a_pOther->m_v3MinL.z);
	otherCornersL[3] = vector3(a_pOther->m_v3MaxL.x, a_pOther->m_v3MaxL.y, a_pOther->m_v3MinL.z);

	// front face
	// 6 7
	// 4 5
	otherCornersL[4] = vector3(a_pOther->m_v3MinL.x, a_pOther->m_v3MinL.y, a_pOther->m_v3MaxL.z);
	otherCornersL[5] = vector3(a_pOther->m_v3MaxL.x, a_pOther->m_v3MinL.y, a_pOther->m_v3MaxL.z);
	otherCornersL[6] = vector3(a_pOther->m_v3MinL.x, a_pOther->m_v3MaxL.y, a_pOther->m_v3MaxL.z);
	otherCornersL[7] = a_pOther->m_v3MaxL;
	
	// is this step even necessary?
	// actually yes, very yes
	// how else can I properly project onto axes for each test?
	for (uint i = 0; i < 8; i++)
		myCornersG[i] = vector3(m_m4ToWorld * vector4(myCornersL[i], 1.0f));
	for (uint i = 0; i < 8; i++)
		otherCornersG[i] = vector3(m_m4ToWorld * vector4(otherCornersL[i], 1.0f));

	// alright, how do we do the easy axes?
	// excerpt from orange book:
	// "Two OBBs are separated if for some axis L
	// the sum of their projected radii is
	// less than the distance between their projected centers."

	// T = Other's center - my center
	vector3 v3_distance = a_pOther->GetCenterGlobal() - GetCenterGlobal();
	// L = axis
	vector3 axis;

	// glm::abs(glm::dot(T, L)) = distance b/w centers along axis
	// b/c projected

	// how do we figure out radius for a given axis????
	// project onto axis in global space
	float v3_rA, v3_rB = 0.0f;

	// my x
	// maybe, we 
	vector3 v3_myX = vector3(1.0f, 0.0f, 0.0f);

	// cross local coords to get axis???
	vector3 temp1 = myCornersG[2] - myCornersG[0];//myCornersG[4] - myCornersG[0]; // z // these two crossed w/each other form y-axis
	vector3 temp2 = myCornersG[4] - myCornersG[0];//myCornersG[1] - myCornersG[0]; // x // these two crossed w/each other form y-axis

	v3_myX = glm::normalize(glm::cross(temp1, temp2));
	//std::cout << "(" << v3_myX.x << ", " << v3_myX.y << ", " << v3_myX.z << ")\n";

	if (!CollidingOnAxis(v3_myX, myCornersG, otherCornersG))
	{
		std::cout << "failed my x\n";
		return 1;
	}

	// my y
	temp1 = myCornersG[4] - myCornersG[0];
	temp2 = myCornersG[1] - myCornersG[0];
	vector3 v3_myY = glm::normalize(glm::cross(temp1, temp2));
	std::cout << "(" << v3_myY.x << ", " << v3_myY.y << ", " << v3_myY.z << ")\n";
	if (!CollidingOnAxis(v3_myY, myCornersG, otherCornersG))
	{
		std::cout << "failed my y\n";
		return 1;
	}

	// my z
	temp1 = myCornersG[1] - myCornersG[0];
	temp2 = myCornersG[2] - myCornersG[0];
	vector3 v3_myZ = glm::normalize(glm::cross(temp1, temp2));
	std::cout << "(" << v3_myZ.x << ", " << v3_myZ.y << ", " << v3_myZ.z << ")\n";
	if (!CollidingOnAxis(v3_myZ, myCornersG, otherCornersG))
	{
		std::cout << "failed my z\n";
		return 1;
	}

	// other x
	temp1 = otherCornersG[2] - otherCornersG[0];
	temp2 = otherCornersG[4] - otherCornersG[0];
	vector3 v3_otherX = glm::normalize(glm::cross(temp1, temp2));
	std::cout << "(" << v3_otherX.x << ", " << v3_otherX.y << ", " << v3_otherX.z << ")\n";
	if (!CollidingOnAxis(v3_otherX, myCornersG, otherCornersG))
	{
		std::cout << "failed other y\n";
		return 1;
	}
	
	// other y
	temp1 = otherCornersG[4] - otherCornersG[0];
	temp2 = otherCornersG[1] - otherCornersG[0];
	vector3 v3_otherY = glm::normalize(glm::cross(temp1, temp2));
	std::cout << "(" << v3_otherY.x << ", " << v3_otherY.y << ", " << v3_otherY.z << ")\n";
	if (!CollidingOnAxis(v3_otherY, myCornersG, otherCornersG))
	{
		std::cout << "failed other y\n";
		return 1;
	}
	
	// other z
	temp1 = otherCornersG[1] - otherCornersG[0];
	temp2 = otherCornersG[2] - otherCornersG[0];
	vector3 v3_otherZ = glm::normalize(glm::cross(temp1, temp2));
	std::cout << "(" << v3_otherZ.x << ", " << v3_otherZ.y << ", " << v3_otherZ.z << ")\n";
	if (!CollidingOnAxis(v3_otherZ, myCornersG, otherCornersG))
	{
		std::cout << "failed other z\n";
		return 1;
	}

	// now onto the 9 cross products
	
	// my x cross other x
	// my x cross other y
	// my x cross other z

	// my y cross other x
	// my y cross other y
	// my y cross other z

	// my z cross other x
	// my z cross other y
	// my z cross other z

	std::cout << "";


	/*
	// start projecting global points onto axis for both objects
	vector3 myCornersP[8], otherCornersP[8]; // P for projected along axis

	// now, how do I figure out which point is max and min?
	// my suspicion = store dot product, b/c that reps magnitude, and it can be negative
	// whichever is lowest = min
	// whichever is highest = max

	float myCornersM[8], otherCornersM[8]; // M for magnitude of dot product
	float myMaxAlongAxis = 0.0f, myMinAlongAxis = 0.0f, otherMaxAlongAxis = 0.0f, otherMinAlongAxis = 0.0f;
	for (uint i = 0; i < 8; i++)
	{
		// point = axis (unit vector) multiplied by magnitude (dot product)
		myCornersM[i] = glm::dot(myCornersG[i], v3_myX);
		myCornersP[i] = v3_myX * glm::dot(myCornersG[i], v3_myX);
		//std::cout << "myCornersP["<< i << "] = (" << myCornersP[i].x << ", " << myCornersP[i].y << ", " << myCornersP[i].z << ")\n";
		std::cout << "myCornersM[" << i << "] = " << myCornersM[i] << "\n";

		// if point is greater than max, set max to that
		if (myCornersM[i] > myMaxAlongAxis)
			myMaxAlongAxis = myCornersM[i];
		// if point is less than min, set min to that
		if (myCornersM[i] < myMinAlongAxis)
			myMinAlongAxis = myCornersM[i];

		otherCornersM[i] = glm::dot(otherCornersG[i], v3_myX);
		otherCornersP[i] = v3_myX * glm::dot(otherCornersG[i], v3_myX);
		//std::cout << "otherCornersP[" << i << "] = (" << otherCornersP[i].x << ", " << otherCornersP[i].y << ", " << otherCornersP[i].z << ")\n";
		std::cout << "otherCornersM[" << i << "] = " << otherCornersM[i] << "\n";

		// if point is greater than max, set max to that
		if (otherCornersM[i] > otherMaxAlongAxis)
			otherMaxAlongAxis = otherCornersM[i];
		// if point is less than min, set min to that
		if (otherCornersM[i] < otherMinAlongAxis)
			otherMinAlongAxis = otherCornersM[i];
	}

	// find my max and min
	myMaxAlongAxis = myCornersM[0];
	myMinAlongAxis = myCornersM[0];
	otherMaxAlongAxis = otherCornersM[0];
	otherMinAlongAxis = otherCornersM[0];
	for (uint i = 0; i < 8; i++)
	{
		// if point is greater than max, set max to that
		if (myCornersM[i] > myMaxAlongAxis)
			myMaxAlongAxis = myCornersM[i];
		// if point is less than min, set min to that
		if (myCornersM[i] < myMinAlongAxis)
			myMinAlongAxis = myCornersM[i];
		
		// if point is greater than max, set max to that
		if (otherCornersM[i] > otherMaxAlongAxis)
			otherMaxAlongAxis = otherCornersM[i];
		// if point is less than min, set min to that
		if (otherCornersM[i] < otherMinAlongAxis)
			otherMinAlongAxis = otherCornersM[i];
	}

	std::cout << "my max = " << myMaxAlongAxis << "\n";
	std::cout << "my min = " << myMinAlongAxis << "\n";

	// and other
	std::cout << "other max = " << otherMaxAlongAxis << "\n";
	std::cout << "other min = " << otherMinAlongAxis << "\n\n";

	// now test collision along this one axis
	if (myMaxAlongAxis < otherMinAlongAxis || otherMaxAlongAxis < myMinAlongAxis)
	{
		std::cout << "failed my x\n";
		return 1;
	}
	//*/

	// BAD
	// local to global
	//v3_myX = glm::normalize(vector3(m_m4ToWorld * vector4(m_v3Center, 1.0f)));
	
	// DEBUG
	//std::cout << "(" << v3_myX.x << ", " << v3_myX.y << ", " << v3_myX.z << ")\n";
	//std::cout << "(" << m_v3MinL.x << ", " << m_v3MinL.y << ", " << m_v3MinL.z << ")\n";
	//std::cout << "(" << myCornersL[0].x << ", " << myCornersL[0].y << ", " << myCornersL[0].z << ")\n";
	//std::cout << "(" << m_v3Center.x << ", " << m_v3Center.y << ", " << m_v3Center.z << ")\n";
	/*
	v3_rA = GetHalfWidth().x;
	v3_rA = glm::dot(GetHalfWidth(), v3_myX);
	v3_rB = a_pOther->GetHalfWidth().x;
	v3_rB = glm::dot(a_pOther->GetHalfWidth(), v3_myX);
	if (glm::abs(glm::dot(v3_distance, v3_myX)) > v3_rA + v3_rB)
	{
		std::cout << "failed my x\n";
		return 1;
	}
	//*/
	/*
	// my y
	vector3 v3_myY = vector3(0.0f, 1.0f, 0.0f);
	v3_rA = GetHalfWidth().y;
	v3_rB = a_pOther->GetHalfWidth().y;
	if (glm::abs(glm::dot(v3_distance, v3_myY)) > v3_rA + v3_rB)
	{
		std::cout << "failed my y\n";
		return 1;
	}

	// my z
	vector3 v3_myZ = vector3(0.0f, 0.0f, 1.0f);
	v3_rA = GetHalfWidth().z;
	v3_rB = a_pOther->GetHalfWidth().z;
	if (glm::abs(glm::dot(v3_distance, v3_myZ)) > v3_rA + v3_rB)
	{
		std::cout << "failed my z\n";
		return 1;
	}


	// other's x (my x, but the other way)
	// other's y (my y, but the other way)
	// other's z (my z, but the other way)

	// wacky cross products

	//there is no axis test that separates this two objects
	//return 1;
	//*/
	return eSATResults::SAT_NONE;
}

bool MyRigidBody::CollidingOnAxis(vector3 axis, vector3 myCornersG[], vector3 otherCornersG[])
{
	// start projecting global points onto axis for both objects
	vector3 myCornersP[8], otherCornersP[8]; // P for projected along axis

	// now, how do I figure out which point is max and min?
	// my suspicion = store dot product, b/c that reps magnitude, and it can be negative
	// whichever is lowest = min
	// whichever is highest = max

	float myCornersM[8], otherCornersM[8]; // M for magnitude of dot product
	float myMaxAlongAxis = 0.0f, myMinAlongAxis = 0.0f, otherMaxAlongAxis = 0.0f, otherMinAlongAxis = 0.0f;
	for (uint i = 0; i < 8; i++)
	{
		// point = axis (unit vector) multiplied by magnitude (dot product)
		myCornersM[i] = glm::dot(myCornersG[i], axis);
		myCornersP[i] = axis * glm::dot(myCornersG[i], axis);
		//std::cout << "myCornersP["<< i << "] = (" << myCornersP[i].x << ", " << myCornersP[i].y << ", " << myCornersP[i].z << ")\n";
		std::cout << "myCornersM[" << i << "] = " << myCornersM[i] << "\n";

		// if point is greater than max, set max to that
		if (myCornersM[i] > myMaxAlongAxis)
			myMaxAlongAxis = myCornersM[i];
		// if point is less than min, set min to that
		if (myCornersM[i] < myMinAlongAxis)
			myMinAlongAxis = myCornersM[i];

		otherCornersM[i] = glm::dot(otherCornersG[i], axis);
		otherCornersP[i] = axis * glm::dot(otherCornersG[i], axis);
		//std::cout << "otherCornersP[" << i << "] = (" << otherCornersP[i].x << ", " << otherCornersP[i].y << ", " << otherCornersP[i].z << ")\n";
		std::cout << "otherCornersM[" << i << "] = " << otherCornersM[i] << "\n";

		// if point is greater than max, set max to that
		if (otherCornersM[i] > otherMaxAlongAxis)
			otherMaxAlongAxis = otherCornersM[i];
		// if point is less than min, set min to that
		if (otherCornersM[i] < otherMinAlongAxis)
			otherMinAlongAxis = otherCornersM[i];
	}

	// find my max and min
	myMaxAlongAxis = myCornersM[0];
	myMinAlongAxis = myCornersM[0];
	otherMaxAlongAxis = otherCornersM[0];
	otherMinAlongAxis = otherCornersM[0];
	for (uint i = 0; i < 8; i++)
	{
		// if point is greater than max, set max to that
		if (myCornersM[i] > myMaxAlongAxis)
			myMaxAlongAxis = myCornersM[i];
		// if point is less than min, set min to that
		if (myCornersM[i] < myMinAlongAxis)
			myMinAlongAxis = myCornersM[i];

		// if point is greater than max, set max to that
		if (otherCornersM[i] > otherMaxAlongAxis)
			otherMaxAlongAxis = otherCornersM[i];
		// if point is less than min, set min to that
		if (otherCornersM[i] < otherMinAlongAxis)
			otherMinAlongAxis = otherCornersM[i];
	}

	std::cout << "my max = " << myMaxAlongAxis << "\n";
	std::cout << "my min = " << myMinAlongAxis << "\n";

	// and other
	std::cout << "other max = " << otherMaxAlongAxis << "\n";
	std::cout << "other min = " << otherMinAlongAxis << "\n";

	// now test collision along this one axis
	if (myMaxAlongAxis < otherMinAlongAxis || otherMaxAlongAxis < myMinAlongAxis)
	{
		std::cout << "failed on axis (" << axis.x << ", " << axis.y << ", " << axis.z << ")\n\n";
		return false;
	}
	return true;
}
