/*
 ============================================================================
 Name        : LIN.cpp
 Author      : ISO9660
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C++,
 ============================================================================
 */

#include <locale.h>
#include <gtk/gtk.h>
#include <VentanaInicio.h>


using namespace std;
using namespace ui;


int main(int argc, char *argv[])
{
	GtkBuilder *builder;
	GError *error = NULL;

	// Initialize GTK
	gtk_init(&argc, &argv);
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "src/ui/Emulin.glade", &error);

	// Change locale because GTK changes it to the system one
	setlocale(LC_ALL, "en_US");

	// Instantiate VentanaInicio
	VentanaInicio *v = new VentanaInicio(builder);

	// Run GTK's main loop
	gtk_main();

	// Resource erasing
	delete v;

	return 0;
}
