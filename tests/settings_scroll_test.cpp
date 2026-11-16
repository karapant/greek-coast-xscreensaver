#include <gtk/gtk.h>
#include <iostream>
#include <stdexcept>
struct XScreenSaverDialog {GtkDialog parent;};
struct XScreenSaverDialogClass {GtkDialogClass parent;};
G_DEFINE_TYPE(XScreenSaverDialog,xscreensaver_dialog,GTK_TYPE_DIALOG)
static void xscreensaver_dialog_class_init(XScreenSaverDialogClass*){}
static void xscreensaver_dialog_init(XScreenSaverDialog*){}
void check(bool b,const char *s){if(!b)throw std::runtime_error(s);}
void name(GtkWidget *w,const char *s){gtk_buildable_set_name(GTK_BUILDABLE(w),s);}
void pump(){for(int i=0;i<50;++i)while(gtk_events_pending())gtk_main_iteration();}
template<typename Predicate> bool waitUntil(Predicate ready){
    const gint64 deadline=g_get_monotonic_time()+3*G_TIME_SPAN_SECOND;
    do{
        pump();
        if(ready())return true;
        g_usleep(1000);
    }while(g_get_monotonic_time()<deadline);
    return false;
}
int main(int argc,char **argv){
    if(!gtk_init_check(&argc,&argv)){std::cout<<"Desktop display unavailable for native GTK scrolling test\n";return 77;}
    auto *dialog=GTK_WIDGET(g_object_new(xscreensaver_dialog_get_type(),nullptr));
    auto *body=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    auto *actions=gtk_dialog_get_action_area(GTK_DIALOG(dialog));name(actions,"dialog_action_area");
    auto *save=gtk_dialog_add_button(GTK_DIALOG(dialog),"Save",GTK_RESPONSE_OK);
    auto *content=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);name(content,"vbox1");
    auto *notebook=gtk_notebook_new();name(notebook,"opt_notebook");
    auto *settings=gtk_box_new(GTK_ORIENTATION_VERTICAL,4);name(settings,"settings_vbox");
    for(int i=0;i<40;++i){auto *control=gtk_check_button_new_with_label("Native screensaver setting");gtk_box_pack_start(GTK_BOX(settings),control,FALSE,FALSE,0);}
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),settings,gtk_label_new("Standard"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),gtk_entry_new(),gtk_label_new("Advanced"));
    gtk_box_pack_start(GTK_BOX(content),notebook,TRUE,TRUE,0);gtk_box_pack_start(GTK_BOX(body),content,TRUE,TRUE,0);
    gtk_widget_show_all(dialog);pump();
    auto *scroll=GTK_WIDGET(g_object_get_data(G_OBJECT(dialog),"greek-coast-scroll-installed"));
    check(scroll&&GTK_IS_SCROLLED_WINDOW(scroll),"native settings receive a scrollable container");
    check(gtk_widget_is_ancestor(settings,scroll),"controls are inside scrolling content");
    check(!gtk_widget_is_ancestor(save,scroll),"Save remains outside scrolling content");
    gtk_window_resize(GTK_WINDOW(dialog),700,450);
    // X11 configure events and GTK frame-clock allocations arrive asynchronously.
    // Draining only currently pending events can still leave the initial size.
    gint width=0,height=0;
    const bool resized=waitUntil([&]{
        gtk_window_get_size(GTK_WINDOW(dialog),&width,&height);
        return height<=450&&gtk_widget_get_allocated_height(dialog)==height;
    });
    if(!resized)std::cerr<<"Requested height <= 450, got "<<height
                         <<" (allocation "<<gtk_widget_get_allocated_height(dialog)<<")\n";
    check(resized,"large control list cannot force dialog taller than requested size");
    auto *adjustment=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroll));
    check(gtk_adjustment_get_upper(adjustment)>gtk_adjustment_get_page_size(adjustment),"overflowing controls can scroll");
    gtk_adjustment_set_value(adjustment,gtk_adjustment_get_upper(adjustment));pump();
    gint x,y;check(gtk_widget_translate_coordinates(save,dialog,0,0,&x,&y),"Save is allocated in dialog");
    check(y>=0&&y+gtk_widget_get_allocated_height(save)<=height,"Save remains visible after scrolling to the last setting");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),1);pump();gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),0);pump();
    gtk_widget_show_all(dialog);check(g_object_get_data(G_OBJECT(dialog),"greek-coast-scroll-installed")==scroll,"reopening and page switching do not nest scroll wrappers");
    auto *ordinary=gtk_dialog_new();gtk_widget_show_all(ordinary);check(!g_object_get_data(G_OBJECT(ordinary),"greek-coast-scroll-installed"),"unrelated GTK dialogs are unaffected");
    gtk_widget_destroy(ordinary);gtk_widget_destroy(dialog);
    std::cout<<"Scrollable native controls, fixed Save button, resizing and page switching passed\n";
}
