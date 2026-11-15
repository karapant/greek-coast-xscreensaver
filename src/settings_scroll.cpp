// Keep the native configurator's action area outside scrolling content.
// Loaded only by this user's existing XScreenSaver Settings adapter.
#include <gtk/gtk.h>
#include <algorithm>
#include <cstring>
#include <dlfcn.h>

namespace {
constexpr const char *installed="greek-coast-scroll-installed";
thread_local bool adapting=false;
GtkWidget *named(GtkWidget *root,const char *name){
    if(GTK_IS_BUILDABLE(root)){
        const char *id=gtk_buildable_get_name(GTK_BUILDABLE(root));
        if(id&&std::strcmp(id,name)==0)return root;
    }
    if(!GTK_IS_CONTAINER(root))return nullptr;
    GList *children=gtk_container_get_children(GTK_CONTAINER(root));GtkWidget *found=nullptr;
    for(GList *item=children;item&&!found;item=item->next)found=named(GTK_WIDGET(item->data),name);
    g_list_free(children);return found;
}
bool settingsDialog(GtkWidget *widget){return GTK_IS_WINDOW(widget)&&std::strcmp(G_OBJECT_TYPE_NAME(widget),"XScreenSaverDialog")==0;}
GdkRectangle available(GtkWindow *window){
    GdkDisplay *display=gtk_widget_get_display(GTK_WIDGET(window));
    GdkMonitor *monitor=nullptr;
    GtkWindow *parent=gtk_window_get_transient_for(window);
    GdkWindow *anchor=gtk_widget_get_window(parent?GTK_WIDGET(parent):GTK_WIDGET(window));
    if(anchor)monitor=gdk_display_get_monitor_at_window(display,anchor);
    if(!monitor)monitor=gdk_display_get_primary_monitor(display);
    if(!monitor&&gdk_display_get_n_monitors(display)>0)monitor=gdk_display_get_monitor(display,0);
    GdkRectangle area{0,0,1024,768};if(monitor)gdk_monitor_get_workarea(monitor,&area);
    return area;
}
void adapt(GtkWidget *widget){
    if(adapting)return;
    GtkWidget *root=gtk_widget_get_toplevel(widget);
    if(!settingsDialog(root)||g_object_get_data(G_OBJECT(root),installed))return;
    // Native prefs.ui has this content box as a sibling of the action area.
    // Reparent the whole box so documentation and Advanced controls scroll too.
    GtkWidget *content=named(root,"vbox1");
    GtkWidget *options=named(root,"settings_vbox");
    GtkWidget *buttons=named(root,"dialog_action_area");
    if(!content||!options||!buttons)return;
    GtkWidget *parent=gtk_widget_get_parent(content);
    if(!GTK_IS_BOX(parent)||gtk_widget_is_ancestor(buttons,content))return;
    adapting=true;
    gboolean expand,fill;guint padding;GtkPackType pack;
    gtk_box_query_child_packing(GTK_BOX(parent),content,&expand,&fill,&padding,&pack);
    gint position=0;gtk_container_child_get(GTK_CONTAINER(parent),content,"position",&position,nullptr);
    auto area=available(GTK_WINDOW(root));
    int limit=std::max(200,area.height-80);
    GtkWidget *scroll=gtk_scrolled_window_new(nullptr,nullptr);
    gtk_widget_set_name(scroll,"greek-coast-settings-scroll");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),GTK_SHADOW_NONE);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll),100);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scroll),240);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroll),std::max(100,std::min(600,limit-100)));
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroll),TRUE);
    gtk_widget_set_hexpand(scroll,TRUE);gtk_widget_set_vexpand(scroll,TRUE);
    g_object_ref(content);gtk_container_remove(GTK_CONTAINER(parent),content);
    gtk_box_pack_start(GTK_BOX(parent),scroll,TRUE,TRUE,padding);
    gtk_box_set_child_packing(GTK_BOX(parent),scroll,TRUE,TRUE,padding,pack);
    gtk_box_reorder_child(GTK_BOX(parent),scroll,position);
    gtk_container_add(GTK_CONTAINER(scroll),content);g_object_unref(content);
    g_object_set_data(G_OBJECT(root),installed,scroll);
    gtk_window_set_resizable(GTK_WINDOW(root),TRUE);
    gtk_window_set_position(GTK_WINDOW(root),GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_default_size(GTK_WINDOW(root),std::min(760,std::max(320,area.width-64)),std::min(720,limit));
    gtk_widget_show_all(scroll);
    adapting=false;
}
}
extern "C" void gtk_widget_show(GtkWidget *widget){
    static auto original=reinterpret_cast<void(*)(GtkWidget*)>(dlsym(RTLD_NEXT,"gtk_widget_show"));
    adapt(widget);original(widget);
}
extern "C" void gtk_widget_show_all(GtkWidget *widget){
    static auto original=reinterpret_cast<void(*)(GtkWidget*)>(dlsym(RTLD_NEXT,"gtk_widget_show_all"));
    adapt(widget);original(widget);
}
extern "C" void gtk_window_present(GtkWindow *window){
    static auto original=reinterpret_cast<void(*)(GtkWindow*)>(dlsym(RTLD_NEXT,"gtk_window_present"));
    adapt(GTK_WIDGET(window));original(window);
}
extern "C" void gtk_window_resize(GtkWindow *window,gint width,gint height){
    static auto original=reinterpret_cast<void(*)(GtkWindow*,gint,gint)>(dlsym(RTLD_NEXT,"gtk_window_resize"));
    adapt(GTK_WIDGET(window));
    if(settingsDialog(GTK_WIDGET(window))&&g_object_get_data(G_OBJECT(window),installed)){
        auto area=available(window);width=std::min(width,std::max(320,area.width-64));height=std::min(height,std::max(200,area.height-80));
    }
    original(window,width,height);
}
