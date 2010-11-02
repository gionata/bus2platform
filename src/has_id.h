/*! \file has_id.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef HAS_ID_H_
#define HAS_ID_H_

/**
 *
 */
class has_id {
public:
	has_id(const int id);
	~has_id();
	int id() const;
	void id(const int id);	
	friend class lthas_id;

private:
	int _id;
};

class lthas_id {
public:
	bool operator() (const has_id *a, has_id *b)const {
		return (a->_id < b->_id);
	}
};

#endif /* HAS_ID_H_ */
