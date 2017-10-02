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

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);

	//*
	// pointy point
	vector3 point(0.0f, 0.0f, 0.0f);

	// center point on flat end of cone
	vector3 center(point.x, point.y, a_fHeight);

	// first, angle
	float angle = (2 * glm::pi<float>()) / a_nSubdivisions;
	std::cout << "subdivisions = " << a_nSubdivisions << ", angle = " << angle << ", 2 * pi = " << angle * a_nSubdivisions << ", pi = " << glm::pi<float>() << "\n";

	// next, temp angle to increment as we go thru loop without
	// having to override or recall the original angle value
	float tempAngle = angle;

	// then temp vector to keep track of where in (x,y) to put next point
	vector2 temp(a_fRadius, 0.0f);

	// array points
	std::vector<vector3> basePoints = GenerateCircleOfPoints(a_fRadius, a_nSubdivisions, angle, a_fHeight);

	// generate faces, but how?
	// you need to loop over the points
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		int next = i + 1;
		if (i == a_nSubdivisions - 1)
			next = 0;

		// base, both sides
		AddTri(basePoints[i], basePoints[next], center);
		AddTri(basePoints[next], basePoints[i], center);

		// coney pointy part, both sides
		AddTri(basePoints[i], basePoints[next], point);
		AddTri(basePoints[next], basePoints[i], point);
	}

	// loop for each subdivision
	//for (int i = 0; i < a_nSubdivisions; i++)
	//{
	//	// set point based on current directional vector
	//	basePoints[i] = vector3(temp, a_fHeight);
	//
	//	// DEBUG
	//	std::cout << "(" << basePoints[i].x << ", " << basePoints[i].y << ", " << basePoints[i].z << ")"
	//		<< "\ncurrent angle = " << tempAngle << "\n\n";
	//	
	//	// increment directional vector by angle
	//	// but, how?
	//	//temp = vector2(glm::cos(basePoints[i].x), glm::sin(basePoints[i].y));
	//	//if (i == 0);
	//
	//	// here's a thought
	//	// set the temp = radius * (cos(angle), sin(angle));
	//	temp = a_fRadius * vector2(glm::cos(tempAngle), glm::sin(tempAngle));
	//	// increment current angle by original angle amount for a uniform increment
	//	tempAngle += angle;
	//
	//	// what I'm doing now, (not working)
	//	//temp = vector2(glm::sin(temp.x / a_fRadius), glm::cos(temp.y / a_fRadius));
	//	// else temp = vector
	//}
	//*/

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

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	
	//*
	// center point on top
	vector3 centerTop(0.0f, 0.0f, 0.0f);

	// center point on flat end of cylinder
	vector3 centerBottom(centerTop.x, centerTop.y, a_fHeight);

	// first, angle
	float angle = (2 * glm::pi<float>()) / a_nSubdivisions;
	std::cout << "subdivisions = " << a_nSubdivisions << ", angle = " << angle << ", 2 * pi = " << angle * a_nSubdivisions << ", pi = " << glm::pi<float>() << "\n";

	// next, temp angle to increment as we go thru loop without
	// having to override or recall the original angle value
	float tempAngle = angle;

	// then temp vector to keep track of where in (x,y) to put next point
	vector2 temp(a_fRadius, 0.0f);

	// array points
	std::vector<vector3> topBasePoints = GenerateCircleOfPoints(a_fRadius, a_nSubdivisions, angle, 0.0f);

	// loop for each subdivision
	//for (int i = 0; i < a_nSubdivisions; i++)
	//{
	//	// set point based on current directional vector
	//	topBasePoints[i] = vector3(temp, 0);
	//
	//	// DEBUG
	//	std::cout << "(" << topBasePoints[i].x << ", " << topBasePoints[i].y << ", " << topBasePoints[i].z << ")"
	//		<< "\ncurrent angle = " << tempAngle << "\n\n";
	//
	//	// increment directional vector by angle
	//	// but, how?
	//	//temp = vector2(glm::cos(topBasePoints[i].x), glm::sin(topBasePoints[i].y));
	//	//if (i == 0);
	//
	//	// here's a thought
	//	// set the temp = radius * (cos(angle), sin(angle));
	//	temp = a_fRadius * vector2(glm::cos(tempAngle), glm::sin(tempAngle));
	//	// increment current angle by original angle amount for a uniform increment
	//	tempAngle += angle;
	//
	//	// what I'm doing now, (not working)
	//	//temp = vector2(glm::sin(temp.x / a_fRadius), glm::cos(temp.y / a_fRadius));
	//	// else temp = vector
	//}

	// array points
	std::vector<vector3> bottomBasePoints(a_nSubdivisions);

	// loop for each subdivision
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		// set x and y based on top base points, then set z to height (top z = 0 for all points)
		bottomBasePoints[i] = vector3(topBasePoints[i].x, topBasePoints[i].y, a_fHeight);
		// DEBUG
		std::cout << "(" << bottomBasePoints[i].x << ", " << bottomBasePoints[i].y << ", " << bottomBasePoints[i].z << ")\n\n";
			//<< "\ncurrent angle = " << tempAngle << "\n\n";
	}
	//*/

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

	// Replace this with your code
	//GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	//*
	// center point on top
	//vector3 centerTop(0.0f, 0.0f, 0.0f);

	// center point on flat end of cylinder
	//vector3 centerBottom(centerTop.x, centerTop.y, a_fHeight);

	// first, angle
	float angle = (2 * glm::pi<float>()) / a_nSubdivisions;
	std::cout << "subdivisions = " << a_nSubdivisions << ", angle = " << angle << ", 2 * pi = " << angle * a_nSubdivisions << ", pi = " << glm::pi<float>() << "\n";

	// next, temp angle to increment as we go thru loop without
	// having to override or recall the original angle value
	//float tempOuterAngle = angle;
	//float tempInnerAngle = angle;

	// then temp vector to keep track of where in (x,y) to put next point
	//vector2 temp(a_fRadius, 0.0f);

	// array points
	std::vector<vector3> topOuterRadiusPoints = GenerateCircleOfPoints(a_fOuterRadius, a_nSubdivisions, angle, 0.0f);
	std::vector<vector3> topInnerRadiusPoints = GenerateCircleOfPoints(a_fInnerRadius, a_nSubdivisions, angle, 0.0f);

	std::vector<vector3> bottomOuterRadiusPoints(a_nSubdivisions);
	std::vector<vector3> bottomInnerRadiusPoints(a_nSubdivisions);
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		bottomOuterRadiusPoints[i] = vector3(topOuterRadiusPoints[i].x, topOuterRadiusPoints[i].y, a_fHeight);
		bottomInnerRadiusPoints[i] = vector3(topInnerRadiusPoints[i].x, topInnerRadiusPoints[i].y, a_fHeight);
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

	// Replace this with your code
	GenerateCube(a_fRadius * 2.0f, a_v3Color);
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
		std::cout << "(" << circleOfPoints[i].x << ", " << circleOfPoints[i].y << ", " << circleOfPoints[i].z << ")"
			<< "\ncurrent angle = " << tempAngle << "\n\n";

		temp = radius * vector2(glm::cos(tempAngle), glm::sin(tempAngle));
		tempAngle += angle;
	}

	/*
	// loop for each subdivision
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		// set point based on current directional vector
		basePoints[i] = vector3(temp, a_fHeight);

		// DEBUG
		std::cout << "(" << basePoints[i].x << ", " << basePoints[i].y << ", " << basePoints[i].z << ")"
			<< "\ncurrent angle = " << tempAngle << "\n\n";
		
		// increment directional vector by angle
		// but, how?
		//temp = vector2(glm::cos(basePoints[i].x), glm::sin(basePoints[i].y));
		//if (i == 0);

		// here's a thought
		// set the temp = radius * (cos(angle), sin(angle));
		temp = a_fRadius * vector2(glm::cos(tempAngle), glm::sin(tempAngle));
		// increment current angle by original angle amount for a uniform increment
		tempAngle += angle;

		// what I'm doing now, (not working)
		//temp = vector2(glm::sin(temp.x / a_fRadius), glm::cos(temp.y / a_fRadius));
		// else temp = vector
	}
	*/

	return circleOfPoints;
}
