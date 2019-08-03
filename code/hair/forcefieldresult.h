#ifndef _FORCEFIELDRESULT_H_
#define _FORCEFIELDRESULT_H_


class CEntity;
class Transform;
class ForceField;


//--------------------------------------------------
//!
//!	One stuff which is influence by a force field
//!
//--------------------------------------------------
class ForceFieldResult
{
protected:
	CVector m_worldForce;
public:
	static CPoint GetWorldForceFieldPosition(const CEntity* e);
	
	CVector GetWorldForce() const {return m_worldForce;}
	
	void ResetForce()
	{
		m_worldForce = CVector(CONSTRUCT_CLEAR);
	}
	
	void AddForce(const CVector& worldForce)
	{
		m_worldForce += worldForce;
		m_worldForce.W() += 1.0f;
	}

	bool NotInfluenced()
	{
		return m_worldForce.W() < 0.5f;
	}

	void SetIsInfluenced()
	{
		m_worldForce.W() = 1.0f;
	}
	
	void AddForce(const ForceField* f, const CEntity* e);
	
	ForceFieldResult():m_worldForce(CONSTRUCT_CLEAR) {};
}; // end of class ForceFieldResult



#endif // end of _FORCEFIELDRESULT_H_
