
#ifndef _ML_DRAWOBJECT_H_
#define _ML_DRAWOBJECT_H_

#include "PMath.h"
#include "GraphObj.h"
#include "ML_Material.h"

/// base object for drawing

class DrawObject {
public:
	DrawObject(Material* pMaterial) : mp_Material(pMaterial) {
		PMath::Mat44Identity(m_LocalToWorld);
	}

	virtual ~DrawObject() { }

	void SetMatrix(float* pMatrix) {
		PMath::Mat44Set(m_LocalToWorld, pMatrix);
	}

	void LoadMatrix() {
		glMatrixMode(GL_MODELVIEW);
		glMultMatrixf(m_LocalToWorld);
	}

	void BindMaterial() {
		mp_Material->Bind();
	}

	void UnbindMaterial() {
		mp_Material->Unbind();
	}

	void DrawBoundingBox() {
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] + m_Extent[2]);

		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] + m_Extent[2]);

		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] - m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] - m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] + m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] - m_Extent[2]);
		glVertex3f(m_Center[0] + m_Extent[0], m_Center[1] + m_Extent[1], m_Center[2] + m_Extent[2]);
		glEnd();
	}

	virtual void Render() = 0;
	virtual void CalcBoundingBox() = 0;

	PMath::Vec3f	m_Center;
	PMath::Vec3f	m_Extent;

private:
	float		m_LocalToWorld[16];
	Material*	mp_Material;
};

/// an infinite plane in XY

class DrawPlane : public DrawObject {
public:
	DrawPlane(Material* pMaterial) : DrawObject(pMaterial) { }
	virtual ~DrawPlane() { }

	virtual void Render() {
		m_Plane.Render();
	}

	virtual void CalcBoundingBox() {
		PMath::Vec3fZero(m_Center);
		m_Extent[0] = 1.0e6f;
		m_Extent[1] = 1.0e6f;
		m_Extent[2] = k0;
	}

	GraphObj::Plane m_Plane;
};

/// a mesh

class DrawMesh : public DrawObject {
public:
	DrawMesh(Material* pMaterial) : DrawObject(pMaterial) { }
	virtual ~DrawMesh() { }

	virtual void Render() {
		mp_Mesh->Render();
	}

	virtual void CalcBoundingBox() {
		mp_Mesh->CalcBoundingBox(m_Center, m_Extent);
	}

	void SetMesh(GraphObj::Mesh* pMesh) { mp_Mesh = pMesh; CalcBoundingBox(); }

private:
	GraphObj::Mesh* mp_Mesh;
};

/// a sphere

class DrawSphere : public DrawObject {
public:
	DrawSphere(Material* pMaterial) : DrawObject(pMaterial) { }
	virtual ~DrawSphere() { }

	virtual void Render() {
		m_Sphere.Render();
	}

	virtual void CalcBoundingBox() {
		PMath::Vec3fZero(m_Center);
		m_Extent[0] = m_Sphere.m_Radius;
		m_Extent[1] = m_Sphere.m_Radius;
		m_Extent[2] = m_Sphere.m_Radius;
	}

	GraphObj::Sphere m_Sphere;
};

#endif