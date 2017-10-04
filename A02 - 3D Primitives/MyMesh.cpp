#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// pointy point
	float pointZ = 0.0f;
	pointZ = a_fHeight / 2;
	vector3 point(0.0f, 0.0f, pointZ);

	// center point on flat end of cone
	float centerZ = a_fHeight;
	centerZ = -pointZ;
	vector3 center(point.x, point.y, centerZ);

	// first, angle
	float angle = (2 * glm::pi<float>()) / a_nSubdivisions;
	//std::cout << "subdivisions = " << a_nSubdivisions << ", angle = " << angle << ", 2 * pi = " << angle * a_nSubdivisions << ", pi = " << glm::pi<float>() << "\n";

	// next, temp angle to increment as we go thru loop without
	// having to override or recall the original angle value
	float tempAngle = angle;

	// then temp vector to keep track of where in (x,y) to put next point
	vector2 temp(a_fRadius, 0.0f);

	// array points
	std::vector<vector3> basePoints = GenerateCircleOfPoints(a_fRadius, a_nSubdivisions, angle, centerZ);

	// generate faces, but how?
	// you need to loop over the points
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		int next = i + 1;
		if (i == a_nSubdivisions - 1)
			next = 0;

		// base
		AddTri(basePoints[next], basePoints[i], center);

		// coney pointy part
		AddTri(basePoints[i], basePoints[next], point);
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// center point on top
	float centerTopZ = 0.0f;
	centerTopZ = a_fHeight / 2;
	vector3 centerTop(0.0f, 0.0f, centerTopZ);

	// center point on flat end of cylinder
	float centerBottomZ = a_fHeight;
	centerBottomZ = -centerTopZ;
	vector3 centerBottom(centerTop.x, centerTop.y, centerBottomZ);

	// first, angle
	float angle = (2 * glm::pi<float>()) / a_nSubdivisions;
	//std::cout << "subdivisions = " << a_nSubdivisions << ", angle = " << angle << ", 2 * pi = " << angle * a_nSubdivisions << ", pi = " << glm::pi<float>() << "\n";

	// next, temp angle to increment as we go thru loop without
	// having to override or recall the original angle value
	float tempAngle = angle;

	// then temp vector to keep track of where in (x,y) to put next point
	vector2 temp(a_fRadius, 0.0f);

	// array points
	std::vector<vector3> topBasePoints = GenerateCircleOfPoints(a_fRadius, a_nSubdivisions, angle, centerTopZ);

	// array points
	std::vector<vector3> bottomBasePoints(a_nSubdivisions);

	// loop for each subdivision
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		// set x and y based on top base points, then set z to height (top z = 0 for all points)
		bottomBasePoints[i] = vector3(topBasePoints[i].x, topBasePoints[i].y, centerBottomZ);
		// DEBUG
		//std::cout << "(" << bottomBasePoints[i].x << ", " << bottomBasePoints[i].y << ", " << bottomBasePoints[i].z << ")\n\n";
			//<< "\ncurrent angle = " << tempAngle << "\n\n";
	}

	// draw each face, remember BOTH SIDES
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		int next = i + 1;
		if (i == a_nSubdivisions - 1)
			next = 0;

		// top base
		AddTri(topBasePoints[i], topBasePoints[next], centerTop);

		// middle portion
		AddQuad(bottomBasePoints[i], bottomBasePoints[next], topBasePoints[i], topBasePoints[next]);

		// bottom base
		AddTri(bottomBasePoints[next], bottomBasePoints[i], centerBottom);
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// first, angle
	float angle = (2 * glm::pi<float>()) / a_nSubdivisions;
	//std::cout << "subdivisions = " << a_nSubdivisions << ", angle = " << angle << ", 2 * pi = " << angle * a_nSubdivisions << ", pi = " << glm::pi<float>() << "\n";

	// floats to keep track of z-values of top and bottom
	float topZ = 0.0f;
	topZ = a_fHeight;
	float bottomZ = a_fHeight;
	bottomZ = -topZ;

	// array points
	std::vector<vector3> topOuterRadiusPoints = GenerateCircleOfPoints(a_fOuterRadius, a_nSubdivisions, angle, topZ);
	std::vector<vector3> topInnerRadiusPoints = GenerateCircleOfPoints(a_fInnerRadius, a_nSubdivisions, angle, topZ);

	std::vector<vector3> bottomOuterRadiusPoints(a_nSubdivisions);
	std::vector<vector3> bottomInnerRadiusPoints(a_nSubdivisions);
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		bottomOuterRadiusPoints[i] = vector3(topOuterRadiusPoints[i].x, topOuterRadiusPoints[i].y, bottomZ);
		bottomInnerRadiusPoints[i] = vector3(topInnerRadiusPoints[i].x, topInnerRadiusPoints[i].y, bottomZ);
	}

	// draw each face, remember BOTH SIDES
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		int next = i + 1;
		if (i == a_nSubdivisions - 1)
			next = 0;

		// top ring
		AddQuad(topOuterRadiusPoints[i], topOuterRadiusPoints[next], topInnerRadiusPoints[i], topInnerRadiusPoints[next]);

		// outer middle
		AddQuad(bottomOuterRadiusPoints[i], bottomOuterRadiusPoints[next], topOuterRadiusPoints[i], topOuterRadiusPoints[next]);

		// inner middle
		AddQuad(bottomInnerRadiusPoints[next], bottomInnerRadiusPoints[i], topInnerRadiusPoints[next], topInnerRadiusPoints[i]);

		// bottom ring
		AddQuad(bottomOuterRadiusPoints[next], bottomOuterRadiusPoints[i], bottomInnerRadiusPoints[next], bottomInnerRadiusPoints[i]);
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// center top
	vector3 centerTop(0.0f, 0.0f, a_fRadius);

	// center bottom
	vector3 centerBottom(centerTop.x, centerTop.y, -a_fRadius);

	// angle of horizontal increment (for a ring of points at a given height)
	float angle = (2 * glm::pi<float>()) / a_nSubdivisions;
	//std::cout << "subdivisions = " << a_nSubdivisions << ", angle = " << angle << ", 2 * pi = " << angle * a_nSubdivisions << ", pi = " << glm::pi<float>() << "\n";

	// angle of vertical increment
	float vertAngle = glm::pi<float>() / a_nSubdivisions;
	//std::cout << "vertical angle = " << vertAngle << ", pi = " << vertAngle * a_nSubdivisions << "\n";

	// temp angle for iterating thru the rings
	// this goes from 0 to pi, rather than 0 to 2pi like the temp angle in GenerateCircleOfPoints
	float tempVertAngle = vertAngle;
	
	// temps to keep track of radius and height at which to generate a new ring
	float tempRadius = 0.0f;
	float tempHeight = 0.0f;

	// std::vector to keep track of each ring
	std::vector<std::vector<vector3>> rings(a_nSubdivisions - 1);

	// DEBUG
	//std::cout << "ABOUT TO ENTER RING LOOP\ncurrent radius = " << tempRadius << "\ncurrent height = " << tempHeight << "\ncurrent angle = " << tempVertAngle << "\n\n";

	// you may need to make the # of iterations (subdivisions - 1) if the last ring is at the height of centerBottom
	for (int i = 0; i < a_nSubdivisions - 1; i++)
	{
		// update the radius and height 
		// this gets done first so that the first ring isn't at the same height as the center top
		tempRadius = a_fRadius * glm::sin(tempVertAngle);
		tempHeight = (a_fRadius * glm::cos(tempVertAngle));

		// DEBUG
		//std::cout << "current radius = " << tempRadius << "\ncurrent height = " << tempHeight << "\ncurrent angle = " << tempVertAngle << "\n\n";

		// generate a circle of points at the current radius and height
		rings[i] = GenerateCircleOfPoints(tempRadius, a_nSubdivisions, angle, tempHeight);

		// increment angle
		tempVertAngle += vertAngle;
	}

	int size = rings.size();
	bool middleDone = false;
	// now, draw points!
	// draw each base face
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		int nextI = i + 1;
		if (i == a_nSubdivisions - 1)
			nextI = 0;

		// top base
		AddTri(rings[0][i], rings[0][nextI], centerTop);

		// bottom base
		AddTri(rings[size - 1][nextI], rings[size - 1][i], centerBottom);
	}

	// an alternate approach
	// a separate loop for the middle faces that only executes once
	for (int i = 0; i < size - 1; i++)
	{
		int nextI = i + 1;

		for (int j = 0; j < a_nSubdivisions; j++)
		{
			int nextJ = j + 1;
			if (j == a_nSubdivisions - 1)
				nextJ = 0;

			// DEBUG
			//std::cout << "now, do both sides of rings[" << nextI << "][" << j << "] and rings[" << i << "][" << nextJ << "]\n";
			
			// middle faces
			AddQuad(rings[nextI][j], rings[nextI][nextJ], rings[i][j], rings[i][nextJ]);
		}
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}

// helper method??? (hopefully in the future, prevent repeated code)
std::vector<vector3> MyMesh::GenerateCircleOfPoints(float radius, float numSubdivisions, float angle, float zValue)
{
	vector2 temp(radius, 0.0f);
	float tempAngle = angle;

	std::vector<vector3> circleOfPoints(numSubdivisions);
	
	for (int i = 0; i < numSubdivisions; i++)
	{
		circleOfPoints[i] = vector3(temp, zValue);

		// DEBUG
		//std::cout << "(" << circleOfPoints[i].x << ", " << circleOfPoints[i].y << ", " << circleOfPoints[i].z << ")"
		//	<< "\ncurrent angle = " << tempAngle << "\n\n";

		temp = radius * vector2(glm::cos(tempAngle), glm::sin(tempAngle));
		tempAngle += angle;
	}

	return circleOfPoints;
}
