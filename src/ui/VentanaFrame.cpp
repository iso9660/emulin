/*
 * VentanaFrame.cpp
 *
 *  Created on: 21 ago. 2019
 *      Author: iso9660
 */

#include <VentanaFrame.h>

namespace ui {

VentanaFrame::VentanaFrame(GtkBuilder *builder, ldf *db, const char *frame_name)
{
	GtkTreeIter it;

	// Store input info
	this->builder = builder;
	this->db = db;
	this->frame_name = frame_name;
	this->handle = gtk_builder_get_object(builder, "VentanaFrame");

	// Pin widgets
	G_PIN(VentanaFrameName);
	G_PIN(VentanaFrameID);
	G_PIN(VentanaFrameSize);
	G_PIN(VentanaFramePublisher);
	G_PIN(VentanaFrameSignalsList);
	G_PIN(VentanaFrameSignalsSelection);
	G_PIN(VentanaFrameSignalsNew);
	G_PIN(VentanaFrameSignalsEdit);
	G_PIN(VentanaFrameSignalsDelete);
	G_PIN(VentanaFrameAccept);
	G_PIN(VentanaFrameCancel);

	// Fill publisher field with static data, selecting the publisher by default
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(g_VentanaFramePublisher));
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(g_VentanaFramePublisher), (gchar *)db->GetMasterNode()->GetName(), (gchar *)db->GetMasterNode()->GetName());
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(g_VentanaFramePublisher), (gchar *)db->GetMasterNode()->GetName());
	for (uint32_t ix = 0; ix < db->GetSlaveNodesCount(); ix++)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(g_VentanaFramePublisher), (gchar *)db->GetSlaveNode(ix)->GetName(), (gchar *)db->GetSlaveNode(ix)->GetName());

	// Prepare signals list
	PrepareListSignals();

	// Fill fields with data
	ldfframe *f = db->GetFrameByName((uint8_t *)frame_name);
	if (f != NULL)
	{
		// Name
		gtk_entry_set_text(GTK_ENTRY(g_VentanaFrameName), (gchar *)f->GetName());

		// ID
		gtk_entry_set_text(GTK_ENTRY(g_VentanaFrameID), GetStrPrintf("%d", f->GetId()));

		// Size
		gtk_entry_set_text(GTK_ENTRY(g_VentanaFrameSize), GetStrPrintf("%d", f->GetSize()));

		// Publisher
		gtk_combo_box_set_active_id(GTK_COMBO_BOX(g_VentanaFramePublisher), (gchar *)f->GetPublisher());

		// Signals
		GtkListStore *ls = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(g_VentanaFrameSignalsList)));
		gtk_list_store_clear(ls);
		for (uint32_t ix = 0; ix < f->GetSignalsCount(); ix++)
		{
			ldfframesignal *fs = f->GetSignal(ix);
			ldfsignal *ss = db->GetSignalByName(fs->GetName());

			// Add Offset, name and bit size
			gtk_list_store_append(ls, &it);
			gtk_list_store_set(ls, &it, 0, (gchar *)GetStrPrintf("%d", fs->GetOffset()), -1);
			gtk_list_store_set(ls, &it, 1, (gchar *)fs->GetName(), -1);
			gtk_list_store_set(ls, &it, 2, (gchar *)GetStrPrintf("%d", ss->GetBitSize()), -1);
		}
	}
	else
	{
		// Name
		gtk_entry_set_text(GTK_ENTRY(g_VentanaFrameName), "frame");

		// ID
		gtk_entry_set_text(GTK_ENTRY(g_VentanaFrameID), "0");

		// Size
		gtk_entry_set_text(GTK_ENTRY(g_VentanaFrameSize), "8");

		// Publisher should be a default one

		// No subscribers
		GtkListStore *ls = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(g_VentanaFrameSignalsList)));
		gtk_list_store_clear(ls);
	}

	// Connect text fields
	G_CONNECT_INSTXT(VentanaFrameName, NAME_EXPR);
	G_CONNECT_INSTXT(VentanaFrameID, INT3_EXPR);
	G_CONNECT_INSTXT(VentanaFrameSize, INT_1_8_EXPR);

	// Connect lists
	G_CONNECT(VentanaFramePublisher, changed);
	G_CONNECT(VentanaFrameSignalsSelection, changed);
	G_CONNECT(VentanaFrameSignalsList, row_activated);

	// Connect buttons
	G_CONNECT(VentanaFrameSignalsNew, clicked);
	G_CONNECT(VentanaFrameSignalsEdit, clicked);
	G_CONNECT(VentanaFrameSignalsDelete, clicked);
	G_CONNECT(VentanaFrameAccept, clicked);
	G_CONNECT(VentanaFrameCancel, clicked);
}

VentanaFrame::~VentanaFrame()
{
	// Disconnect text fields
	G_DISCONNECT_FUNC(VentanaFrameName, EditableInsertValidator);
	G_DISCONNECT_FUNC(VentanaFrameID, EditableInsertValidator);
	G_DISCONNECT_FUNC(VentanaFrameSize, EditableInsertValidator);

	// Disconnect lists
	G_DISCONNECT_DATA(VentanaFrameSignalsSelection, this);
	G_DISCONNECT_DATA(VentanaFrameSignalsList, this);

	// Disconnect buttons
	G_DISCONNECT_DATA(VentanaFrameSignalsNew, this);
	G_DISCONNECT_DATA(VentanaFrameSignalsEdit, this);
	G_DISCONNECT_DATA(VentanaFrameSignalsDelete, this);
	G_DISCONNECT_DATA(VentanaFrameAccept, this);
	G_DISCONNECT_DATA(VentanaFrameCancel, this);
}

ldfframe *VentanaFrame::ShowModal(GObject *parent)
{
	ldfframe *res = NULL;

	// Put this window always on top of parent
	gtk_window_set_transient_for(GTK_WINDOW(handle), GTK_WINDOW(parent));

	// Show dialog
	if (gtk_dialog_run(GTK_DIALOG(handle)))
	{
		// Name
		const char *name = gtk_entry_get_text(GTK_ENTRY(g_VentanaFrameName));

		// ID
		uint16_t id = MultiParseInt(gtk_entry_get_text(GTK_ENTRY(g_VentanaFrameID)));

		// Size
		uint32_t size = MultiParseInt(gtk_entry_get_text(GTK_ENTRY(g_VentanaFrameSize)));

		// Publisher
		const gchar *publisher = gtk_combo_box_get_active_id(GTK_COMBO_BOX(g_VentanaFramePublisher));

		// Compose the signal
		res = new ldfframe((uint8_t *)name, id, (uint8_t *)publisher, size);

		// Signals
		char *strOffset;
		char *signal;
		GtkTreeIter iter;
		GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_VentanaFrameSignalsList));
		if (gtk_tree_model_get_iter_first(model, &iter))
		{
			do
			{
				gtk_tree_model_get(model, &iter, 0, &strOffset, -1);
				gtk_tree_model_get(model, &iter, 1, &signal, -1);
				res->AddSignal(new ldfframesignal((uint8_t *)signal, MultiParseInt(strOffset)));
			}
			while (gtk_tree_model_iter_next(model, &iter));
		}
	}
	gtk_widget_hide(GTK_WIDGET(handle));

	return res;
}

void VentanaFrame::PrepareListSignals()
{
	GtkListStore *s = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeView *v = GTK_TREE_VIEW(g_VentanaFrameSignalsList);

	// Add columns
	TreeViewRemoveColumn(v, 0);
	TreeViewRemoveColumn(v, 0);
	TreeViewRemoveColumn(v, 0);
	TreeViewAddColumn(v, "Offset", 0);
	TreeViewAddColumn(v, "Signal", 1);
	TreeViewAddColumn(v, "Bit size", 2);

	// Set model and unmanage reference from this code
	gtk_tree_view_set_model(v, GTK_TREE_MODEL(s));
	g_object_unref(s);

	// Disable edit and delete buttons
	gtk_widget_set_sensitive(GTK_WIDGET(g_VentanaFrameSignalsEdit), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(g_VentanaFrameSignalsDelete), FALSE);
}

void VentanaFrame::OnVentanaFramePublisher_changed(GtkComboBoxText *widget, gpointer user_data)
{
	VentanaFrame *v = (VentanaFrame *)user_data;
	GtkListStore *ls = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(v->g_VentanaFrameSignalsList)));

	// When publisher changes all signals will be removed from list
	gtk_list_store_clear(ls);
}

void VentanaFrame::OnVentanaFrameSignalsList_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	VentanaFrame *v = (VentanaFrame *)user_data;

	gtk_widget_activate(GTK_WIDGET(v->g_VentanaFrameSignalsEdit));
}

void VentanaFrame::OnVentanaFrameSignalsSelection_changed(GtkTreeSelection *widget, gpointer user_data)
{
	VentanaFrame *v = (VentanaFrame *)user_data;

	bool enable = gtk_tree_selection_count_selected_rows(widget) == 1;
	gtk_widget_set_sensitive(GTK_WIDGET(v->g_VentanaFrameSignalsEdit), enable);
	gtk_widget_set_sensitive(GTK_WIDGET(v->g_VentanaFrameSignalsDelete), enable);
}

void VentanaFrame::OnVentanaFrameSignalsNew_clicked(GtkButton *button, gpointer user_data)
{

}

void VentanaFrame::OnVentanaFrameSignalsEdit_clicked(GtkButton *button, gpointer user_data)
{

}

void VentanaFrame::OnVentanaFrameSignalsDelete_clicked(GtkButton *button, gpointer user_data)
{

}

void VentanaFrame::OnVentanaFrameAccept_clicked(GtkButton *button, gpointer user_data)
{
	VentanaFrame *v = (VentanaFrame *)user_data;
	const char *new_frame_name = gtk_entry_get_text(GTK_ENTRY(v->g_VentanaFrameName));
	const uint8_t new_frame_id = MultiParseInt(gtk_entry_get_text(GTK_ENTRY(v->g_VentanaFrameID)));

	ldfframe *frame_by_new_name = v->db->GetFrameByName((uint8_t *)new_frame_name);
	ldfframe *frame_by_old_name = v->db->GetFrameByName((uint8_t *)v->frame_name);
	ldfframe *frame_by_new_id = v->db->GetFrameById(new_frame_id);

	// Store maximum frame position depending on the frame size
	uint32_t max_frame_pos = MultiParseInt(gtk_entry_get_text(GTK_ENTRY(v->g_VentanaFrameSize))) * 8;

	// Store maximum signal position
	uint32_t max_signal_pos = 0;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(v->g_VentanaFrameSignalsList));
	if (gtk_tree_model_get_iter_first(model, &iter))
	{
		do
		{
			char *p;
			uint32_t q;
			gtk_tree_model_get(model, &iter, 0, &p, -1);	// Offset
			q = MultiParseInt(p);
			gtk_tree_model_get(model, &iter, 2, &p, -1);	// Bit size
			q += MultiParseInt(p);
			max_signal_pos = (q > max_signal_pos) ? q : max_signal_pos;
		}
		while (gtk_tree_model_iter_next(model, &iter));
	}

	if (strlen(new_frame_name) == 0)
	{
		ShowErrorMessageBox(v->handle, "Frame name length shall not be 0.");
		return;
	}
	else if (v->frame_name == NULL && frame_by_new_name != NULL)
	{
		ShowErrorMessageBox(v->handle, "Frame name '%s' is already in use.", new_frame_name);
		return;
	}
	else if (v->frame_name != NULL && strcmp((char *)v->frame_name, new_frame_name) != 0 && frame_by_new_name != NULL)
	{
		ShowErrorMessageBox(v->handle, "Frame name changed to '%s' that is already in use.", new_frame_name);
		return;
	}
	else if (v->frame_name == NULL && frame_by_new_id != NULL)
	{
		ShowErrorMessageBox(v->handle, "Frame ID %d is already in use.", new_frame_id);
		return;
	}
	else if (frame_by_new_id != NULL && frame_by_old_name != NULL && frame_by_new_id != frame_by_old_name)
	{
		ShowErrorMessageBox(v->handle, "Frame ID %d is in use by frame '%s'.", new_frame_id, frame_by_new_id->GetName());
		return;
	}
	else if (max_frame_pos == 0)
	{
		ShowErrorMessageBox(v->handle, "Frame size cannot be 0.");
		return;
	}
	else if (max_frame_pos < max_signal_pos)
	{
		ShowErrorMessageBox(v->handle, "Frame shall have enough bytes for all signals (%d).", (max_signal_pos + 7) / 8);
		return;
	}

	// Return true
	gtk_dialog_response(GTK_DIALOG(v->handle), true);
}

void VentanaFrame::OnVentanaFrameCancel_clicked(GtkButton *button, gpointer user_data)
{
	VentanaFrame *v = (VentanaFrame *)user_data;

	// Return false
	gtk_dialog_response(GTK_DIALOG(v->handle), false);
}


} /* namespace ui */
