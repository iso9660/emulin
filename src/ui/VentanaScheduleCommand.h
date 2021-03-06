/*
 * VentanaCommandFrame.h
 *
 *  Created on: 6 sept. 2019
 *      Author: iso9660
 */

#ifndef UI_VENTANASCHEDULECOMMAND_H_
#define UI_VENTANASCHEDULECOMMAND_H_

#include "tools.h"
#include "ldf.h"

using namespace tools;
using namespace lin;


namespace ui {

class VentanaScheduleCommand
{

private:
	GtkBuilder *builder;
	ldf *db;
	GObject *handle;

	// Widgets
	G_VAR(VentanaScheduleCommandType);
	G_VAR(VentanaScheduleCommandFrameName);
	G_VAR(VentanaScheduleCommandSlave);
	G_VAR(VentanaScheduleCommandTimeout);
	G_VAR(VentanaScheduleCommandDataRaw);
	G_VAR(VentanaScheduleCommandDataFrame);
	G_VAR(VentanaScheduleCommandDataCount);
	G_VAR(VentanaScheduleCommandAccept);
	G_VAR(VentanaScheduleCommandCancel);
	G_VAR(VentanaScheduleCommandTypeLabel);
	G_VAR(VentanaScheduleCommandFrameNameLabel);
	G_VAR(VentanaScheduleCommandSlaveLabel);
	G_VAR(VentanaScheduleCommandTimeoutLabel);
	G_VAR(VentanaScheduleCommandDataRawLabel);
	G_VAR(VentanaScheduleCommandDataFrameLabel);
	G_VAR(VentanaScheduleCommandDataCountLabel);

	void UpdateLayout();

	// Signal callbacks
	static void OnVentanaScheduleCommandType_changed(GtkComboBoxText *widget, gpointer user_data);
	static void OnVentanaScheduleCommandSlave_changed(GtkComboBoxText *widget, gpointer user_data);
	static void OnVentanaScheduleCommandAccept_clicked(GtkButton *button, gpointer user_data);
	static void OnVentanaScheduleCommandCancel_clicked(GtkButton *button, gpointer user_data);

public:
	VentanaScheduleCommand(GtkBuilder *builder, ldf *db, const char *command, const char *timeout);
	virtual ~VentanaScheduleCommand();

	ldfschedulecommand *ShowModal(GObject *parent);


};

} /* namespace ui */

#endif /* UI_VENTANASCHEDULECOMMAND_H_ */
