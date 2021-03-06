
/** @file CollisionEngine.cpp 
	@brief	collision response and resolution */

/*
---------------------------------------------------------------------------------------------------
Meshula Physics Demo
Created for Games Programming Gems IV
Copyright (c) 2003 Nick Porcino, http://meshula.net

The MIT License: http://www.opensource.org/licenses/mit-license.php

Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, 
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or 
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---------------------------------------------------------------------------------------------------
*/

#include "PMath.h"
#include "RigidBody.h"
#include "CollisionEngine.h"
#include "opcode.h"

using namespace PMath;
using Physics::RigidBody;

#define ICEMATHS_SPHERE(a, b) IceMaths::Sphere* a = (IceMaths::Sphere*) b;
namespace Collision {

	Sphere :: Sphere(const Real radius) : m_Radius(radius) {
		m_pAux = (void*) new IceMaths::Sphere();
		ICEMATHS_SPHERE(s, m_pAux);
		s->SetRadius(radius);
	}

	Sphere :: ~Sphere() { 
		ICEMATHS_SPHERE(s, m_pAux);
		delete s;
	}
}


namespace Collision {

	/** @class Contact
		@brief records contact parameters
	 */

	class Contact
	{
	public:
		Contact();
		PMath::Vec3f		m_Position;
		PMath::Vec3f		m_Normal;
		Real				m_ContactTime;
		Real				m_PenetrationDepth;
		Physics::RigidBody*	m_pBodyA;
		Physics::RigidBody*	m_pBodyB;
	};

	Contact::Contact() {
	}


/*
                       ____      _ _ _     _
                      / ___|___ | | (_)___(_) ___  _ __
                     | |   / _ \| | | / __| |/ _ \| '_ \
                     | |__| (_) | | | \__ \ | (_) | | | |
                   ___\____\___/|_|_|_|___/_|\___/|_| |_|
                  |  _ \  ___| |_ ___  ___| |_(_) ___  _ __
                  | | | |/ _ \ __/ _ \/ __| __| |/ _ \| '_ \
                  | |_| |  __/ ||  __/ (__| |_| | (_) | | | |
                  |____/ \___|\__\___|\___|\__|_|\___/|_| |_|
 */


typedef bool (*collfn)(Contact*, RigidBody* pBodyA, RigidBody* pBodyB);
typedef void (*resfn) (Contact*);



bool Collide_InfPlane_InfPlane(Contact* pContact, RigidBody* pBodyA, RigidBody* pBodyB)
{
	bool collided;

	// if neither body can move return true if they are not parallel
	if (!(pBodyA->GetTranslatable() & pBodyA->GetSpinnable() & pBodyB->GetTranslatable() & pBodyB->GetSpinnable())) {
		collided = !Vec3fEqual(pBodyA->m_Extent, pBodyB->m_Extent, kEps);
	}
	else {
		// if either can move, we must assume that the chance that they have not collided is exceedingly small
		collided = true;
	}

	return collided;
}
// cf: www.gamasutra.com/features/19991018/Gomez_1.htm

bool Collide_InfPlane_Sphere(Contact* pContact, RigidBody* pPlaneBody, RigidBody* pSphere)
{
	PMath::Plane* pPlane = &((Collision::Plane*) pPlaneBody->m_pCollideGeo)->m_Plane;

	Vec3f c0, c1;
	PMath::Vec3fSet(c0, pSphere->m_StateT0.m_Position);
	PMath::Vec3fSet(c1, pSphere->m_StateT1.m_Position);
	Real d0 = pPlane->DistanceToPoint(c0);
	Real d1 = pPlane->DistanceToPoint(c1);
	Real radius = ((Collision::Sphere*) pSphere->m_pCollideGeo)->m_Radius;

	bool retval = false;

	if (PMath::Abs(d0) <= radius) {
		Vec3fSet(pContact->m_Normal, pPlane->m_Normal);
		pContact->m_ContactTime = k0;
		retval = true;							// not supposed to get here; this engine never allows intersected positions
	}
	else if (d0 > radius && d1 < radius) {		// if penetrated this frame
		Vec3fSet(pContact->m_Normal, pPlane->m_Normal);
		Real u = (d0 - radius) / (d0 - d1);		// normalized time of first contact
		Vec3fScale(c1, u);
		Vec3fScale(c0, k1 - u);
		Vec3fAdd(pContact->m_Position, c0, c1);		// calc center of sphere at point of first contact
		retval = true;
		pContact->m_ContactTime = u;
	}
	return retval;
}

bool Collide_Sphere___InfPlane(Contact* pContact, RigidBody* pSphere, RigidBody* pPlane)
{
	return Collide_InfPlane_Sphere(pContact, pPlane, pSphere);
}

// Quadratic Formula from http://www.gamasutra.com/features/19991018/Gomez_2.htm
// returns true if both roots are real

inline bool QuadraticFormula(Real a, Real b, Real c, Real& root1, Real& root2) 
{
	Real q = b*b - k4 * a * c;
	if (q >= k0) {
		Real sq = Sqrt(q);
		Real d = k1 / (k2 * a);
		root1 = (-b + sq) * d;
		root2 = (-b - sq) * d;
		return true;			// real roots
	}
	else {
		return false;			// complex roots
	}
}

bool Collide_Sphere___Sphere(Contact* pContact, RigidBody* pBodyA, RigidBody* pBodyB)
{
	bool	retval = false;

	Real	u0, u1;
	Real	radiusA = ((Collision::Sphere*) pBodyA->m_pCollideGeo)->m_Radius;
	Real	radiusB = ((Collision::Sphere*) pBodyB->m_pCollideGeo)->m_Radius;
	Vec3f va;
			Vec3fSubtract(va, pBodyA->m_StateT1.m_Position, pBodyA->m_StateT0.m_Position);
	Vec3f vb;
			Vec3fSubtract(vb, pBodyB->m_StateT1.m_Position, pBodyB->m_StateT0.m_Position);
	Vec3f vab;
			Vec3fSubtract(vab, vb, va);
	Vec3f ab;
			Vec3fSubtract(ab, pBodyB->m_StateT0.m_Position, pBodyA->m_StateT0.m_Position);
	Real	rab = radiusA + radiusB;
	Real	a = Vec3fDot(vab, vab);				// u*u coefficient
	Real	b = k2 * Vec3fDot(vab, ab);				// u coefficient
	Real	c = Vec3fDot(ab, ab) - rab * rab;	// constant term

#if 0

	// check if they are overlapping
	if (c <= k0) {
		u0 = k0;
		u1 = k0;
		retval = true;
	}

	if (!retval) {
		// they didn't hit if they are moving perfectly parallel and at the same speed
		if (Abs(a) < kEps) {
			retval = false;
		}

		// check if they hit
		else if (QuadraticFormula(a, b, c, u0, u1)) {
			if (u0 > u1) {
				Real temp = u0;
				u0 = u1;
				u1 = temp;
			}
			if (u0 < k0 && u1 < k0) retval = false;
			else if (u1 > k1 && u1 > k1) retval = false;
			else retval = true;
		}
	}

#else

	// test for overlap
	if (c <= k0) {
		retval = true;
		u0 = u1 = k0;
	}
	else if (QuadraticFormula(a, b, c, u0, u1)) {
		if ((u0 > k0) && (u0 <= u1)) {
			retval = true;					// time of contact was u0
		}
		else if ((u1 > 0) && (u1 < k1)) {
			retval = true;					// time of contact was u1
		}
	}
	else {
		retval = false;
	}

#endif


	if (retval) {
		Vec3fSubtract(pContact->m_Normal, pBodyA->m_StateT1.m_Position, pBodyB->m_StateT1.m_Position);
		Vec3fNormalize(pContact->m_Normal, pContact->m_Normal);
	}

	return retval;
}

collfn CollisionFunctions[2][2] = {

		// infinite plane			sphere
	Collide_InfPlane_InfPlane,	Collide_InfPlane_Sphere,	// infinite plane
	Collide_Sphere___InfPlane,	Collide_Sphere___Sphere,	// sphere

};

Contact* Collision::Engine::TestCollision(RigidBody* pBodyA, RigidBody* pBodyB)
{
	Contact* pRetVal = 0;

	// get a contact pointer
	if (m_FreePool.size() == 0) {
		APILOG("Contact FreePool is empty\n");
	}

	Contact* pContact = m_FreePool.front();		// get first available
	m_FreePool.pop_front();						// and drop it out of the pool

	if (CollisionFunctions[pBodyA->m_pCollideGeo->GetKind()][pBodyB->m_pCollideGeo->GetKind()](pContact, pBodyA, pBodyB)) {
		m_Contacts.push_back(pContact);
		pContact->m_pBodyA = pBodyA;
		pContact->m_pBodyB = pBodyB;
		pRetVal = pContact;						// point to current contact
	}
	else {
		m_FreePool.push_back(pContact);			// didn't need it, give it back
	}

	return pRetVal;
}

/*
                       ____      _ _ _     _
                      / ___|___ | | (_)___(_) ___  _ __
                     | |   / _ \| | | / __| |/ _ \| '_ \
                     | |__| (_) | | | \__ \ | (_) | | | |
                 ____ \____\___/|_|_|_|___/_|\___/|_| |_|
                |  _ \ ___ ___  ___ | |_   _| |_(_) ___  _ __
                | |_) / _ Y __|/ _ \| | | | | __| |/ _ \| '_ \
                |  _ <  __|__ \ (_) | | |_| | |_| | (_) | | | |
                |_| \_\___|___/\___/|_|\__,_|\__|_|\___/|_| |_|
 */




void Resolve_InfPlane_InfPlane(Contact* pContact)
{
}



/** @todo - check for the condition where an object can be put to sleep
			since it has hit a stationary object, it may be able to come to
			rest upon it
 */

void _ResolveSphereVsPlane(Contact* pContact)
{
	RigidBody* pPlane = pContact->m_pBodyA;
	RigidBody* pSphere = pContact->m_pBodyB;
	Vec3f temp;

	// calculate point of contact on sphere
	Vec3f contact;
	Collision::Sphere* pCSphere = (Collision::Sphere*) pSphere->m_pCollideGeo;
	Vec3fSetScaled(contact, -pCSphere->m_Radius, pContact->m_Normal);

	Vec3f velocityA;
	Vec3fSet(velocityA, pSphere->m_StateT1.m_Velocity);

	// if object can spin, calculate velocity of the contact point
	if (pSphere->GetSpinnable()) {
		Vec3fCross(temp, pSphere->m_StateT1.m_AngularVelocity, contact);
		Vec3fAdd(velocityA, temp);
	}
	
	// velocity is the direction of the collision normal (v dot n)
	Real velNormalComponent = Vec3fDot(velocityA, pContact->m_Normal);

	// make sure velocity is towards static object (negative)
	if (velNormalComponent < k0) {
		// impulses change velocities directly without waiting for integration
		// so calculate impulse to fix the velocities
		// we'll use Newton's Law of Restitution for Instaneous Collisions with No Friction
		// as per Chris Hecker's articles in Game Developer magazine

		// the coefficient of restitution relates incoming and outgoing relative normal velocities
		//! @todo restitution coefficient is supposed to be in the PhysicsBody
		#undef RESTITUTION
		#define RESTITUTION Real(0.60f)
		Real impulseNumerator = -(k1 + RESTITUTION) * velNormalComponent;

		Vec3fCross(temp, contact, pContact->m_Normal);
		
		if (pSphere->GetInertialKind() == Physics::kI_Sphere) {
			Vec3fScale(temp, pSphere->m_InertiaITD[0]);
		}
		else {
			Vec3fMultiply(temp, temp, pSphere->m_InertiaITD);
		}

		Vec3f temp2;
		Vec3fCross(temp2, temp, contact);
		
		// changing impulseDenominator and result to double will make the equations work for objects down to a mass of 0.1
		// with Real, the equations work down to mass of 0.2. 

		Real impulseDenominator = pSphere->GetOOMass() + Vec3fDot(temp2, pContact->m_Normal);
		
		// final velocity = initial velocity + (impulse / mass) * normal
		Vec3f impulse;
		Real result = (pSphere->GetOOMass() * impulseNumerator) / impulseDenominator;
		Vec3fSetScaled(impulse, (Real) result, pContact->m_Normal);
		Vec3fAdd(pSphere->m_StateT1.m_Velocity, impulse);

		if (pSphere->GetSpinnable()) {
			// calculate angular impulse
			// final angular momentum = init ang vel + (collision point DOT impulse * normal) / inertia tensor
			Vec3fCross(temp2, contact, impulse);
			Vec3fSubtract(pSphere->m_StateT1.m_AngularMomentum, temp2);			/// @todo subtract or add?
			// angular velocity will be recalculate on next time step
		}
	}
}




void Resolve_InfPlane_Sphere(Contact* pContact)
{
	RigidBody* pPlaneBody = pContact->m_pBodyA;
	RigidBody* pSphere = pContact->m_pBodyB;
	Vec3f adjust;
	PMath::Vec3fSet(adjust, pSphere->m_StateT1.m_Position);
	PMath::Vec3fSubtract(adjust, pContact->m_Position);					// this is the motion removed from the object
	Real residual = PMath::Vec3fLength(adjust);							// left over motion

	PMath::Vec3fSet(pSphere->m_StateT1.m_Position, pContact->m_Position);	// move sphere out of trouble

	_ResolveSphereVsPlane(pContact);

	PMath::Vec3fSet(adjust, pSphere->m_StateT1.m_Velocity);
	PMath::Vec3fNormalize(adjust, adjust);
	PMath::Vec3fMultiplyAccumulate(pSphere->m_StateT1.m_Position, residual, adjust);		// Vec3fAdd residual motion along the new velocity vector
}

void Resolve_Sphere___InfPlane(Contact* pContact)
{
	RigidBody* pTemp = pContact->m_pBodyA;
	pContact->m_pBodyA = pContact->m_pBodyB;
	pContact->m_pBodyB = pTemp;
	Vec3fNegate(pContact->m_Normal);

	Resolve_InfPlane_Sphere(pContact);					/// @todo swap!!!
}

void Resolve_Sphere___Sphere(Contact* pContact)
{
	RigidBody* pBodyA = pContact->m_pBodyA;
	RigidBody* pBodyB = pContact->m_pBodyB;

	Vec3f temp,			temp2;
	Vec3f contactA,		contactB;
	Vec3f velocityA,	velocityB, velocityAB;

	// calculate point of contact on body A and Vec3fAdd in angular contribution
	Collision::Sphere* pCSphereA = (Collision::Sphere*) pBodyA->m_pCollideGeo;
	Collision::Sphere* pCSphereB = (Collision::Sphere*) pBodyB->m_pCollideGeo;

	Vec3fSetScaled(contactA, -pCSphereA->m_Radius, pContact->m_Normal);
	Vec3fSetScaled(contactB,  pCSphereB->m_Radius, pContact->m_Normal);
	Vec3fSet      (velocityA, pBodyA->m_StateT1.m_Velocity);
	Vec3fSet      (velocityB, pBodyB->m_StateT1.m_Velocity);

	// Vec3fAdd tangential velocity at collision point if the body can spin
	if (pBodyA->GetSpinnable()) {
		Vec3fCross(temp, pBodyA->m_StateT0.m_AngularVelocity, contactA);
		Vec3fAdd(velocityA, temp);
	}

	if (pBodyB->GetSpinnable()) {
		Vec3fCross(temp, pBodyB->m_StateT0.m_AngularVelocity, contactB);
		Vec3fAdd(velocityB, temp);
	}

	Vec3fSubtract(velocityAB, velocityA, velocityB);							// calculate relative velocity
	Real velNormalComponent = Vec3fDot(velocityAB, pContact->m_Normal);		// get velocity component along collision normal

	if (velNormalComponent < k0) {	// only accept if the two objects are moving towards each other
		// impulses change velocities directly without waiting for integration
		// so calculate impulse to fix the velocities
		// we'll use Newton's Law of Restitution for Instaneous Collisions with No Friction

		// the coefficient of restitution relates incoming and outgoing relative normal velocities		
		//! @todo restitution coefficient is supposed to be in the PhysicsBody
		#undef RESTITUTION
		#define RESTITUTION Real(0.95f)
		Real impulseNumerator = (kN1 - RESTITUTION) * velNormalComponent;	// -(1+e) (v dot n)

		Vec3fCross(temp, contactA, pContact->m_Normal);						// rAP cross n
		
		if (pBodyA->GetInertialKind() == Physics::kI_Sphere) {
			Vec3fScale(temp, pBodyA->m_InertiaITD[0]);
		}
		else {
			Vec3fMultiply(temp, temp, pBodyA->m_InertiaITD);
		}

		Vec3fCross(temp2, temp, contactA);									// Iinv (rAP cross N) cross rAP
		
		Real denTerm1 = pBodyA->GetOOMass() + pBodyB->GetOOMass();
		Vec3f temp3;
		Vec3fScale(temp3, denTerm1, pContact->m_Normal);
		denTerm1 = Vec3fDot(pContact->m_Normal, temp3);						// n dot n(mAInv + mBInv)

		Vec3fCross(temp, contactB, pContact->m_Normal);
		if (pBodyB->GetInertialKind() == Physics::kI_Sphere) {
			Vec3fScale(temp, pBodyB->m_InertiaITD[0]);
		}
		else {
			Vec3fMultiply(temp, temp, pBodyB->m_InertiaITD);
		}

		Vec3fCross(temp3, temp, contactB);									// Iinv(rBP cross N) cross rBP
		Vec3fAdd(temp3, temp3, temp2);
		denTerm1 += Vec3fDot(temp3, pContact->m_Normal);
		Real result = impulseNumerator / denTerm1;

		Vec3f impulse;
		Vec3fSetScaled(impulse, (Real) result, pContact->m_Normal);
		Vec3fMultiplyAccumulate(pBodyA->m_StateT1.m_Velocity, pBodyA->GetOOMass(), impulse);
		
		// apply opposite impulse to body B
		Vec3f negImpulse;
		Vec3fSetScaled(negImpulse, -pBodyB->GetOOMass(), impulse);
		Vec3fAdd(pBodyB->m_StateT1.m_Velocity, negImpulse);

		// calculate angular impulse
		// final angular momentum = init ang vel + (collision point DOT impulse * normal) / inertia tensor

		Vec3fSubtract(temp, pBodyA->m_StateT1.m_Position, contactA);
		Vec3fCross(temp2, temp, impulse);
		Vec3fAdd(pBodyA->m_StateT1.m_AngularMomentum, temp2);

		Vec3fSubtract(temp, pBodyB->m_StateT1.m_Position, contactB);
		Vec3fCross(temp2, temp, impulse);
		Vec3fAdd(pBodyB->m_StateT1.m_AngularMomentum, temp2);

		// angular velocity will be recalculated on next time step
	}
}

resfn ResolveFunctions[2][2] = {

	// infinite plane			sphere
	Resolve_InfPlane_InfPlane,	Resolve_InfPlane_Sphere,	// infinite plane
	Resolve_Sphere___InfPlane,	Resolve_Sphere___Sphere,	// sphere
};

void Engine::Resolve(Contact* pContact)
{
	ResolveFunctions[pContact->m_pBodyA->m_pCollideGeo->GetKind()][pContact->m_pBodyB->m_pCollideGeo->GetKind()](pContact);
	pContact->m_pBodyA->m_Collided = true;
	pContact->m_pBodyB->m_Collided = true;
}

Engine::Engine()
{
	SetCapacity(1024);		// default maximum capacity
}

Engine::~Engine()
{
}

void Engine::Begin()
{
}

void Engine::End()
{
	while (m_Contacts.size() > 0) {
		Contact* pContact = m_Contacts.back();
		m_Contacts.pop_back();
		m_FreePool.push_back(pContact);
	}
}

// capacity can only increase

void Engine::SetCapacity(int maxContacts)
{
	int numToAdd = maxContacts - (int) m_FreePool.size();
	while (--numToAdd >= 0) {
		m_FreePool.push_back(new Contact());
	}
}

}	// end namespace Collision

