/*
 * ldfnode.h
 *
 *  Created on: 3 jul. 2019
 *      Author: iso9660
 */

#ifndef LIN_LDFNODE_H_
#define LIN_LDFNODE_H_

#include <stdint.h>

namespace lin {

class ldfnode {

private:
	uint8_t *name;

public:
	ldfnode(const uint8_t *name);
	virtual ~ldfnode();

	static bool CheckNodeName(uint8_t *name, ldfnode *master, ldfnode **slaves, uint32_t slaves_count);

	uint8_t *GetName();
	void UpdateName(const uint8_t *old_name, const uint8_t *new_name);


};

}

#endif /* LIN_LDFNODE_H_ */
