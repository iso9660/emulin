/*
 * ldfnodeattributes.cpp
 *
 *  Created on: 5 jul. 2019
 *      Author: iso9660
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ldfcommon.h>
#include <ldfnodeattributes.h>

namespace lin {

ldfnodeattributes::ldfnodeattributes(const uint8_t *name)
{
	this->name = StrDup(name);
	this->protocol = LIN_PROTOCOL_VERSION_NONE;
	this->configured_NAD = 0xFF;
	this->initial_NAD = 0xFF;
	this->product_id = { 0, 0, 0 };
	this->response_error_signal_name = NULL;
	this->fault_state_signals_count = 0;
	this->P2_min = 50;
	this->ST_min = 0;
	this->N_As_timeout = 1000;
	this->N_Cr_timeout = 1000;
	this->configurable_frames_count = 0;
}

ldfnodeattributes::~ldfnodeattributes()
{
	if (name != NULL) delete name;
	if (response_error_signal_name != NULL) delete response_error_signal_name;
	while (fault_state_signals_count > 0) delete fault_state_signals[--fault_state_signals_count];
	while (configurable_frames_count > 0) delete configurable_frames[--configurable_frames_count];
}

void ldfnodeattributes::UpdateFromLdfStatement(uint8_t *statement)
{
	char *p = NULL;

	// Isolate parameter name
	p = strtok((char *) statement, "=," BLANK_CHARACTERS);
	if (!p) return;

	// Isolate and parse parameter value
	if (StrEq(p, "LIN_protocol"))
	{
		p = strtok(NULL, "=," BLANK_CHARACTERS);
		if (!p) return;

		if (StrEq(p, "\"2.1\""))
			protocol = LIN_PROTOCOL_VERSION_2_1;
		else if (StrEq(p, "\"2.0\""))
			protocol = LIN_PROTOCOL_VERSION_2_0;
	}
	else if (StrEq(p, "configured_NAD"))
	{
		p = strtok(NULL, "=," BLANK_CHARACTERS);
		if (p) configured_NAD = ParseInt(p);
	}
	else if (StrEq(p, "initial_NAD"))
	{
		p = strtok(NULL, "=," BLANK_CHARACTERS);
		if (p) initial_NAD = ParseInt(p);
	}
	else if (StrEq(p, "product_id"))
	{
		// Supplier ID
		p = strtok(NULL, "=," BLANK_CHARACTERS);
		if (p) product_id.supplier_id = ParseInt(p);

		// Function ID
		if (p) p = strtok(NULL, "=," BLANK_CHARACTERS);
		if (p) product_id.function_id = ParseInt(p);

		// Variant
		if (p) p = strtok(NULL, "=," BLANK_CHARACTERS);
		if (p) product_id.variant = ParseInt(p);
	}
	else if (StrEq(p, "response_error"))
	{
		p = strtok(NULL, "=,");
		if (p) response_error_signal_name = StrDup(p);
	}
	else if (StrEq(p, "fault_state_signals"))
	{
		while (p)
		{
			p = strtok(NULL, "=,");
			if (p) fault_state_signals[fault_state_signals_count++] = StrDup(p);
		}
	}
	else if (StrEq(p, "P2_min"))
	{
		p = strtok(NULL, "=,");
		if (p) P2_min = atof(p);
	}
	else if (StrEq(p, "ST_min"))
	{
		p = strtok(NULL, "=,");
		if (p) ST_min = atof(p);
	}
	else if (StrEq(p, "N_As_timeout"))
	{
		p = strtok(NULL, "=,");
		if (p) N_As_timeout = atof(p);
	}
	else if (StrEq(p, "N_Cr_timeout"))
	{
		p = strtok(NULL, "=,");
		if (p) N_Cr_timeout = atof(p);
	}
}

void ldfnodeattributes::AddConfigurableFrame(ldfconfigurableframe *frame)
{
	configurable_frames[configurable_frames_count++] = frame;
}

void ldfnodeattributes::ValidateNode(ldfnode **slaves, uint32_t slaves_count, uint8_t **validation_messages, uint32_t *validation_messages_count)
{
	char str[1000];

	if (this->protocol == LIN_PROTOCOL_VERSION_NONE)
	{
		sprintf(str, STR_ERR "Node_attributes '%s' protocol not defined.", name);
		validation_messages[*validation_messages_count++] = StrDup(str);
	}

	if (!ldfnode::CheckNodeName(name, NULL, slaves, slaves_count))
	{
		sprintf(str, STR_ERR "Node_attributes '%s' node not defined in database's slaves", name);
		validation_messages[*validation_messages_count++] = StrDup(str);
	}
}

void ldfnodeattributes::ValidateUnicity(ldfnodeattributes *attributes, uint8_t **validation_messages, uint32_t *validation_messages_count)
{
	char str[1000];

	if (StrEq(name, attributes->name))
	{
		sprintf(str, STR_ERR "Node_attributes '%s' node defined twice", name);
		validation_messages[*validation_messages_count++] = StrDup(str);
	}
}

void ldfnodeattributes::ValidateFrames(ldfframe **frames, uint32_t frames_count, uint8_t **validation_messages, uint32_t *validation_messages_count)
{
	char str[1000];
	uint32_t i, j;

	for (i = 0; i < configurable_frames_count; i++)
	{
		ldfframe *f = NULL;

		// Look for frame definition
		for (j = 0; (f == NULL) && (j < frames_count); j++)
		{
			f = StrEq(frames[j]->GetName(), configurable_frames[i]->GetName()) ? frames[j] : NULL;
		}

		// Check frame exists
		if (f == NULL)
		{
			sprintf(str, STR_ERR "Node_attributes '%s' configurable frame '%s' not defined.", name, configurable_frames[i]->GetName());
			validation_messages[*validation_messages_count++] = StrDup(str);
			continue;
		}

		// Check configurable frame repeated
		for (j = i + 1; j < configurable_frames_count; j++)
		{
			configurable_frames[i]->ValidateUnicity(name, configurable_frames[j], validation_messages, validation_messages_count);
		}
	}
}

uint8_t *ldfnodeattributes::GetName()
{
	return name;
}

lin_protocol_version_e ldfnodeattributes::GetProtocolVersion()
{
	return protocol;
}

uint8_t ldfnodeattributes::GetInitialNAD()
{
	return initial_NAD;
}

uint8_t ldfnodeattributes::GetConfiguredNAD()
{
	return configured_NAD;
}

uint16_t ldfnodeattributes::GetSupplierID()
{
	return product_id.supplier_id;
}

uint16_t ldfnodeattributes::GetFunctionID()
{
	return product_id.function_id;
}

uint8_t ldfnodeattributes::GetVariant()
{
	return product_id.variant;
}

uint8_t *ldfnodeattributes::GetResponseErrorSignalName()
{
	return response_error_signal_name;
}

ldfconfigurableframe *ldfnodeattributes::GetConfigurableFrame(uint32_t ix)
{
	return configurable_frames[ix];
}

uint16_t ldfnodeattributes::GetConfigurableFramesCount()
{
	return configurable_frames_count;
}

uint16_t ldfnodeattributes::GetP2_min()
{
	return P2_min;
}

uint16_t ldfnodeattributes::GetST_min()
{
	return ST_min;
}

uint16_t ldfnodeattributes::GetN_As_timeout()
{
	return N_As_timeout;
}

uint16_t ldfnodeattributes::GetN_Cr_timeout()
{
	return N_Cr_timeout;
}

void ldfnodeattributes::SetProtocolVersion(lin_protocol_version_e v)
{
	protocol = v;
}

void ldfnodeattributes::SetInitialNAD(uint8_t v)
{
	initial_NAD = v;
}

void ldfnodeattributes::SetConfiguredNAD(uint8_t v)
{
	configured_NAD = v;
}

void ldfnodeattributes::SetSupplierID(uint16_t v)
{
	product_id.supplier_id = v;
}

void ldfnodeattributes::SetFunctionID(uint16_t v)
{
	product_id.function_id = v;
}

void ldfnodeattributes::SetVariant(uint8_t v)
{
	product_id.variant = v;
}

void ldfnodeattributes::SetP2_min(uint16_t v)
{
	P2_min = v;
}

void ldfnodeattributes::SetST_min(uint16_t v)
{
	ST_min = v;
}

void ldfnodeattributes::SetN_As_timeout(uint16_t v)
{
	N_As_timeout = v;
}

void ldfnodeattributes::SetN_Cr_timeout(uint16_t v)
{
	N_Cr_timeout = v;
}

void ldfnodeattributes::SetResponseErrorSignalName(const uint8_t *v)
{
	if (response_error_signal_name != NULL) delete response_error_signal_name;
	response_error_signal_name = (v != NULL) ? StrDup(v) : NULL;
}

void ldfnodeattributes::SortData()
{
	qsort(configurable_frames, configurable_frames_count, sizeof(configurable_frames[0]), ldfconfigurableframe::SorterConfigurableFrames);
}

void ldfnodeattributes::UpdateConfigurableFrameNames(const uint8_t *old_frame_name, const uint8_t *new_frame_name)
{
	for (int i = 0; i < configurable_frames_count; i++)
	{
		configurable_frames[i]->UpdateName(old_frame_name, new_frame_name);
	}
}

void ldfnodeattributes::DeleteConfigurableFramesByName(const uint8_t *frame_name)
{
	for (int i = 0; i < configurable_frames_count; i++)
	{
		// Skip configurable frames with different name
		if (!StrEq(configurable_frames[i]->GetName(), frame_name))
		{
			continue;
		}

		// Delete configurable frame and move back the other frames over the gap
		delete configurable_frames[i];
		configurable_frames_count--;
		for (; i < configurable_frames_count; i++)
			configurable_frames[i] = configurable_frames[i + 1];
		break;
	}
}

void ldfnodeattributes::UpdateResponseErrorSignalName(const uint8_t *old_signal_name, const uint8_t *new_signal_name)
{
	if (StrEq(response_error_signal_name, old_signal_name))
	{
		delete response_error_signal_name;
		response_error_signal_name = StrDup(new_signal_name);
	}
}

void ldfnodeattributes::ToLdfFile(FILE *f)
{
	fprintf(f, "    %s {\r\n", name);
	fprintf(f, "        LIN_protocol = \"%s\";\r\n", (protocol == LIN_PROTOCOL_VERSION_2_0) ? "2.0" : "2.1");
	fprintf(f, "        configured_NAD = 0x%02X;\r\n", configured_NAD);
	if (initial_NAD != 0xFF)
		fprintf(f, "        initial_NAD = 0x%02X;\r\n", initial_NAD);

	if (protocol >= LIN_PROTOCOL_VERSION_2_0)
	{
		fprintf(f, "        product_id = 0x%04X, 0x%04X, 0x%04X;\r\n", product_id.supplier_id, product_id.function_id, product_id.variant);
		if (response_error_signal_name)
			fprintf(f, "        response_error = %s;\r\n", response_error_signal_name);
		fprintf(f, "        P2_min = %d ms;\r\n", P2_min);
		fprintf(f, "        ST_min = %d ms;\r\n", ST_min);
		fprintf(f, "        N_As_timeout = %d ms;\r\n", N_As_timeout);
		fprintf(f, "        N_Cr_timeout = %d ms;\r\n", N_Cr_timeout);
		fprintf(f, "\r\n");

		fprintf(f, "        configurable_frames {\r\n");
		for (uint32_t i = 0; i < configurable_frames_count; i++)
		{
			if (protocol == LIN_PROTOCOL_VERSION_2_0)
			{
				fprintf(f, "            %s = 0x%02X;\r\n", configurable_frames[i]->GetName(), configurable_frames[i]->GetId());
			}
			else if (protocol == LIN_PROTOCOL_VERSION_2_1)
			{
				fprintf(f, "            %s;\r\n", configurable_frames[i]->GetName());
			}
		}
		fprintf(f, "        }\r\n");
	}

	fprintf(f, "    }\r\n");
}


} /* namespace lin */
