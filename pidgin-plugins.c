/*
 * pidgin-plugins.c
 * 
 * Copyright 2013 Bhaskar Kandiyal <bkandiyal@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <webkit/webkitwebview.h>

GtkWidget *window;
GtkWidget *paned;
GtkWidget *notebook;
GtkWidget *pluginsView;
GtkListStore *pluginStore;

WebKitWebView *web_view_download_plugins;
WebKitWebView *web_view_plugins_info;

gchar *user_agent = "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16";

void set_plugin_info(gchar *name, gchar *version, gchar *description);
void add_plugin(GtkListStore *pluginStore, gchar *name, gchar *version, gchar *description);
void populate_installed_plugins();
void on_pluginsView_button_press_event(GtkWidget* widget, GdkEventButton *button_event, gpointer user_data);
void on_pluginsView_cursor_changed_event(GtkTreeView *treeview, gpointer user_data);

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	
	GtkBuilder *builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "plugins-ui.glade", NULL);
	gtk_builder_connect_signals(builder, NULL);
	
	window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
	g_signal_connect(G_OBJECT(window), "destroy", gtk_main_quit, NULL);
	
	paned = GTK_WIDGET(gtk_builder_get_object(builder, "paned"));
	
	notebook = GTK_WIDGET(gtk_builder_get_object(builder, "notebook"));
	
	//pluginsView = GTK_WIDGET(gtk_builder_get_object(builder, "pluginsView"));
	
	//pluginStore = GTK_LIST_STORE(gtk_builder_get_object(builder, "pluginStore"));
	
	web_view_plugins_info = WEBKIT_WEB_VIEW(webkit_web_view_new());
	web_view_download_plugins = WEBKIT_WEB_VIEW(webkit_web_view_new());
	
    WebKitWebSettings *settings = webkit_web_settings_new();
    g_object_set(G_OBJECT(settings), "user-agent", user_agent, NULL);
    g_object_set(G_OBJECT(settings), "user-stylesheet-uri", "style.css", NULL);
      
    webkit_web_view_set_settings(web_view_download_plugins, settings);
    webkit_web_view_set_settings(web_view_plugins_info, settings);
    
    GtkWidget *container_download_plugins, *container_plugins_info;
    container_download_plugins = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (container_download_plugins), GTK_WIDGET(web_view_download_plugins));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), container_download_plugins, GTK_WIDGET(gtk_label_new("Download Plugins")));
    
    container_plugins_info = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(container_plugins_info), GTK_WIDGET(web_view_plugins_info));
    gtk_paned_add2(GTK_PANED(paned), container_plugins_info);
    
    webkit_web_view_load_uri(web_view_download_plugins, "http://bhaskar-kandiyal.rhcloud.com/");
	
	populate_installed_plugins();
	
	gtk_widget_show_all(window);
	//set_plugin_info("AutoAccept", "2.8", "Auto-Accept files");
	
	g_object_unref(builder);
	gtk_main();
	
	return 0;
}

void populate_installed_plugins()
{
    GtkTreeView *treeView;
    GtkTreeIter *iter;
    GtkCellRenderer *rendererText;
    
    //Setup the cell renderers
    rendererText = gtk_cell_renderer_text_new();
    g_object_set(rendererText, "markup", "0", NULL);
    
	pluginStore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    
    /* Fill treeStore with dummy values */
    
    add_plugin(pluginStore, "Auto Accept", "1.0", "Auto-accept file transfer requests from selected users.");    
    add_plugin(pluginStore, "Buddy Notes", "1.0", "Store notes on particular buddies.");    
    add_plugin(pluginStore, "Buddy State Notification", "1.0", "Notifies in a conversation window when a buddy goes or returns from away or idle.");    
    add_plugin(pluginStore, "Buddy Ticker", "1.0", "A horizontal scrolling version of the buddy list.");    
    
    /* -- */
    
    //g_print("Setting up columns");
    
    pluginsView = gtk_tree_view_new();
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pluginsView), TRUE);
    gtk_tree_view_set_model(GTK_TREE_VIEW(pluginsView), GTK_TREE_MODEL(pluginStore));
    
    g_signal_connect(G_OBJECT(pluginsView), "button-press-event", G_CALLBACK(on_pluginsView_button_press_event), NULL);
    g_signal_connect(G_OBJECT(pluginsView), "cursor-changed", G_CALLBACK(on_pluginsView_cursor_changed_event), NULL);
    
    //g_object_unref(pluginStore);
    
    //Setup columns
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pluginsView), -1, "Name", rendererText, "text", 0, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pluginsView), -1, "Version", rendererText, "text", 1, NULL);
    //gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pluginsView), -1, "Description", rendererText, "text", 2, NULL);
     
    gtk_paned_add1(GTK_PANED(paned), pluginsView);
    
    // Select first element
    GtkTreePath *path = gtk_tree_path_new_from_string("0");
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(pluginsView), path, NULL, FALSE);

    //g_object_unref(rendererText);
}

void set_plugin_info(gchar *name, gchar *version, gchar *description)
{
	gchar *template;
	if(!g_file_get_contents("template.html", &template, NULL, NULL))
	{
		g_print("\nError: Cannot get file contents!");
	}
	
	gchar *output = g_strdup_printf(template, name, version, description);
	webkit_web_view_load_string(web_view_plugins_info, output, NULL, NULL, g_get_current_dir());
	//g_print(g_get_current_dir());
	
	g_free(output);
	g_free(template);
}

void add_plugin(GtkListStore *pluginStore, gchar *name, gchar *version, gchar *description)
{
	GtkTreeIter iter;
    gtk_list_store_insert_with_values(pluginStore, &iter, -1, 0, name, 1, version, 2, description, -1);
}

void on_pluginsView_button_press_event(GtkWidget* widget, GdkEventButton *button_event, gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	
	gchar *name, *version, *description;
	
	gboolean ret = gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), button_event->x, button_event->y, &path, NULL, NULL, NULL);
  
	if(!ret)
	{
		return;
	}

	//gtk_tree_selection_get_selected(selected, &model, &iter);  
	gtk_tree_model_get_iter(GTK_TREE_MODEL(pluginStore), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(pluginStore), &iter, 0, &name, 1, &version, 2, &description, -1);
	
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), path, NULL, false);
	
	//set_plugin_info(name, version, description);
	
	//g_print("%s\n", name);
}

void on_pluginsView_cursor_changed_event(GtkTreeView *treeview, gpointer user_data)
{
	gchar *name, *version, *description;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	if(model != NULL) {
		gtk_tree_selection_get_selected(selection, &model, &iter);
		gtk_tree_model_get(GTK_TREE_MODEL(pluginStore), &iter, 0, &name, 1, &version, 2, &description, -1);
		set_plugin_info(name, version, description);
	}
}
