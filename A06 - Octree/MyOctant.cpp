//#include "Simplex\Physics\Octant.h"
#include "MyOctant.h"
using namespace Simplex;

// this constructor is only called on the root node
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	// default values
	if (a_nMaxLevel <= 0 || a_nMaxLevel == -1)
		a_nMaxLevel = 2;
	if (a_nIdealEntityCount <= 0 || a_nIdealEntityCount == -1)
		a_nIdealEntityCount = 5;
	
	/*MyOctant::*/m_uOctantCount++;
	/*MyOctant::*/m_uMaxLevel = a_nMaxLevel;
	/*MyOctant::*/m_uIdealEntityCount = a_nIdealEntityCount;
	
	Init();
	ConstructTree();
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	m_v3Center = a_v3Center;

	float half = a_fSize / 2;
	m_v3Min = m_v3Center - half;
	m_v3Max = m_v3Center + half;
}

MyOctant::MyOctant(MyOctant const& other)
{
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize;

	m_pMeshMngr = other.m_pMeshMngr;
	m_pEntityMngr = other.m_pEntityMngr;

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pParent = other.m_pParent;
	//m_pChild = other.m_pChild;
	for (uint i = 0; i < 8; i++)
		m_pChild[i] = other.m_pChild[i];

	m_EntityList = other.m_EntityList;

	m_pRoot = other.m_pRoot;
	
	if (m_uID == 0)
		m_lChild = other.m_lChild;
}

MyOctant& MyOctant::operator=(MyOctant const& other) { return *this; }

MyOctant::~MyOctant(void){}
//*
void MyOctant::Swap(MyOctant& other)
{
	//std::swap();
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);

	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pEntityMngr, other.m_pEntityMngr);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	std::swap(m_pParent, other.m_pParent);

	std::swap(m_pChild, other.m_pChild);
	
	std::swap(m_EntityList, other.m_EntityList);

	std::swap(m_pRoot, other.m_pRoot);

	if (m_uID == 0)
		std::swap(m_lChild, other.m_lChild);
}
//*/
float MyOctant::GetSize(void) { return m_fSize; }

vector3 MyOctant::GetCenterGlobal(void) { return m_v3Center; }

vector3 MyOctant::GetMinGlobal(void) { return m_v3Min; }

vector3 MyOctant::GetMaxGlobal(void) { return m_v3Max; }

bool MyOctant::IsColliding(uint a_uRBIndex) { return false; }

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	// validate color
	if (a_v3Color != C_YELLOW)
		a_v3Color = C_YELLOW;
}

void MyOctant::Display(vector3 a_v3Color)
{
	// validate color
	if (a_v3Color != C_YELLOW)
		a_v3Color = C_YELLOW;
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	// validate color
	if (a_v3Color != C_YELLOW)
		a_v3Color = C_YELLOW;
}

void MyOctant::ClearEntityList(void)
{
	m_EntityList.clear();
}

void MyOctant::Subdivide(void)
{
	// get half-width of current octant
	vector3 half = glm::abs(m_v3Center - m_v3Min);

	// and halve it again.
	// this will be quarter-width for current octant
	// but it will be half-width for the subsequent octant
	vector3 quarter = half / 2.0f;

	// back
	// 2 - 3
	//   \  
	// 0 - 1
	m_pChild[0] = new MyOctant(m_v3Center - quarter, quarter.x);
	m_pChild[1] = new MyOctant(vector3(m_v3Center.x + quarter.x, m_v3Center.y - quarter.y, m_v3Center.z - quarter.z), quarter.x);
	m_pChild[2] = new MyOctant(vector3(m_v3Center.x - quarter.x, m_v3Center.y + quarter.y, m_v3Center.z - quarter.z), quarter.x);
	m_pChild[3] = new MyOctant(vector3(m_v3Center.x + quarter.x, m_v3Center.y + quarter.y, m_v3Center.z - quarter.z), quarter.x);

	// front
	// 6 - 7
	//   \  
	// 4 - 5
	m_pChild[4] = new MyOctant(vector3(m_v3Center.x - quarter.x, m_v3Center.y - quarter.y, m_v3Center.z + quarter.z), quarter.x);
	m_pChild[5] = new MyOctant(vector3(m_v3Center.x + quarter.x, m_v3Center.y - quarter.y, m_v3Center.z + quarter.z), quarter.x);
	m_pChild[6] = new MyOctant(vector3(m_v3Center.x - quarter.x, m_v3Center.y + quarter.y, m_v3Center.z + quarter.z), quarter.x);
	m_pChild[7] = new MyOctant(m_v3Center + quarter, quarter.x);

	// increment octant count
	/*MyOctant::*/m_uOctantCount += 8;

	// current node has 8 children
	m_uChildren = 8;

	// iterate through children
	// but first, get the current node's ID + 1
	uint IDTemp = m_uID + 1;
	//*
	for (uint i = 0; i < 8; i++)
	{
		// get singletons
		m_pChild[i]->m_pMeshMngr = MeshManager::GetInstance();
		m_pChild[i]->m_pEntityMngr = MyEntityManager::GetInstance();

		// set their parent to current node
		m_pChild[i]->m_pParent = this;

		// update their ID and increment our temp
		m_pChild[i]->m_uID = IDTemp;
		IDTemp++;

		// each child is one level deeper than the parent
		m_pChild[i]->m_uLevel = m_uLevel + 1;

		// right now, none of these children have any children of their own
		m_pChild[i]->m_uChildren = 0;
		//m_pChild[i]->

		// every child will have the same root octant
		// so traverse backwards until you reach a nullptr
		// the one before that is the root
		MyOctant* NodeTemp = this;
		while (NodeTemp->m_pParent != nullptr)
		{
			NodeTemp = NodeTemp->m_pParent;
		}
		m_pChild[i]->m_pRoot = NodeTemp;

		// get all objects (if any) in this octant
		// if objects present here, add this octant to root's m_lChild
	}
}


MyOctant* MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 8) a_nChild = 8;
	if (a_nChild <= 0 || a_nChild == -1) a_nChild = 0;
	return m_pChild[a_nChild];
}

MyOctant* MyOctant::GetParent(void) { return m_pParent; }

bool MyOctant::IsLeaf(void) { return (m_uChildren == 0); }

bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	return (m_EntityList.size() > a_nEntities);
}

void MyOctant::KillBranches(void)
{
	// we can only delete all of the branches at the root
	// so get to the root
	MyOctant* OctTemp = nullptr;
	if (this->m_pRoot == nullptr)
		OctTemp = this;
	else
		OctTemp = this->m_pRoot;
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	// validate default value
	if (a_nMaxLevel != 3)
		a_nMaxLevel = 3;

	// validate root
	if (this->m_pParent != nullptr)
		return;
	else
	{
		// in case not already fetched, get entity manager
		m_pEntityMngr = MyEntityManager::GetInstance();

		// DEBUG
		std::cout << "num entities = " << m_pEntityMngr->GetEntityCount() << "\n";
	}
}

void MyOctant::AssignIDtoEntity(void)
{
	// start here
	MyOctant* temp = this;
	while (temp->m_uChildren != 0)
	{
		// get children
		for (uint i = 0; i < temp->m_uChildren; i++)
		{
			// validate child node's existence
			if (temp->m_pChild[i] != nullptr)
			{
				temp->m_pChild[i]->m_uID = temp->m_uID;
			}
		}
	}
}

uint MyOctant::GetOctantCount(void) { return /*MyOctant::*/m_uOctantCount; }

void MyOctant::Release(void)
{

}

void MyOctant::Init(void)
{
	// create or fetch singletons
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	// since this is the root,
	// let's set member data to their default values just in case
	m_uID = 0;
	m_uLevel = 0;
	m_uChildren = 0;

	m_v3Center = vector3(0.0f);
	m_v3Max = vector3(0.0f);
	m_v3Min = vector3(0.0f);

	m_EntityList = std::vector<uint>();
	m_lChild = std::vector<MyOctant*>();

	m_pParent = nullptr;
	m_pRoot = nullptr;

	m_pChild[8] = new MyOctant[8];
	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}

void MyOctant::ConstructList(void)
{
	// start at the root
	// only gets called in root's constructor
}
