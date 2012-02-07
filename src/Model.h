/*
 *  Model.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  GPL
 *
 */

class Model {

public:
	
	static void ReleaseBuffers();
	static foofoo *GetFoo(const aiScene *a, int s, int e);
  static foofoo *GetBatchFoo(GLuint t, int max_face_count, int max_model_count);
	
	
	Model(foofoo *a);
  ~Model();
	void Render(StateFoo *sf, foofoo *batch_foo = NULL);
	static void RenderFoo(StateFoo *sf, foofoo *foo, bool copy);
	float Simulate(float st, float dt, bool pushing = false);
	void Die(float dt);
	void Live(float dt);
	void Harm(Model *other);
	void Help(Model *other, float dt);
	void CollideWith(Model *other, float dt);
	void Move(int direction);
	bool MoveTo(float x, float y, float z, float dt);
	bool ClimbTo(float y, float dt);
	bool IsCollidedWith(Model *other);
	
	void SetPosition(float x,float y,float z) {
		m_Position[0] = x;
		m_Position[1] = y;
		m_Position[2] = z;
	}
	
	void SetScale(float x,float y,float z) {
		m_Scale[0] = x;
		m_Scale[1] = y;
		m_Scale[2] = z;
	}
	
	void SetRotation(float x, float y, float z) {
		m_Rotation[0] = x;
		m_Rotation[1] = y;
		m_Rotation[2] = z;
	}

	void SetVelocity(float x, float y, float z) {
		m_Velocity[0] = x;
		m_Velocity[1] = y;
		m_Velocity[2] = z;
	}

	void SetLife(float life) {
		m_Life = life;
	}

	void SetTexture(int t) {
		m_Texture = t;
	}

	void SetFrame(int f) {
		m_Frame = f;
	}

	void ScaleTo(float x, float y, float z, float dt) {
		float dx = m_Scale[0] - x;
		float dy = m_Scale[0] - x;
		float dz = m_Scale[0] - x;
		m_Scale[0] = m_Scale[0] - ((0.99 * dx) * dt);
		m_Scale[1] = m_Scale[1] - ((0.99 * dy) * dt);
		m_Scale[2] = m_Scale[2] - ((0.99 * dz) * dt);
	}

	bool IsMovable () {
		if (m_IsMoving) {
			return false;
		} else if (m_IsStuck) {
			return false;
		} else {
			return true;
		}
	}
	
	bool IsClimbing () {
		if (m_Climbing) {
			return true;
		} else {
			return false;
		}
	}

	bool IsClimbable (Model *other) {
		if (m_IsStuck) {
		return true;
		} else {
		return false;
		}
	}

	void Climb(Model *other) {
		if (m_IsStuck || m_Climbing) {
		} else {
		m_Climbing = other;
		}
	}

	void Fall() {
		m_IsFalling = true;
	}

	bool m_IsPushing;
	bool m_IsPlayer;
	bool m_IsEnemy;
	bool m_IsBomb;
	bool m_IsShield;
	bool m_IsStuck;
	bool m_IsHarmfulToPlayers;
	bool m_IsHardfultoEnemies;
	bool m_IsHelpfulToPlayers;
	bool m_IsHelpfulToEnemies;
	bool m_NeedsClimbBeforeMove;
	bool m_NeedsClimbAfterMove;
	bool m_IsMoving;
	bool m_IsFalling;
	bool m_IsAlive;
	foofoo *m_FooFoo;
	float m_Life;
	GLfloat *m_Scale;
	float *m_Position;
	float *m_Rotation;
	float *m_Velocity;
	float m_Theta;
	int m_Frame;
	GLuint m_Texture;
	Model *m_Climbing;
	int m_Direction;
	int m_FramesOfAnimationCount;	
	std::vector<void *> *m_Steps;
	float m_Fps;
};
