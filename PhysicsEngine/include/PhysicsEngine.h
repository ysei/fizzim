
/** @file PhysicsEngine.h */
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

#ifndef _PHYSICENGINE_H_
#define _PHYSICENGINE_H_

#include "PMath.h"
#include "PhysicsEngineDef.h"
#include "CollisionEngineDef.h"

/** @namespace	Physics 
	@brief		Contains the interface to the physics engine
*/

namespace Physics {

	class PEAux;	// forward declaration - hidden implementation of physics engine

	/** @class	Engine
		@brief	Physics Engine for the Games Gem 4 chapter "Creating a Verlet Based Physics Engine"
	*/

	class Engine {
	public:
		Engine();
		~Engine();

		/*
                         ____            _ _
                        | __ )  ___   __| (_) ___ ___
                        |  _ \ / _ \ / _` | |/ _ Y __|
                        | |_) | (_) | (_| | |  __|__ \
                        |____/ \___/ \__,_|_|\___|___/
		*/
		//----------------------- Rigid RigidBody Factory

		/**
		* create an infinite plane and add it to the physics engine
		* Adds body at rest, at the origin, and with default properties
		* 
		* @param plane parameterized plane
		* @return the unique ID of the new rigid body
		*/
		uint32	AddRigidBodyPlane(PMath::Plane& plane);

		/**
		* create a sphere and add it to the physics engine
		* Adds body at rest, at the origin, and with default properties
		* 
		* @return the unique ID of the new rigid body
		*/
		uint32	AddRigidBodySphere(Real radius);

		/**
		* create a spring mesh and add it to the physics engine
		* Adds body at rest, at the origin, and with default properties
		* 
		* @return the unique ID of the new rigid body
		*/
		uint32	AddSpringMesh();

		/**
		* Removes a rigid body from the simulation.
		* Does nothing if the specified id doesn't exist
		* If the rigid body is connected to springs, the corresponding springs will be removed
		* 
		* @param id    id of the body to remove
		* @return		true if successfully removed
		*/
		bool	RemoveRigidBody(uint32 id);

		void	RemoveAll();

		//----------------------- RigidBody Properties

		enum ERigidBodyBool			{ propActive, propUseGravity, propCollidable, propSpinnable, propTranslatable };
		enum ERigidBodyScalar		{ propAngularVelocityDamp, propLinearVelocityDamp, propMass };
		enum ERigidBodyVector		{ propExtent, propPosition, propVelocity };
		enum ERigidBodyQuat 		{ propOrientation };
		enum ERigidBodyVectorArray	{ propPositions };
		enum ERigidBodyIntArray		{ propIndices };

		void				SetRigidBodyBool			(uint32 id, ERigidBodyBool			prop,	bool value);
		bool				GetRigidBodyBool			(uint32 id, ERigidBodyBool			prop);
		void				SetRigidBodyScalar			(uint32 id, ERigidBodyScalar		prop,	Real value);
		Real				GetRigidBodyScalar			(uint32 id, ERigidBodyScalar		prop);
		void 				SetRigidBodyVec3f			(uint32 id, ERigidBodyVector		prop,	PMath::Vec3f value);
		PMath::Vec3f*		GetRigidBodyVec3fPtr		(uint32 id, ERigidBodyVector		prop);
		void				SetRigidBodyQuat			(uint32 id, ERigidBodyQuat			prop,	PMath::Quaternion value);
		PMath::Quaternion*	GetRigidBodyQuatPtr			(uint32 id, ERigidBodyQuat			prop);

		void				SetRigidBodyVectorArray		(uint32 id,	ERigidBodyVectorArray	prop,	PMath::Vec3f const*const value, int byteStride, int count);
		void				SetRigidBodyIntArray		(uint32 id, ERigidBodyIntArray		prop,	int const*const val, int count);

		void				GetRigidBodyTransformMatrix	(uint32 id, Real *const pResult);

		/*
                      ____             _
                     / ___| _ __  _ __(_)_ __   __ _ ___
                     \___ \| '_ \| '__| | '_ \ / _` / __|
                      ___) | |_) | |  | | | | | (_| \__ \
                     |____/| .__/|_|  |_|_| |_|\__, |___/
                           |_|                 |___/
		*/

		//----------------------- Spring Factory

		/// create and add a spring the system, and return the ID of the new spring
		uint32	AddSpring();

		/// remove a spring from the system
		bool	RemoveSpring(uint32 id);

		enum ESpringBool	{ propResistCompression };	// if true, the spring will push if compressed, and pull if stretched. If false it will only pull if stretched.
		enum ESpringScalar	{ propSpringStiffness, propSpringDamping, propSpringRestLength };
		enum ESpringVector	{ propAttachPointA, propAttachPointB };
		enum ESpringUint32	{ propBodyA, propBodyB };

		void				SetSpringBool				(uint32 id, ESpringBool			prop,	bool value);
		bool				GetSpringBool				(uint32 id, ESpringBool			prop);
		void				SetSpringUInt32				(uint32 id, ESpringUint32		prop,	uint32 value);
		uint32				GetSpringUInt32				(uint32 id, ESpringUint32		prop);
		void				SetSpringScalar				(uint32 id, ESpringScalar		prop,	Real value);
		Real				GetSpringScalar				(uint32 id, ESpringScalar		prop);
		void 				SetSpringVec3f				(uint32 id, ESpringVector		prop,	PMath::Vec3f value);
		PMath::Vec3f*		GetSpringVec3fPtr			(uint32 id, ESpringVector		prop);

		/*
               ____                _             _       _
              / ___|___  _ __  ___| |_ _ __ __ _(_)_ __ | |_ ___
             | |   / _ \| '_ \/ __| __| '__/ _` | | '_ \| __/ __|
             | |__| (_) | | | \__ \ |_| | | (_| | | | | | |_\__ \
              \____\___/|_| |_|___/\__|_|  \__,_|_|_| |_|\__|___/
		*/

		uint32				AddDistanceConstraint		(uint32 a, uint32 b, Real distance, Real tolerance);
		bool				RemoveConstraint			(uint32 id);

		enum EConstraintBool	{ propConstraintActive };
		enum EConstraintScalar	{ propConstraintDistance };

		void				SetConstraintBool			(uint32 id, EConstraintBool		prop, bool value);
		bool				GetConstraintBool			(uint32 id, EConstraintBool		prop);
		void				SetConstraintScalar			(uint32 id, EConstraintScalar	prop, Real value);
		Real				GetConstraintScalar			(uint32 id, EConstraintScalar	prop);

		/*
                 ____                              _
                |  _ \ _   _ _ __   __ _ _ __ ___ (_) ___ ___
                | | | | | | | '_ \ / _` | '_ ` _ \| |/ __/ __|
                | |_| | |_| | | | | (_| | | | | | | | (__\__ \
                |____/ \__, |_| |_|\__,_|_| |_| |_|_|\___|___/
                       |___/
		*/

		//----------------------- Force inputs during a frame

		/// add an impulse (instantaneous force) to a particular body
		void				AddImpulse					(uint32 id, PMath::Vec3f force);		

		/// simply stop an object in its tracks; doesn't stop spinning
		void				StopMoving					(uint32 id);
				
		//----------------------- Torque inputs during a frame

		/// add twist (instantaneous torque) to a particular body
		void				AddTwist					(uint32 id, PMath::Vec3f twist);	
		void				AddTwist					(uint32 id, PMath::Vec3f force, PMath::Vec3f position);

		/// stop an object's rotation completely; doesn't stop motion
		void				StopSpinning				(uint32 id);

		/// Set the gravity vector that applies to all bodies, if body's gravity flag is true
		void				SetGravity					(PMath::Vec3f val);


		/*
               ____  _                 _       _   _
              / ___|(_)_ __ ___  _   _| | __ _| |_(_) ___  _ __
              \___ \| | '_ ` _ \| | | | |/ _` | __| |/ _ \| '_ \
               ___) | | | | | | | |_| | | (_| | |_| | (_) | | | |
              |____/|_|_| |_| |_|\__,_|_|\__,_|\__|_|\___/|_| |_|
		*/
		//----------------------- Control the simulation

		/**
		* Sets a collision callback object. Note that the object is owned by the application,
		* not the physics engine, and will not be deleted by the physics engine. It is illegal
		* to call any physics engine functions from the callback.
		* 
		* @param pCB The callback object to use when collisions occur.
		*/
		void				SetCollisionCallback(Collision::ICallback* pCB);

		/// Set the minimum time step to ensure stability
		void				SetMinTimeStep(Real dt);

		///	Run one step of the simulation given dt in seconds
		void				Simulate(Real dt);

	protected:
		PEAux*	m_pAux;
	};

}	// end Physics namespace

#endif
