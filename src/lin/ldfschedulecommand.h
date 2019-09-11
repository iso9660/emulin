/*
 * ldfschedulecommand.h
 *
 *  Created on: 5 jul. 2019
 *      Author: iso9660
 */

#ifndef LIN_LDFSCHEDULECOMMAND_H_
#define LIN_LDFSCHEDULECOMMAND_H_

#include <stdint.h>

namespace lin {

class ldf;

class ldfschedulecommand {

public:
	typedef enum ldfschedulecommandtype_e
	{
		LDF_SCMD_TYPE_UnconditionalFrame = 0,
		LDF_SCMD_TYPE_MasterReq = 1,
		LDF_SCMD_TYPE_SlaveResp = 2,
		LDF_SCMD_TYPE_AssignNAD = 3,
		LDF_SCMD_TYPE_DataDump = 4,
		LDF_SCMD_TYPE_SaveConfiguration = 5,
		LDF_SCMD_TYPE_FreeFormat = 6,
		LDF_SCMD_TYPE_AssignFrameIdRange = 7,
		LDF_SCMD_TYPE_AssignFrameId = 8
	} ldfschedulecommandtype_t;

private:
	ldfschedulecommandtype_t type;
	uint8_t *frame_name;
	uint16_t timeout;
	uint8_t *slave_name;
	uint8_t data[8];
	uint8_t *assign_frame_name;

public:
	ldfschedulecommand(ldfschedulecommandtype_t, uint8_t *frame_name, uint16_t timeout, uint8_t *slave_name, uint8_t *data, uint8_t *assign_frame_name);
	virtual ~ldfschedulecommand();

	static ldfschedulecommand *FromLdfStatement(uint8_t *statement);

	ldfschedulecommandtype_t GetType();
	uint8_t *GetFrameName();
	uint16_t GetTimeoutMs();
	uint8_t *GetSlaveName();
	uint8_t *GetData();
	uint8_t *GetAssignFrameName();

	void UpdateFrameName(const uint8_t *old_name, const uint8_t *new_name);
	void ValidateUnicity(uint8_t *schedule_table, ldfschedulecommand *command, uint8_t **validation_messages, uint32_t *validation_messages_count);
	uint8_t *GetCommandText(ldf *db);


};

} /* namespace lin */

#endif /* LIN_LDFSCHEDULECOMMAND_H_ */
